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
  cd ..
  echo "==== Done! ===="
}

run () {
  echo "==== Running $1 ===="
  opt -load-pass-plugin build/libCacheAnalysisPass.so \
    -passes=lru-misses,mem2reg \
    test/$1.ll -o /dev/null
}

allBenchs=( "adpcm"
  "bs"
  "bsort100"
  "cnt"
  "compress"
  "cover"
  "crc"
  "dijkstra"
  "duff"
  "edn"
  "expint"
  "fdct"
  "fft1"
  "fibcall"
  "fir"
  "hello"
  "insertsort"
  "janne_complex"
  "jfdctint"
  "lcdnum"
  "lms"
  "ludcmp"
  "matmult"
  "minver"
  "ndes"
  "nsichneu"
  "ns"
  "prime"
  "qsort-exam"
  "qurt"
  "recursion"
  "select"
  "sqrt"
  "statemate"
  "ud"
  "whet"
)

runall () {
  for str in ${allBenchs[@]}; do
    echo
    run $str
  done
}

case $1 in
  clean)
    clean
    ;;
  config)
    config
    ;;
  c | compile)
    compile
    ;;
  cr)
    compile
    if [ $2 ]; then
      run $2
    else
      echo "==== Please provide name of the test as second argument! ===="
    fi
    ;;
  r | run)
    if [ $2 ]; then
      run $2
    else
      echo "==== Please provide name of the test as second argument! ===="
    fi
    ;;
  ra | runall)
    runall
    ;;
  a | all)
    clean
    config
    make
    echo "==== Done! ===="
    ;;
  *)
    if [ $1 ]; then
      echo "Unknown argument: $1"
    fi
    echo "Please provide one of the following arguments:"
    echo "  clean               Deletes the build folder"
    echo "  config              Creates build folder and configures build System"
    echo "  c | compile             Compiles the Project"
    echo "  a | all                 Cleans, configures and compiles the project"
    echo "  r | run [name]          Run pass on test/[name] from the test folder"
    echo "  cr [name]               Compile and run pass on test/[name] from the test folder"
    echo "  ra | runall              Run pass on all tests from the test folder"
    exit
  ;;
esac
