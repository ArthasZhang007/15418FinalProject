#pragma once
#include "consts.h"
struct Cacheline
{
    Cacheline():state(State::I){}
    bool dirty_bit;
    State state;
    void* tag;
    char data[CachelineSize];
};