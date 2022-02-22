# RTSA-lab01-CacheAnalysis

In this lab session you will learn how to implement a LRU cache in abstract representation.


Configure:

    $ ./configure.sh

Build:

    $ ./build.sh

Run:

    $ opt -load-pass-plugin build/libCacheAnalysisPass.so -passes=hello-world test/dijkstra.ll
    $ opt -load-pass-plugin build/libCacheAnalysisPass.so -passes=hello-world test/hello.ll