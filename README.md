# 15418 Final Project
It is what it is.

Lingxi Zhang (lingxiz), Xinyu Wu (xwu3)

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

## Adjustment for Goals and Deliverables

## Poster Session

## Preliminary Results

## Concerns

# Final Report

## Project Summary

## Background and Workload 

## System Design and Data Structures

## Results 

## References

## Work Distribution