#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include "cacheline.h"
#include "cache.h"
//#define DEBUG
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

struct MetaStat{
    MetaStat() : cold_misses(0), capacity_misses(0), coherence_misses(0), flushes(0){}
    int cold_misses;
    int capacity_misses;
    int coherence_misses;
    int flushes;
    friend MetaStat operator + (MetaStat a, MetaStat b)
    {
        MetaStat s;
        s.cold_misses = a.cold_misses + b.cold_misses;
        s.capacity_misses = a.capacity_misses + b.capacity_misses;
        s.coherence_misses = a.coherence_misses + b.coherence_misses;
        s.flushes = a.flushes + b.flushes;
        return s;
    }
};
std::ostream& operator <<(std::ostream &os, MetaStat s) {
    os<<"cold misses : "<< s.cold_misses<<std::endl;
    os<<"capacity misses : " << s.capacity_misses<<std::endl;
    os<<"coherence misses : " << s.coherence_misses<<std::endl;
    os<<"flush : " << s.flushes<<std::endl;
    return os;
}
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
    Processor():bus(nullptr), stat(){}
    ~Processor(){} 
    void mainloop();
    void add_trace(Trace trace);
    void resize(int capacity){cache.resize(capacity);}  
    void read(void *addr, int cnt);
    void write(void *addr, int cnt);
    void pull_request(); // from the bus
    void wait_request(void *tag); // on the bus

    void flush(void *tag);// flush from cache to memory
    void push_stats()
    {
        stat.cold_misses = cache.cold_misses;
        stat.capacity_misses = cache.capacity_misses;
    }
    void print_stats()
    {
        std::cout<<"-----Processor "<<tid<<"-----"<<std::endl;
        std::cout<<stat;
        std::cout<<"-----Processor "<<tid<<"-----"<<std::endl;
    }

    /*main data structures*/
    LRUCache<void*, Cacheline> cache;
    Bus *bus;
    int tid;
    std::queue<BusTransaction> requests;
    //std::map<void*, BusTransaction> requests;
    std::queue<Trace> traces;
    std::mutex trace_lock;
    std::mutex request_lock;


    /*benchmarking stats*/
    MetaStat stat;
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
    void push(Cacheline cline, int thread_id, PrTsnType pr_request, bool is_new);
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
    stat.flushes++;
}
//from remote bus to local
void Processor::pull_request()
{
    request_lock.lock();
    while(!requests.empty())
    {
        auto request = requests.front();
        requests.pop();
        auto tsn = request.tsn;
        auto tag = request.tag;
        auto cline = cache.get(tag, false);
        if(cline.tag != nullptr)
        {
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
                        //std::cout<<"invalidate M\n";
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
                        //std::cout<<"invalidate S\n";
                        cline.state = State::I;
                    }
                    break;
                case State::I:
                    // no action
                    break;
            }
            cache.put(tag, cline);
        }
    }
    request_lock.unlock();   
}
void Processor::read(void *addr, int cnt = 0)
{
    void *tag = convert(addr);
    pull_request();
    //wait_request(tag);
    
    auto res = cache.get(tag, false);

    auto cline = cache.get(tag, true);
    cline.cnt = cnt;

    bus->push(cline, tid, PrTsnType::PrRd, res.tag == nullptr);

}
void Processor::write(void *addr, int cnt = 0)
{
    void *tag = convert(addr);
    pull_request();
    //wait_request(tag);
    auto res = cache.get(tag, false);

    auto cline = cache.get(tag, true);
    cline.cnt = cnt;
    cline.tag = tag;
    cache.put(addr, cline);
    bus->push(cline, tid, PrTsnType::PrWr, res.tag == nullptr);

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

void Bus::push(Cacheline cline, int thread_id, PrTsnType pr_request, bool is_new)
{
    bus_lock.lock();

    BusTsnType tsn_type;
    bool has_response = false;
    
    /*if(is_new == false)
    {
        std::cout<<cline.state<<" freak\n";
    }*/
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
            if(!is_new)
            {
                std::cout<<"coherence miss! by "<<thread_id<<std::endl; 
                processors[thread_id]->stat.coherence_misses++;
            }
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
#ifdef DEBUG
    #pragma critical
    {
        std::cout<<request<<std::endl;
    }
#endif
    //send requests to other machines
    for(int i = 0; i < processors.size(); i++)
    {
        if(i != request.tid)
        {
            processors[i]->request_lock.lock();
            processors[i]->requests.push(request);
            processors[i]->request_lock.unlock();
        }
    }
#ifdef DEBUG
    #pragma critical
    {
        for(int i = 0; i < processors.size(); i++)
        {
            std::cout<<"processors : "<<i<<std::endl;
            std::cout<<processors[i]->cache;
        }
    }
#endif
}