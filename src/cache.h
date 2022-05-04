#pragma once

#include <list>
#include <map>


template <typename Key, typename Value>
class LRUCache {
private:
    void promote(typename std::list<std::pair<Key, Value> >::iterator it)
    {
        std::pair<Key, Value> pa = *it;
        list_.erase(it);
        list_.push_front(pa);
        hash_map_[pa.first] = list_.begin();
    }
public:
    LRUCache():cap(5), cold_misses(0), capacity_misses(0){}
    LRUCache(int capacity):cap(capacity), cold_misses(0), capacity_misses(0){}

    void resize(int capacity){cap = capacity;}
    template <typename T>
    bool checkin(typename std::list<T>::iterator it, std::list<T> ls)
    {
        for(auto i = ls.begin(); i != ls.end(); i++)if(it == i)return true;
        return false;
    }
    template <typename T>
    bool checklast(typename std::list<T>::iterator it, std::list<T> ls)
    {
        for(auto i = ls.begin(); i != ls.end();)
        {
            auto last = i;

            i++;
            if(i == ls.end())
            {
                return it == last;
            }
        }
    }
    Value get(Key key, bool create_new = false) {
        auto it = hash_map_.find(key);
        auto def_val = Value(key);
        if(it == hash_map_.end())
        {
            
            if(create_new)
            {
                put(key, def_val);
                return def_val;
            }
            return Value();
        }
        else 
        {
            auto pair = *(it->second);
            
            if(it->second != list_.end())
            {
                auto ans = it->second->second;
                promote(it->second);
                return ans;
            }
            return Value();
        }
    }

    void put(Key key, Value value) {
        auto it = hash_map_.find(key);
        if(it == hash_map_.end())
        {
            if(hash_map_.size() == cap)
            {
                auto p = list_.back();
                //assert(checklast(hash_map_[p.first],list_));
                hash_map_.erase(p.first);
                list_.pop_back();
                

                capacity_misses ++;
            }
            else {
                cold_misses ++;
            }
            list_.push_front(std::make_pair(key,value));
            hash_map_[key] = list_.begin();
        }
        else
        {
            it->second->second = value;
            promote(it->second);
        }
    }
    

    std::list<std::pair<Key, Value> > list_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value> >::iterator > hash_map_;
    int cap;
    int cold_misses;
    int capacity_misses;
};

template <typename Key, typename Value>
std::ostream& operator <<(std::ostream &os, LRUCache<Key, Value> cache) {
    for(auto &it : cache.hash_map_)
    {
        os<<(*it.second).second<<std::endl;
    }
    return os;
}