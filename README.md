# RTSA-lab01-CacheAnalysis

In this lab session you will learn how to implement a LRU cache in abstract representation.
The Goal is to implement an LRU must Join in include/AbstractState.h
The Project can build, tested and Evaluated with the Helper Script.

## Disclaimer

This is the first time we provide this exercise. 
Should you encounter something you think is a Bug, please let me know, during lab sessions.

Also keep track of the Repository as I may add more features to the script.

## Setup

We recommend using docker and VS Code for setup.
If this is not your preferred Setup, take a look in the Docker file for the dependencies.

Also we do not support the usage of Windows, Linux is free of charge so get a copy.

### Setting Docker up:

1.) install docker and VS Code on your Distribution.

https://docs.docker.com/get-docker/

https://code.visualstudio.com/

For this setup you cannot use the OSS version of VS code or the version from Snap, as the remote development extensions will not work.

2.) We recommend you install the following extensions in vs code

C/C++, 
clangd, 
Docker and 
Remote Development

3.) Use the helper script to build and run a Container

    $ ./helper.sh docker

This will build a docker image and run a Docker container with the current directory mounted.

4.) Attach VS Code to the container, in the Docker Tab, and start developing

## Debugging

When you are using VS Code you can simply use the Debugging Tab, we prepared a debug script for you.
You can also set the following variables in the CacheAnalysisPass/CacheAnalysisPass.cpp file, for more output:

```cpp
  // Development Options
  bool PrintAddresses = false;
  bool PrintEdges = false;
  bool PrintEdgesPost = false;
  bool DumpToDot = false;
  bool DumpNodes = false;
```

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

This section is not needed, fi you are using the script but for the sake of completeness it is provided anyways.

Initial Setup:

    $ mkdir build
    $ cd build
    $ cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../CacheAnalysisPass/
    $ make
    $ cd ..

Run:

    $ opt -load-pass-plugin build/libCacheAnalysisPass.so -passes=lru-misses test/crc.ll