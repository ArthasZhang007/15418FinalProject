#include <pthread.h>
#include <bits/stdc++.h>
#include "msi.h"
#include <unistd.h>

using namespace std;
char *inputFilename = NULL;
int nthreads = 1;
int NCachelines = 30;

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
void inputloop(Bus &bus)
{
    /* initialization of file input and address*/
    freopen(inputFilename, "r", stdin);
    std::map<std::string, void*> addr_map;
    void* base_ptr = (void *)0x10000000;
    
    //main loop of inputing traces
    int cnt = 0;
    while(!std::cin.eof())
    {
        int tid;
        char type;
        std::string address;

        std::cin>>tid>>type>>address;
        //std::cout<<tid<<' '<<type<<' '<<address<<std::endl;
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
        bus.processors[i]->print_stats();
    }
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


    inputloop(bus);
    

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