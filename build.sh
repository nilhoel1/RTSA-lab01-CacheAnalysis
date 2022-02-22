#!/bin/bash

case $1 in
  clean)
    echo "==== Cleaning build folder ===="
    rm -rf build/
    ;;
  config)
    echo "==== Crating build folder ===="
    mkdir build
    cd build
    echo "==== Configuring cmake ===="
    cmake cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../CacheAnalysisPass/
    echo "==== Done! ===="
    ;;
  compile)
    cd build
    echo "==== Compiling Project ===="
    make
    echo "==== Done! ===="
    ;;
  run)
    opt -load-pass-plugin build/libCacheAnalysisPass.so -passes=lru-misses test/$2.ll
    ;;
  all)
    echo "==== Cleaning build folder ===="
    rm -rf build/
    echo "==== Crating build folder ===="
    mkdir build
    cd build
    echo "==== Configuring cmake ===="
    cmake cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../CacheAnalysisPass/
    echo "==== Compiling Project ===="
    make
    echo "==== Done! ===="
    ;;
esac
