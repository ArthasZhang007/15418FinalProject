<script src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML" type="text/javascript"></script>

# Snooping Based Coherence Simulator

In this project, we implement a software simulator of the Snooping Based Coherence Protocol with the basic MSI state transitions. We test the correctness of our program on the ghc77 machines using the examples from the 418 lecture first to make sure the simulator is sequentially consistent, then benchmark our simulator with programs that have different memory access patterns. Our final deliverables include graphs and analysis between the relationship among several important independent variables and dependent variables. The final trend fits our 
general prediction, which practically shows the correctness of our simulator.

# Background 

In computer architecture, cache coherence is the uniformity of shared resource data that ends up stored in multiple local caches. When clients in a system maintain caches of a common memory resource, problems may arise with incoherent data, especially with CPUs with a multiprocessing system. To ensure the correctness of parallel programs, several memory coherence protocols are implemented. Among all of them the most famous ones are MSI and MESI. Design and implementation detail of the protocol greatly influences the memory usage and speed of the communication system, and in many cases we want to know when and how to choose the right protocol. However, sometimes hardwares are limited, and as students we do not have as much resources. Therefore, a software simulation program may become handy in evaluating the performance and help us choose the correct design.

## Key Data Structures 

### Cacheline 
### LRU Cache 

The LRU Cache is a polymorphic. The key is the void *   
### Processor Abstraction 

### Bus Abstraction

# Approach

### Cacheline Mapping 


### Intel Pin Tools
### Parallelism 
 pthread
### Statistics Collecting Methods



# Results

# Conclusion && Reference

# Contribution of Works

### Markdown

Markdown is a lightweight and easy-to-use syntax for styling your writing. It includes conventions for

```markdown
Syntax highlighted code block

# Header 1
## Header 2
### Header 3

- Bulleted
- List

1. Numbered
2. List

**Bold** and _Italic_ and `Code` text

[Link](url) and ![Image](src)
```

For more details see [Basic writing and formatting syntax](https://docs.github.com/en/github/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax).

### Jekyll Themes

Your Pages site will use the layout and styles from the Jekyll theme you have selected in your [repository settings](https://github.com/ArthasZhang007/15418FinalProject/settings/pages). The name of this theme is saved in the Jekyll `_config.yml` configuration file.

### Support or Contact

Having trouble with Pages? Check out our [documentation](https://docs.github.com/categories/github-pages-basics/) or [contact support](https://support.github.com/contact) and we’ll help you sort it out.
