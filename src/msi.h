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
    BusTransaction(void *cline, int thread_id, BusTsnType tsn_type, int ct) : 
        tag(cline), tid(thread_id), tsn(tsn_type), cnt(ct){}
    void* tag;
    int tid;
    int cnt;
    BusTsnType tsn;
};
std::ostream& operator <<(std::ostream &os, BusTransaction bts) {
    os<<"timestamp : "<<bts.cnt<<' '<< " tid : "<<bts.tid<<" address : "<<bts.tag;
    switch(bts.tsn)
    {
        case BusTsnType::BusRd:
            os<<" BusRd  ";
            break;
        case BusTsnType::BusRdX:
            os<<" BusRdX ";
            break;
    }
    return os;
}
class Processor
{
    public:
        Processor():bus(nullptr){}
        ~Processor(){} 
        void resize(int capacity){cache.resize(capacity);}  
        void read(void *addr, int cnt);
        void write(void *addr, int cnt);


        void pull_request(void *tag);
        void wait_request(void *tag);
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
                case State::M : // modified
                    break;
                case State::S :
                    if(pr_request == PrTsnType::PrWr)
                    {
                        tsn_type = BusTsnType::BusRdX;
                        cline.state = State::M;
                        has_response = true;
                    }
                    // do nothing for PrRd
                    break;
                case State::I : 
                    if(pr_request == PrTsnType::PrRd)
                    {
                        tsn_type = BusTsnType::BusRd;
                        cline.state = State::S;
                    }
                    if(pr_request == PrTsnType::PrWr)
                    {
                        tsn_type = BusTsnType::BusRdX;
                        cline.state = State::M;
                    }
                    has_response = true;
                    break;
            }
            //if(cline.state == State::M)std::cout<<"fuck\n";
            processors[thread_id].cache.put(cline.tag, cline);
            //std::cout<<processors[thread_id].cache<<std::endl;
            if(has_response)
            {
                requests.push(BusTransaction(cline.tag, thread_id, tsn_type, cline.cnt));
            }
            
            bus_lock.unlock();
        }
        void process(BusTransaction request)
        {
            #pragma critical
            {
                std::cout<<request<<std::endl;
                for(int i = 0; i < processors.size(); i++)
                {
                    auto &processor = processors[i];
                    std::cout<<"processors : "<<i<<std::endl;
                    std::cout<<processor.cache;
                }
            }
            //send requests to other machines
            for(int i = 0; i < processors.size(); i++)
            {
                if(i != request.tid)
                {
                    processors[i].requests[request.tag] = request;
                }
            }
            for(int i = 0; i < processors.size(); i++)
            {
                if(i != request.tid)
                {
                    processors[i].pull_request(request.tag);
                }
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
            case State::M:
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
            case State::S:
                if(tsn == BusTsnType::BusRd)
                {
                    //no action
                }
                if(tsn == BusTsnType::BusRdX)
                {
                    cline.state = State::I;
                }
                break;
            case State::I:
                // no action
                break;
        }
        cache.put(tag, cline);
        requests.erase(it);
    }
}
void Processor::wait_request(void *tag)
{
    while(requests.find(tag) != requests.end());
}
void Processor::read(void *addr, int cnt = 0)
{
    void *tag = convert(addr);
    wait_request(tag);
    
    auto cline = cache.get(tag, true);
    cline.cnt = cnt;

    bus->push(cline, tid, PrTsnType::PrRd);

}
void Processor::write(void *addr, int cnt = 0)
{
    void *tag = convert(addr);
    wait_request(tag);
    
    auto cline = cache.get(tag, true);
    cline.cnt = cnt;
    bus->push(cline, tid, PrTsnType::PrWr);
    cline.tag = tag;
    cache.put(addr, cline);
}