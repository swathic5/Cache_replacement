# Cache_replacement
Perceptron based reuse prediction for last level caches implementation using C++

This infrastructure is from cache replacement and bypass competition for CSCE 614, Fall 2017. This infrastructure uses a simple linear performance model to translate misses to cycles. It is based on the 2010 JILP CRC infrastructure but replaces the cache simulator with a simple model that only tracks last-level cache accesses. It is configured to simulate a 4MB last-level cache.

The replacement_state.cpp and replacement_state.h files are modified to incorporate perceptron based reuse prediction policy. These files already come with LRU and random implemented as policies numbers 0 and 1. Perceptron policy is number two. To get the simulator to use perceptron policy instead of LRU, set the environment variable DAN_POLICY to 2. For example, in Bourne shell or bash, you would write:

export DAN_POLICY=2; ./efectiu <trace-file-name>.gz

5 traces from SPEC CPU 2006 have been provided in the "traces" directory. These traces are the last-level cache accesses for one billion instructions on a machine with a three-level cache hierarchy where the first level is 32KB split I+D and, level is a unified 256KB, and the third level is the 4MB cache we are optimizing. Goal is to maximize geometric mean speedup over LRU. That is, for each benchmark compute the IPC with perceptron technique, divide that by the IPC from LRU to get the speedup, then take the geometric mean of all the speedups. All these steps are taken care by the run script. Once the code is built, just execute the below command in efectiu directory to run the code with both LRU and perceptron policies to compute the overall speedup:

./run.sh

A report with the results is also provided.