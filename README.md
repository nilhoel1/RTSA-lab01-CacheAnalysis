# RTSA-lab01-CacheAnalysis

In this lab session you will learn how to implement a LRU cache in abstract representation.
The Goal is to implement an LRU must Join in include/AbstractState.h
The Project can build, tested and Evaluated with the Helper Script.

## Use the Helper script

### Initial Setup:

    $ ./helper.sh all

To get a list of what the helper script can do simply type

    $ ./helper.sh 

### Run:
Run the pass on a single test.
fft1 is recommended during development.

    $ ./helper.sh run fft1

### Eval:
Runs the Pass on a set of tests and also prints the expected results. 
This will be used to measure correctness of you implementation.

    $ ./helper.sh eval

## Use the Terminal (Obsolete if script is used)

Initial Setup:

    $ mkdir build
    $ cd build
    $ cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../CacheAnalysisPass/
    $ make
    $ cd ..

Run:

    $ opt -load-pass-plugin build/libCacheAnalysisPass.so -passes=lru-misses test/crc.ll