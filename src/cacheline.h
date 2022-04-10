#pragma once
#include "consts.h"
#include <iostream>

struct Cacheline
{
    Cacheline():state(State::I){}
    bool dirty_bit;
    State state;
    void* tag;
    char data[CachelineSize];
};
std::ostream& operator <<(std::ostream &os, Cacheline cline) {
    switch(cline.state)
    {
        case 'M':
            os<<" Modified ";
            break;
        case 'S':
            os<<" Shared   ";
            break;
        case 'I':
            os<<" Invalid  ";
            break;
    }
    os<<cline.tag<<std::endl;
    return os;
}