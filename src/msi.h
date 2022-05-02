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
// read write trace
struct Trace{
    Trace(){}
    Trace(void *cline, char typ, int ct) : 
        tag(cline), type(typ), cnt(ct){}
    void *tag;
    char type;
    int cnt;
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
    Processor():bus(nullptr){
        cold_misses = 0;
        capacity_misses = 0;
        coherence_misses = 0;
        flushes = 0;
    }
    ~Processor(){} 
    void mainloop();
    void add_trace(Trace trace);
    void resize(int capacity){cache.resize(capacity);}  
    void read(void *addr, int cnt);
    void write(void *addr, int cnt);
    void pull_request(void *tag); // from the bus
    void wait_request(void *tag); // on the bus

    void flush(void *tag);// flush from cache to memory
    void print_stats()
    {
        std::cout<<"-----Processor "<<tid<<"-----"<<std::endl;
        std::cout<<"cold misses : "<< cache.cold_misses<<std::endl;
        std::cout<<"capacity misses : " << cache.capacity_misses<<std::endl;
        std::cout<<"coherence misses : " << coherence_misses<<std::endl;
        std::cout<<"flush : " << flushes<<std::endl;
        std::cout<<"-----Processor "<<tid<<"-----"<<std::endl;
    }

    /*main data structures*/
    LRUCache<void*, Cacheline> cache;
    Bus *bus;
    int tid;
    std::map<void*, BusTransaction> requests;
    std::queue<Trace> traces;
    std::mutex trace_lock;


    /*benchmarking stats*/
    int cold_misses;
    int capacity_misses;
    int coherence_misses;
    int flushes;
};

class Bus{
public:
    Bus(int nthreads = 8, int NCachelines = 30)
    {
        processors.resize(nthreads);
        for(int i = 0; i < nthreads; i++)
        {
            processors[i] = std::make_shared<Processor>();
            processors[i]->resize(NCachelines);
            processors[i]->bus = this;
            processors[i]->tid = i;
        }
    }
    ~Bus(){}
    void mainloop();

    // push a PrRd/PrWr from processor t for address c
    void push(Cacheline cline, int thread_id, PrTsnType pr_request);
    // send a BusRd/RdX to the target processors
    void process(BusTransaction request);

    std::vector<std::shared_ptr<Processor> > processors;
    
    // shared request queue
    std::queue<BusTransaction> requests;
    // bus lock for concurrency
    std::mutex bus_lock;
};
void Processor::flush(void *tag)
{
    flushes++;
}
//from remote bus to local
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
                    flush(tag);
                }
                if(tsn == BusTsnType::BusRdX)
                {
                    cline.state = State::I;
                    flush(tag);
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
    cline.tag = tag;
    cache.put(addr, cline);
    bus->push(cline, tid, PrTsnType::PrWr);

}

void Processor::add_trace(Trace trace)
{
    trace_lock.lock();
    traces.push(trace);
    trace_lock.unlock();
}
void Processor::mainloop()
{
    //std::cout<<tid<<std::endl;
    while(true) 
    {
        while(!traces.empty())
        {
            trace_lock.lock();

            Trace q = traces.front();
            traces.pop();
            if(q.type == 'R') read(q.tag, q.cnt);
            if(q.type == 'W') write(q.tag, q.cnt);
            
            trace_lock.unlock();
        }
    }
}



void Bus::mainloop()
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

void Bus::push(Cacheline cline, int thread_id, PrTsnType pr_request)
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
            processors[thread_id]->coherence_misses++;
            has_response = true;
            break;
    }
    //if(cline.state == State::M)std::cout<<"freak\n";
    processors[thread_id]->cache.put(cline.tag, cline);
    //std::cout<<processors[thread_id].cache<<std::endl;
    if(has_response)
    {
        requests.push(BusTransaction(cline.tag, thread_id, tsn_type, cline.cnt));
    }
    
    bus_lock.unlock();
}

void Bus::process(BusTransaction request)
{
    #pragma critical
    {
        std::cout<<request<<std::endl;
    }
    //send requests to other machines
    for(int i = 0; i < processors.size(); i++)
    {
        if(i != request.tid)
        {
            processors[i]->requests[request.tag] = request;
        }
    }
    for(int i = 0; i < processors.size(); i++)
    {
        if(i != request.tid)
        {
            processors[i]->pull_request(request.tag);
        }
    }
    #pragma critical
    {
        for(int i = 0; i < processors.size(); i++)
        {
            std::cout<<"processors : "<<i<<std::endl;
            std::cout<<processors[i]->cache;
        }
    }
}