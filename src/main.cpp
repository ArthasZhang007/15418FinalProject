#include <pthread.h>
#include <bits/stdc++.h>
#include "msi.h"
#include <unistd.h>

using namespace std;
char *inputFilename = NULL;
int nthreads = 1;
int NCachelines = 30;

string default_input_dir = "../samples";
string default_output_dir = "../sampleoutput";


void* bus_loop(void *args)
{
    Bus * bus = (Bus *)(args);
    bus->mainloop();
    return nullptr;
}
void* processor_loop(void *args)
{
    std::shared_ptr<Processor> processor = std::shared_ptr<Processor>((Processor *)args);
    processor->mainloop();
    return nullptr;
}
void sample_in(int *tid, char *type, std::string * address)
{
    std::cin>>(*tid)>>(*type)>>(*address);
}

void inputloop(Bus &bus)
{
    /* initialization of file input and address*/
    
    std::map<std::string, void*> addr_map;
    void* base_ptr = (void *)0x10000000;
    
    //main loop of inputing traces
    int cnt = 0;
    while(!std::cin.eof())
    {
        int tid; char type; std::string address;
        sample_in(&tid, &type, &address);

        if(addr_map.find(address) == addr_map.end())
        {
            addr_map[address] = base_ptr;
            base_ptr += 1024;
        }
        
        //std::cout<<tid<<' '<<type<<' '<<addr_map[address]<<std::endl;
        bus.processors[tid]->add_trace(Trace(addr_map[address], type, ++cnt));
        //sleep(0.1);
    }
    
    sleep(3);
    for(int i=0;i<nthreads;i++)
    {
        bus.processors[i]->push_stats();
        bus.processors[i]->print_stats();
    }
    exit(0);
}


void realloop(Bus &bus)
{
    /* initialization of file input and address*/
    
    std::unordered_map<int, int> tid_map;
    std::vector<int> anti_map;
    int num_tids = 0; 
    int cnt = 0;
    while(!std::cin.eof())
    {
        //int tid; char type; std::string address;
        //sample_in(&tid, &type, &address);
        void *addr;char type;int thread_rid;
        std::string waste;
        //std::string addr;
        std::cin>>addr>>type>>type>>addr>>waste>>waste>>thread_rid;

        if(tid_map.find(thread_rid) == tid_map.end())
        {
            tid_map[thread_rid] = num_tids;
            anti_map.push_back(thread_rid);
            num_tids ++;
        }
        int tid = tid_map[thread_rid];
        //std::cout<<addr<<' '<<type<<' '<<thread_rid<<std::endl;
        //std::cout<<tid<<' '<<type<<' '<<addr_map[address]<<std::endl;
        //std::cout<<addr<<' '<<tid<<' '<<num_tids<<std::endl;
        bus.processors[tid]->add_trace(Trace(addr, type, ++cnt));
        //sleep(0.1);
    }
    
    sleep(5);
    MetaStat total;
    for(int i=0;i<nthreads;i++)
    {
        bus.processors[i]->push_stats();
        total = total + bus.processors[i]->stat;
        //bus.processors[i]->print_stats();
    }
    int sum = total.cold_misses + total.capacity_misses + total.coherence_misses;
    std::cout<<total<<std::endl;
    std::cout<<"total miss rate : "<< (double)sum/cnt*100<<"%\n";
    std::cout<<"coherence miss rate : "<< (double)total.coherence_misses/cnt*100<<"%\n";
    std::cout<<"coherence miss percentage : "<<(double)total.coherence_misses/sum*100<<"%\n";
    
    
    exit(0);
}

void execute()
{
    /* initialization of multithreading */
    Bus bus(nthreads, NCachelines);
    std::vector<pthread_t> pthread_id(nthreads + 1);
    pthread_create(&pthread_id[nthreads], nullptr, bus_loop, (void*)(&bus));
    for(int i=0;i<nthreads;i++)
    {
        auto processor = bus.processors[i];
        //std::cout<<i<<' '<<(&processor)<<' '<<processor.tid<<std::endl;
        pthread_create(&pthread_id[i], nullptr, processor_loop, (void*)(processor.get()));
    }


    std::string inputFile = default_input_dir + '/' + inputFilename;
    std::string outputFile = default_output_dir + '/' + inputFilename;

    freopen(inputFile.c_str(), "r", stdin);
    freopen(outputFile.c_str(), "w", stdout);
    
    //inputloop(bus);
    realloop(bus);
    

    /* waiting the thread to terminate (never) */
    for(int i=0;i < nthreads + 1; i++)
        pthread_join(pthread_id[i], NULL);
    
}
int parse_command(int argc, char **argv)
{
    if(argc < 2)
    {
        std::cerr<<"not enough arguments!";
        return -1;
    }
    int opt = 0;
    do {
        opt = getopt(argc, argv, "f:n:i:");
        switch (opt) {
        case 'f':
            inputFilename = optarg;
            break;

        case 'n':
            nthreads = atoi(optarg);
            break;

        case 'i':
            NCachelines = atoi(optarg);
            break;

        case -1:
            break;

        default:
            break;
        }
    } while (opt != -1);
    return 0;
}
int main(int argc, char ** argv)
{
    parse_command(argc, argv); 
    //std::cout<<inputFilename<<' '<<nthreads<<' '<<NCachelines<<std::endl;
    execute();
    
    return 0;
}