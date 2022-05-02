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
    
    Value get(Key key, bool create_new = false) {
        auto it = hash_map_.find(key);
        if(it == hash_map_.end())
        {
            auto def_val = Value(key);
            if(create_new)put(key, def_val);
            return def_val;
        }
        else 
        {
            auto ans = it->second->second;
            promote(it->second);
            return ans;
        }
    }

    void put(Key key, Value value) {
        auto it = hash_map_.find(key);
        if(it == hash_map_.end())
        {
            if(hash_map_.size() == cap)
            {
                auto p = list_.back();
                list_.pop_back();
                hash_map_.erase(p.first);

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