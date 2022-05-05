<script src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML" type="text/javascript"></script>
# Snooping Based Coherence Simulator

In this project, we implement a software simulator of the Snooping Based Coherence Protocol with the basic MSI state transitions. We test the correctness of our program on the ghc77 machines using the examples from the 418 lecture first to make sure the simulator is sequentially consistent, then benchmark our simulator with programs that have different memory access patterns. Our final deliverables include graphs and analysis between the relationship among several important independent variables and dependent variables. The final trend fits our 
general prediction, which practically shows the correctness of our simulator.

# Background 

In computer architecture, cache coherence is the uniformity of shared resource data that ends up stored in multiple local caches. When clients in a system maintain caches of a common memory resource, problems may arise with incoherent data, especially with CPUs with a multiprocessing system. To ensure the correctness of parallel programs, several memory coherence protocols are implemented. Among all of them the most famous ones are MSI and MESI. Design and implementation detail of the protocol greatly influences the memory usage and speed of the communication system, and in many cases we want to know when and how to choose the right protocol. However, sometimes hardwares are limited, and as students we do not have as much resources. Therefore, a software simulation program may become handy in evaluating the performance and help us choose the correct design.

## Key Data Structures 

### Cacheline 

```
struct Cacheline
{
    Cacheline():tag(nullptr), state(State::I){}
    Cacheline(void *tg):tag(tg),state(State::I){}
    int cnt;
    State state;
    void* tag;
    //char data[CachelineSize];
};
```

```cnt``` is actually the timestamp of the operation in the trace. For instance, 
```cnt = 5``` means that is the 5th row from the memory trace input. This 
is for debugging and collecting statistics only.

```tag ``` is the address of this cacheline

```State``` M = Modified, S = Shared, I = Invalid

```data[CachelineSize]``` stores the actual content of the cacheline. We do not need 
it since our simulator does not involve real data transfers. 

Notice that CachelineSize is determined at compile time.

### LRU Cache 

The LRU Cache is a polymorphic. The key is the ```void *``` type, which indicates the 64 bits memory address, the value is simply the corresponding cacheline for this address. 

Lookup API:

```Value get(Key key, bool create_new = false)```

create_new means that if the entry is not in the cache, create a new one.

Insert API:

```void put(Key key, Value value)```


Lookup(Get) and Insert(Put) is $$O(1)$$, implemented using a linked list and hash table. 

### Processor Abstraction 

```
class Processor
{
public:
    Processor():bus(nullptr), stat(){}
    ~Processor(){} 
    void mainloop();

    void add_trace(Trace trace); // resize LRU cache
    void resize(int capacity){cache.resize(capacity);} 
    
    // read and write at address with timestamp cnt 
    void read(void *addr, int cnt);
    void write(void *addr, int cnt);
    void pull_request(); // handle the bus transactions from the request queue
    void flush(void *tag);// flush from cache to memory
    LRUCache<void*, Cacheline> cache;
    Bus *bus;
    int tid; // thread id 
    std::queue<BusTransaction> requests; // request queue
    std::queue<Trace> traces; // the trace(read/write at address x) queue
    // concurrent locks
    std::mutex trace_lock;
    std::mutex request_lock;
}
```

### Bus Abstraction
```
class Bus
{
public:
    void mainloop();

    // push a PrRd/PrWr from processor t for Cacheline c
    void push(Cacheline cline, int thread_id, PrTsnType pr_request, bool is_new);

    // send a BusRd/RdX to the target processors
    void process(BusTransaction request);

    std::vector<std::shared_ptr<Processor> > processors;

    // shared request queue
    std::queue<BusTransaction> requests;
    // bus lock for concurrency
    std::mutex bus_lock;
};
```

## Sample Debug Output
```
(...truncated)

coherence miss! by 0
timestamp : 7  tid : 0 address : 0x10000000 BusRd  
processors : 0
 Shared   0x10000000

processors : 1
 Invalid  0x10000000

timestamp : 8  tid : 0 address : 0x10000000 BusRdX 
processors : 0
 Modified 0x10000000

processors : 1
 Invalid  0x10000000

coherence miss! by 1
timestamp : 6  tid : 1 address : 0x10000000 BusRd  
processors : 0
 Modified 0x10000000

processors : 1
 Invalid  0x10000000

timestamp : 10  tid : 0 address : 0x10000400 BusRd  
processors : 0
 Shared   0x10000400

 Shared   0x10000000

processors : 1
 Invalid  0x10000000

timestamp : 11  tid : 0 address : 0x10000400 BusRdX 
processors : 0
 Modified 0x10000400

 Shared   0x10000000

processors : 1
 Invalid  0x10000000

coherence miss! by 1
timestamp : 9  tid : 1 address : 0x10000000 BusRd  
processors : 0
 Modified 0x10000400

 Shared   0x10000000

processors : 1
 Invalid  0x10000400

 Shared   0x10000000

timestamp : 12  tid : 1 address : 0x10000400 BusRdX 
processors : 0
 Modified 0x10000400

 Shared   0x10000000

processors : 1
 Modified 0x10000400

 Shared   0x10000000

-----Processor 0-----
cold misses : 2
capacity misses : 0
coherence misses : 1
flush : 3
-----Processor 0-----
-----Processor 1-----
cold misses : 2
capacity misses : 0
coherence misses : 3
flush : 1
-----Processor 1-----
```



# Approach

## Real Process Overview
![Image](sample.jpg)
### 1. Program ==> Trace
(Interleave access of 64 threads on array length of 256)
```
void *thread(void *cur_args) {
    
    args *casted = (args*)cur_args;
    for (int i = 0; i < 4; i++){
        casted->arr[casted->tid + i * 64] += 1;
    }

    pthread_exit(NULL);
}
```
==>
```
0x400c58: W 0x7fff1c83cfc0; TID: 31527
0x400c66: W 0x7fff1c83cfb8; TID: 31527
0x400c1b: W 0x7fff1c83cfb0; TID: 31527
0x400c23: W 0x7fff1c83cfac; TID: 31527
0x400c26: W 0x7fff1c83cfa8; TID: 31527
0x400c29: R 0x7fff1c83cfac; TID: 31527
0x400c2f: R 0x7fff1c83cfa8; TID: 31527
0x400c3d: W 0x7fff1c83cf98; TID: 31527
0x400830: R 0x602030; TID: 31527
0x400836: W 0x7fff1c83cf90; TID: 31527
0x4007f0: R 0x602008; TID: 31527
0x4007f0: W 0x7fff1c83cf88; TID: 31527
0x4007f6: R 0x602010; TID: 31527
(Instruction Address) : Read/Write (Memory Address), Thread ID
...
```
### 2. Create Bus and Processors in Separate Threads
Let us say the original program has n threads, then we need n+1 threads in total.
These threads call mainloop() that actively checks if their queues are empty.
If they are not, pull out a request and start to handle the request.
Processors have two queues, one is the trace queue and another is the bustransaction
queue.

Bus only has a bustransaction queue. Whenever there are any operations on the queue,
mutex locks are applied to avoid concurrent faults. 


### 3. Feed Trace to Processors Queue 
```
bus.processors[tid]->add_trace(Trace(addr, rw_type, ++cnt));
```
Notice the inqueue time is in sequential order since the we can not read the trace file in parallel. However, since each processor is in separate thread, the actual processing of the requests among the processors are not necessarily obeying the 
sequential trace order. The order inside each processor is still correct since the queue is FIFO. We believe that, within our range of abilities, this is the most accurate presentation of the data access pattern in real applications.

### 4. Processor Read and Write

Before any other actions, processor locks the bustransaction queue and handle the 
bus transactions(originated in other processors) first until the bus transaction queue is empty. 

This process followed
the MSI diagram. 
A ```BusRd``` makes the state to change from Modified to Shared, ```BusRdX``` may make some cacheline invalid, etc. 

Then, the processor updates its local LRU Cache accordingly.

Finally, the processor calles the Bus to push ```PrRd``` or ```PrWr``` on to the bus transaction queue of the Bus. The Bus will obtain a bus lock on the queue, generates corresponding Bus transaction, and sends the transaction to every other processor. The other processors must handle all the bus transactions in their queue, in the very beginning of the next read or write operations as described in the first paragraph of this section.
## Other Side Details
### Cacheline Mapping 
```#define convert(x) (void*)((long long)(addr) / CachelineSize * CachelineSize)```

For instance, let us say the cacheline size is 16 bit. 
Then address from 64 to 79 will be mapped to 64 and they will be in the same 
cacheline.



### Intel Pin Tools

### Parallelism 
 We use posix thread(pthread) for both the input program and the MSI simulator. The process is just creating different pthreads using `pthread_create` in the beginning and waiting them to join in the end.





# Results

There are so many independent and dependent variables availble for our cache simulator, hence we only selected some of interest.

## Dependent variables 
### number of cold misses 
Measured in LRU Cache.
If a lookup does not find entry in the cache, that is a cold miss. 
### number of capacity misses 
Measured in LRU Cache.
If an insert evicts an entry, that is a capacity miss.
### number of coherence misses 
Measured in ```bus::push()```

A coherence misses whenever a cacheline changes from invalid state to 
shared or modified state, AND the cacheline is not new to the cache, because there are implicit and temporal invalid start state for cachelines that the cache does not have.

### number of flushes 
Measured in ```processor::pull_request()```

A flush happenes whenever a cacheline changes from modified state to the 
shared or invalid state. This is kind of the opposite from coherence miss.

## Independent variables 

### Number of Processors(Threads)

Range from 2 to 64. Since in Snooping-Based Caching, communication complexity scales squarely with the number of threads. 64 is a realistic and feasible upperbound for us.

### Number of Cachelines
How big a cache in a single processor is? This affects the number of cold and capacity misses most.
### The Cacheline size
How many bytes a cacheline has? Our defaults and center point is 64 bytes.

### The Original Program itself
How big is the array? What is the intensity of contentions in our program?
What is the data access pattern? Block or Interleave, or more complex pattern?

## Experiments Configurations && Graphs
Our main program is to using different thread to access the array element and modify it. One division is blocking and another division is interleaving. Our default setting is 64 bytes and 512 cachelines, so the total cache size is 32KB which is close to the real L1 cache configurations.

![drawing](blockvinter_10access.png){width=50%}




![Image](coherence_miss_v_diff_thread_number.png)
![Image](flush_v_diff_thread_number.png)
![Image](x=cache_line_v_coh_flush_block.png)
![Image](x=cache_line_v_coh_flush_interleave.png)
![Image](x=cache_line_v_cold_capacity_block.png)
![Image](x=cache_line_v_cold_capacity_interleave.png)

## Analysis 


# Reference

- Intel Pin User Guide (https://software.intel.com/sites/landingpage/pintool/docs/98547/Pin/html/index.html#MAddressTrace)
- CMU 15418 Snooping Implementation https://www.cs.cmu.edu/afs/cs/academic/class/15418-s22/www/lectures/12_snoopimpl.pdf
- 


# Contribution of Works

## 50%(Lingxi) - 50%(Xinyu)   ? 

## Lingxi Zhang

- Design and Implement the MSI simulator from the scratch
- Test the correctness of the simulator following the example from the 418 lecture
- Debug segfaults and concurrency errors
- Choose core statistics and come up ways of how to catch these statistics in our simulator 

## Xinyu Wu

- Find out how to convert binary program to read/write trace using Intel Pin
- Filter out registers related read or write since they are not related to cache
- Filter out as much memory access, related to library preprocessing/postprocessing, as possible by specifying a region of interest around core program
- Make the graph and table of statistics

## Together 
- Come up and benchmark test cases using different variables
- Find the common trend and source of errors/turbulance among the benchmarking output 
- Analyze and summarize our discovery  