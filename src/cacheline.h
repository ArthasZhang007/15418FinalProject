#pragma once
#include "consts.h"
#include <iostream>

struct Cacheline
{
    Cacheline():tag(nullptr), state(State::I){}
    Cacheline(void *tg):tag(tg),state(State::I){}
    int cnt;
    State state;
    void* tag;
    //char data[CachelineSize];
};
std::ostream& operator <<(std::ostream &os, Cacheline cline) {
    switch(cline.state)
    {
        case State::M:
            os<<" Modified ";
            break;
        case State::S:
            os<<" Shared   ";
            break;
        case State::I:
            os<<" Invalid  ";
            break;
    }
    os<<cline.tag<<std::endl;
    return os;
}