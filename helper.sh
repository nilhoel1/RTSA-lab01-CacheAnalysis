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
  cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../CacheAnalysisPass/
  echo "==== Done! ===="
}

compile () {
  cd build
  echo "==== Compiling Project ===="
  ninja
  cd ..
  echo "==== Done! ===="
}

run () {
  echo "==== Running $1 ===="
  opt -load-pass-plugin build/libCacheAnalysisPass.so \
    -passes='lru-misses(function(loop-unroll-and-jam))' \
    test/$1.ll -o /dev/null
  #llvm-dis < out.bc > out.ll
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
  docker)
    docker build -t rtsalab01cacheanalysis:latest .
    docker run -i -d -v "$(pwd)"/.:/root:rw --name RTSAlab01 rtsalab01cacheanalysis
    ;;
  evaluation | eval)
    run "fft1"
    echo "==== Correct fft1 ===="
    echo "MustHits: 16"
    echo "MayMisses: 280"
    echo
    run "bsort100"
    echo "==== Correct bsort100 ===="
    echo "MustHits: 1"
    echo "MayMisses: 41"
    echo
    run "lms"
    echo "==== Correct lms ===="
    echo "MustHits: 5"
    echo "MayMisses: 288"
    echo
    run "minver"
    echo "==== Correct minver ===="
    echo "MustHits: 6"
    echo "MayMisses: 224"
    echo
    run "qsort-exam"
    echo "==== Correct qsort-exam ===="
    echo "MustHits: 2"
    echo "MayMisses: 152"
    echo
    run "recursion"
    echo "==== Correct recursion ===="
    echo "MustHits: 8"
    echo "MayMisses: 8"
    echo
    run "select"
    echo "==== Correct select ===="
    echo "MustHits: 4"
    echo "MayMisses: 108"
    echo
    run "whet"
    echo "==== Correct whet ===="
    echo "MustHits: 5"
    echo "MayMisses: 265"
    echo
    ;;
  a | all)
    clean
    config
    ninja
    echo "==== Done! ===="
    ;;
  *)
    if [ $1 ]; then
      echo "Unknown argument: $1"
    fi
    echo "Please provide one of the following arguments:"
    echo "  clean               Deletes the build folder"
    echo "  config              Creates build folder and configures build System"
    echo "  docker              Build and Run Docker container for development"
    echo "  eval                Run a subset of tests for evaluation of your implementation"
    echo "  c | compile             Compiles the Project"
    echo "  a | all                 Cleans, configures and compiles the project"
    echo "  r | run [name]          Run pass on test/[name] from the test folder"
    echo "  cr [name]               Compile and run pass on test/[name] from the test folder"
    echo "  ra | runall              Run pass on all tests from the test folder"
    exit
  ;;
esac
