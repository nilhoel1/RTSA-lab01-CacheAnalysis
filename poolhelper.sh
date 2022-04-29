#!/bin/bash

clean () {
  echo "==== Cleaning build folder ===="
  rm -rf build/
  rm compile_commands.json
}

config () {
  echo "==== Crating build folder ===="
  cd llvm
  export LLVM_DIR=$(pwd)
  cd ..
  mkdir build
  cd build
  echo "==== Configuring cmake ===="
  cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ..
  cd ..
  cp build/compile_commands.json compile_commands.json
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
  ./llvm/bin/opt -load-pass-plugin build/lib/libCacheAnalysisPass.so \
    -passes='lru-misses(function(loop-unroll-and-jam))' \
    test/$1.ll -o /dev/null
  #llvm-dis < out.bc > out.ll
}

buildllvm() {
  mkdir llvm
  cd llvm
  wget https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.1/llvm-13.0.1.src.tar.xz
  tar -xf llvm-13.0.1.src.tar.xz
  rm llvm-13.0.1.src.tar.xz
  cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DLLVM_TARGETS_TO_BUILD=host llvm-13.0.1.src/
  make -j 4
  export LLVM_DIR=$(pwd) >> ~/.zshrc
  export LLVM_DIR=$(pwd) >> ~/.bashrc
  cd ..
}


mac() {
  brew install cmake make wget ninja
  buildllvm
  cd llvm
  export LLVM_DIR=$(pwd) >> ~/.zshrc
  cd ..
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
  llvm)
    buildllvm
    ;;
  continuellvm)
    cd llvm
    make -j 4
    cd ..
    ;;
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
    echo "Currently not available!"
    echo "But please continue to implement the must join,"
    echo "to the best of your abilities and check for updates!"
    # run "fft1"
    # echo "==== Correct fft1 ===="
    # echo "MustHits: 16"
    # echo "MayMisses: 280"
    # echo
    # run "bsort100"
    # echo "==== Correct bsort100 ===="
    # echo "MustHits: 1"
    # echo "MayMisses: 41"
    # echo
    # run "lms"
    # echo "==== Correct lms ===="
    # echo "MustHits: 5"
    # echo "MayMisses: 288"
    # echo
    # run "minver"
    # echo "==== Correct minver ===="
    # echo "MustHits: 6"
    # echo "MayMisses: 224"
    # echo
    # run "qsort-exam"
    # echo "==== Correct qsort-exam ===="
    # echo "MustHits: 2"
    # echo "MayMisses: 152"
    # echo
    # run "recursion"
    # echo "==== Correct recursion ===="
    # echo "MustHits: 8"
    # echo "MayMisses: 8"
    # echo
    # run "select"
    # echo "==== Correct select ===="
    # echo "MustHits: 4"
    # echo "MayMisses: 108"
    # echo
    # run "whet"
    # echo "==== Correct whet ===="
    # echo "MustHits: 5"
    # echo "MayMisses: 265"
    # echo
    ;;
  a | all)
    clean
    config
    cd build
    make
    echo "==== Done! ===="
    ;;
  *)
    if [ $1 ]; then
      echo "Unknown argument: $1"
    fi
    echo "Please provide one of the following arguments:"
    echo "  llvm                Build LLVM from source"
    echo "  llvmcontinue        Contionue building llvm where you left off last time"
    echo "  mac                 Setup LLVM for mac (needs brew)"
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
