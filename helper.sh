#!/bin/bash

clean () {
  echo "==== Cleaning build folder ===="
  rm -rf build/
}

config () {
  echo "==== Crating build folder ===="
  mkdir build
  cd build
  echo "==== Configuring cmake ===="
  cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../CacheAnalysisPass/
  echo "==== Done! ===="
}

compile () {
  cd build
  echo "==== Compiling Project ===="
  make
  echo "==== Done! ===="
}

run () {
    opt -load-pass-plugin build/libCacheAnalysisPass.so -passes=lru-misses test/$1.ll
}

case $1 in
  clean)
    clean
    ;;
  config)
    config
    ;;
  compile)
    compile
    ;;
  run)
    if [ $2 ]; then
      run $2
    else
      echo "==== Please provide name of the test as second argument! ===="
    fi
    ;;
  all)
    clean
    config
    make
    echo "==== Done! ===="
    ;;
  *)
    echo "Unknown argument: $1"
    echo "Please provide one of the following arguments:"
    echo ""
    echo "  clean               Deletes the build folder"
    echo "  config              Creates build folder and configures build System"
    echo "  compile             Compiles the Project"
    echo "  all                 Cleans, configures and compiles the project."
    echo "  run [name]          Run pass on test/[name].ll from the test folder"
  ;;
esac
