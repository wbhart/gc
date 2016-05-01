# Welcome to gc

This is a series of garbage collector experiments in order to learn how they work.

To make the experiments, check out the branch corresponding to the version you are interested in and then do:

   make gc

   make check 

v0.1 - The first garbage collector is a very basic mark and sweep collector.

* non-incremental
* non-generational
* uses pools for different sized allocations up to 64 words
* minimum allocation 1 word
* 4kb pages are returned to the OS when all allocations therein are unreachable
* uses memmap
* uses 2 mark bits in a card table in the header of the 4kb pages
* 00 unallocated, 01 allocated, 11 marked
* non-copying

William Hart 2016

