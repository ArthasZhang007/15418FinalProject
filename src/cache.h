#pragma once
#include "consts.h"
struct Cacheline
{
    bool dirty_bit;
    State state;
    void* tag;
    char data[CachelineSize];
};


class Cache{
public:
    Cache(int capacity);
    Cacheline get(void* tag);
    void put(void *tag, char* new_data);
};