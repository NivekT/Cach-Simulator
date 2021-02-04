# Cache-Simulator

csim.c a cache simulator that takes a valgrind memory trace as input. 
Using the LRU (least-recently used) replacement policy, it will simulate the hit or miss behavior 
of a cache memory on this trace, and returns the total number of hits, misses, and evictions. 

It takes in a command line argument as such:
'./csim [-hv] -s [s] -E [E] -b [b] -t [tracefile] '

'-h' and '-v' are optional flags that prints usage help info and trace info, respectively. 

'-s [s]', '-E [E]', '-b [b]' allows the user to set the number of set index bits (S = 2^s is the number of sets),  associativity of simulator (number of lines per set), and number of block bits (B = 2^b is the block size)

'-t [tracefile]' is the name of the valgrind trace to replay

trans.c implements different versions of matrix transposition and seeks to 
minimize the number of misses given a cache (s = 5, E = 1, b = 5) with techniques
such as loop blocking to take advantage of spatial and temporal locality.
