#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "cache.h"



class Processor
{
    public:
        std::map<void*, Cacheline> cachelines;
};
class Bus{
    public:
        State state;
};
class MSI
{
public:
    std::vector<Processor> processors;
    Bus bus;
    MSI(int nthreads)
    {
        processors.resize(nthreads);
    }
};