#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include "cacheline.h"
#include "cache.h"
#define convert(x) (void*)((long long)(addr) / CachelineSize * CachelineSize)
class Processor;
class Bus;

struct BusTransaction{
    BusTransaction(){}
    BusTransaction(void *cline, int thread_id, BusTsnType tsn_type) : 
        tag(cline), tid(thread_id), tsn(tsn_type){}
    void* tag;
    int tid;
    BusTsnType tsn;
};
class Processor
{
    public:
        Processor():bus(nullptr){}
        ~Processor(){} 
        void resize(int capacity){cache.resize(capacity);}  
        void read(void *addr);
        void write(void *addr);
        void pull_request(void *tag);
    LRUCache<void*, Cacheline> cache;
    Bus *bus;
    int tid;
    std::map<void*, BusTransaction> requests;
};

class Bus{
    public:
        void mainloop()
        {
            while(true)
            {
                while(!requests.empty())
                {
                    bus_lock.lock();

                    auto request = requests.front();
                    requests.pop();
                    process(request);

                    bus_lock.unlock();
                } 
            }
        }
        void push(Cacheline cline, int thread_id, PrTsnType pr_request)
        {
            bus_lock.lock();

            BusTsnType tsn_type;
            bool has_response = false;
            switch (cline.state)
            {
                case M : // modified
                    break;
                case S :
                    if(pr_request == PrTsnType::PrWr)
                    {
                        tsn_type = BusTsnType::BusRdX;
                        has_response = true;
                    }
                    // do nothing for PrRd
                    break;
                case I : 
                    if(pr_request == PrTsnType::PrRd)tsn_type = BusTsnType::BusRd;
                    if(pr_request == PrTsnType::PrWr)tsn_type = BusTsnType::BusRdX;
                    has_response = true;
                    break;
            }

            if(has_response)
            {
                requests.push(BusTransaction(cline.tag, thread_id, tsn_type));
            }
            
            bus_lock.unlock();
        }
        void process(BusTransaction request)
        {
            //send requests to other machines
            for(int i = 0; i < processors.size(); i++)
            {
                if(i != request.tid)
                {
                    processors[i].requests[request.tag] = request;
                }
            }
            //waiting for all the request to be finished
            while(true)
            {
                bool finished = true;
                for(int i = 0; i < processors.size(); i++)
                {
                    if(i != request.tid)
                    {
                        auto &rts = processors[i].requests;
                        if(rts.find(request.tag) != rts.end())
                        {
                            finished = false;
                            break;
                        }
                    }
                } 
                if(finished)break;
            }

        }
    

    std::vector<Processor> processors;
    std::queue<BusTransaction> requests;
    std::mutex bus_lock;

    Bus(int nthreads = 8, int NCachelines = 30)
    {
        processors.resize(nthreads);
        for(int i = 0; i < nthreads; i++)
        {
            auto &processor = processors[i];
            processor.resize(NCachelines);
            processor.bus = this;
            processor.tid = i;
        }
    }
    ~Bus(){}
};
//from remote to local
void Processor::pull_request(void *tag)
{
    auto it = requests.find(tag);
    if(it != requests.end())
    {
        auto tsn = it->second.tsn;
        auto cline = cache.get(tag, false);
        switch(cline.state)
        {
            case M:
                if(tsn == BusTsnType::BusRd)
                {
                    cline.state = State::S;
                    //flush
                }
                if(tsn == BusTsnType::BusRdX)
                {
                    cline.state = State::I;
                    //flush
                }
                break;
            case S:
                if(tsn == BusTsnType::BusRd)
                {
                    //no action
                }
                if(tsn == BusTsnType::BusRdX)
                {
                    cline.state = State::I;
                }
                break;
            case I:
                // no action
                break;
        }
        requests.erase(it);
    }
}
void Processor::read(void *addr)
{
    void *tag = convert(addr);
    pull_request(tag);
    
    auto cline = cache.get(addr, true);

    bus->push(cline, tid, PrTsnType::PrRd);

}
void Processor::write(void *addr)
{
    void *tag = convert(addr);
    pull_request(tag);
    
    auto cline = cache.get(addr, true);
    bus->push(cline, tid, PrTsnType::PrWr);
    cline.tag = tag;
    cache.put(addr, cline);
}