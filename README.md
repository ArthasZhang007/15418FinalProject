# 15418 Final Project

# project link 
https://arthaszhang007.github.io/15418FinalProject/
# project pdf version

# project video link


Lingxi Zhang (lingxiz), Xinyu Wu (xwu3)

**The following is the project midpost, everything related to the final project is above.**

# Summary

We are aiming for implementing a snooping based memory coherence protocol software simulation instead of the real hardware one, as well as basic cache implementation for performance testing and comparison. 

# Background

In computer architecture, cache coherence is the uniformity of shared resource data that ends up stored in multiple local caches. When clients in a system maintain caches of a common memory resource, problems may arise with incoherent data, especially with CPUs with a multiprocessing system. To ensure the correctness of parallel programs, several memory coherence protocols are implemented. Among all of them the most famous ones are MSI and MESI. Design and implementation detail of the protocol greatly influences the memory usage and speed of the communication system, and in many cases we want to know when and how to choose the right protocol. However, sometimes hardwares are limited, and as students we do not have as much resources. Therefore, a software simulation program may become handy in evaluating the performance and help us choose the correct design.

For the process, first we want to create a LRU cache with tunable parameters like Cache Line Size and Cache capacity. Then we will try to create a bus, as well as state transitions with an MSI and/or MESI protocol. Next, we will try to write some basic read/write programs with different extent of false sharing and wrap it with our cache implementation as the input to our simulation program. Finally, we can analyze how different parameters, like protocol choices, cache line sizes, thread numbers influence the amount of bus traffic and invalidations.  

Here is a summarizing figure of our project pipeline:

<img width="598" alt="Screen Shot 2022-03-22 at 12 27 32 PM" src="https://user-images.githubusercontent.com/53051688/159527729-30544f7a-bd7e-4ce9-ab41-67c31a99e2ae.png">

# Challenge

Since we strive to simulate the actual behaviors of snooping-based cache coherence protocol, the challenge in parallelizing our programs lies within using a multiprocessing API (e.g. OpenMP) to simulate how caches of different cores are trying to access/manipulate data on the same cache line. Meanwhile, we need to report to users how the cache is behaving given timestamps.

Writing the correct MSI protocol is probably already hard enough, but another main challenge is how we generate good programs that distinguish good and bad protocol design. 

Note that parallelism is only a small part of our input programs(and we will try to create simple parallel programs at first), which is not the main focus of our project.

By completing this project, we hope to learn more about snooping-based cache coherence protocol and explore how parameters of the cache itself affect efficiency of this protocol. 

# Resources

For cache implementation, we are going to refer to how a cache simulator is implemented for 15-213 cache lab, including details about dirty bits, LRU, etc. 

For the parallelism part of the project, we are likely going to use OpenMP or just pthreads.

For data sets for our project, it is basically a sequence of read and write operations to data stored in memory. 

For machines, we want to use GHC machines 47-86 for code development and testing, probably PSC as well for a higher amount of cores. 

# Goals and Deliverables
## 75% target

Getting a functional snooping-based protocol implemented, as well as basic LRU cache. For the input we can manually order cache evictions and invalidations. For instance, inputs like processor 1 write to cache line #43 which invalidates processor 4 which also holds cache line #43. 

## 100% target

Getting a correct snooping-based protocol implemented, as well as LRU cache. Write some basic parallel programs that include false sharing and convert the programs into a sequence of reads and writes to the Cache Input. Test the coherence protocol under different input programs and report the number of invalidations and bus traffic. 

## 125% target

Getting a correct snooping-based protocol implemented, as well as LRU cache. Write some basic parallel programs that include false sharing and convert the programs into a sequence of reads and writes to the Cache Input. Test the coherence protocol under different input programs and report the number of invalidations and bus traffic. Change the cache line size, cache capacity, thread number, and even policies (like MESI, write back v.s. Write through) to compare and analyze the merits and weakness of different protocols.

# Schedule
## Week 3/28-4/3:
Simple write-back, write-allocate cache simulator with LRU
## Week 4/4-4/10:
MSI snooping simulator

4/6 PIN:https://software.intel.com/sites/landingpage/pintool/docs/98547/Pin/html/index.html#MAddressTrace
## Week 4/11-4/17:
4/11: checkpoint 

Integrate LRU cache with MSI together, try some basic read/ write inputs to test the validity of cache and MSI.
## Week 4/18-4/24:
Write some basic parallel programs using pthreads or openMP and measure the number of cache misses, invalidations, and bus traffic.
## Week 4/25-5/1:
Experiment and analyze our program with varying parameters like cache line size, cache capacity, thread number, and maybe with some parallel programs with complex data accessing patterns.

4/29: report

# Milestone Report

## Work Done
We basically finished the MSI cache snooping simulator as well as the LRU Cache. The Cacheline size is determined at compile time while the number of cachelines per processor and the number of processors are given as command line arguments.
We tested the correctness of our simulator on the example given in 11_coherence1.pptx slides 29, basically a sequence of write and read operations by different threads on different addresses.

We also successfully use the Intel pin to generate a memory reference footprint trace of a very simple parallel program and make sure the length of trace scales sequentially with the complexity of the code by filtering register related memory traces, but we have not actually fed the trace into our snooping simulator yet.  

## Adjustment for Goals and Deliverables
We are almost done with our 75% goal, which is implementing a snooping-based cache coherence protocol with LRU implementation for cache misses. The next step in our goals is to test our cache on both programs without false sharing and programs with false sharing. For the testing of these programs, we are going to focus on comparison between the numbers of invalidations. 

We are on track of our goals, and will be working towards our 100% and 125% goals. 

## Poster Session
We will be showing our sample input and output to help the audience understand our internal structure of simulator. Furthermore, we will have graphs that show the benchmarking results about the relations between parameters including cache line size, number of cache lines per processor, number of invalidations and flushes, and maybe implementation details including different protocols and different cache implementation decisions (such as write-back / write-through).

## Preliminary Results
![Screen Shot 2022-04-11 at 5 01 21 PM](https://user-images.githubusercontent.com/53051688/162842802-2cbdda5d-3873-413a-b892-a90fa3c69ec7.jpg)

The preliminary results for memory reference footprint traces are included in the samples folder, where two simple programs (one sequential and one parallel) are included, as well as their memory trace returned by PIN tools. 

The sample input and output for the MSI simulator is in input.txt and output.txt in the root folder, 

1 R X    represents thread 1 read on address X(0x10000000)

0 W Y   represents thread 2 write on address Y(0x10000400)

This represents at timestamp 11, a BusRdX on address 0x10000400 is caused by thread 0 and below are the local cache content in each processor. 

timestamp : 11  tid : 0 address : 0x10000400 BusRdX 

processors : 0

 Modified 0x10000400

 Shared   0x10000000

processors : 1

 Invalid  0x10000400

 Shared   0x10000000

## Concerns
The LRU cache in the MSI simulator does not hold the actual data for reading and writing. Therefore, if we want to evaluate the difference of write-back and write-through policy later, as well as the real efficiency of our simulator, we can not use the execution time as the benchmark but rather time-related tokens that are correlated to the time cost in real scenario like the number of bus transactions, invalidations, coherence miss, and flushes.

Another concern is the concurrency issue, since the bus transaction queue is handled separately by another thread, there is some latency for the bus transaction. For instance, it is entirely possible that thread A PrWr on address X, then the bus issues a busRdX on X. However, thread B PrRd on address X in shared state after the PrWr of threadA but before the busRdX reaches thread B. We are still figuring out the solution to this problem.

Last concern is about the proof of correctness. We can only test and debug the correctness of our simulator on a small sample by reasoning the process manually. However, on any large trace set there is no good way to test the correctness automatically and we can only judge by the trend.

# Final Report

## Project Summary

## Background and Workload 

## System Design and Data Structures

## Results 

## References

## Work Distribution
