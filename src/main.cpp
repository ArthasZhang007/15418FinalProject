#include <pthread.h>
#include <bits/stdc++.h>
#include "msi.h"
#include <unistd.h>

using namespace std;
char *inputFilename = NULL;
int nthreads = 1;
int NCachelines = 30;
void* compute(void *args)
{
    Bus * it = (Bus *)(args);
    it->mainloop();
    return nullptr;
}
void handle(Bus * bus, int tid, char type, void *addr, int cnt)
{
    //std::cout<<bus->processors.size()<<' '<<tid<<std::endl;
    auto &processor = bus->processors[tid];
    if(type == 'R')processor.read(addr, cnt);
    if(type == 'W')processor.write(addr, cnt);
}
void execute()
{
    Bus bus(nthreads, NCachelines);
    pthread_t pthread_id;
    std::map<std::string, void*> addr_map;
    void* base_ptr = (void *)0x10000000;


    pthread_create(&pthread_id, nullptr, compute, (void*)(&bus));


    freopen(inputFilename, "r", stdin);
    int cnt = 0;
    while(!std::cin.eof())
    {
        int tid;
        char type;
        std::string address;
        std::cin>>tid>>type>>address;
        if(addr_map.find(address) == addr_map.end())
        {
            addr_map[address] = base_ptr;
            base_ptr += 1024;
        }
        //std::cout<<tid<<' '<<type<<' '<<addr_map[address]<<std::endl;
        handle(&bus, tid, type, addr_map[address], ++cnt);
        //sleep(0.1);
    }
    pthread_join(pthread_id, NULL);
}
int main(int argc, char ** argv)
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

    execute();
    std::cout<<inputFilename<<' '<<nthreads<<' '<<NCachelines<<std::endl;
    return 0;
}