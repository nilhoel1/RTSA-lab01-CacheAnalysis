# RTSA-lab01-CacheAnalysis

In this lab session you will learn how to implement a LRU cache in abstract representation.

## Use the Helper script

Initial Setup:

    $ ./helper.sh all

To get a list of what the helper script can do simply type

    $ ./helper.sh 

Run:

    $ ./helper.sh run crc

## Use the Terminal

Initial Setup:

    $ mkdir build
    $ cd build
    $ cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../CacheAnalysisPass/
    $ make
    $ cd ..

Run:

    $ opt -load-pass-plugin build/libCacheAnalysisPass.so -passes=lru-misses test/crc.ll