# RTSA-lab01-CacheAnalysis

In this lab session you will learn how to implement a LRU cache in abstract representation.
The Goal is to implement an LRU must Join in include/AbstractState.h.
The Project can be build, tested and Evaluated with the "helper" script.

## Disclaimer

This is the first time we offer this exercise.
Should you encounter something you think is a Bug, please let me (Nils) know, during lab sessions.

Keep track of the Repository as I may add more features to the script.

Also I do not support the usage of Windows, Linux is free of charge so get a copy.
I am more than happy helping you install Linux on your machine.

## Setup

I recommend using docker and VS Code for setup.
Also check out the recommended extensions in the Docker section.
If you prefer to work from Linux or Mac OS X, check the Dockerfile for dependencies.

Also make sure to have a LLVM13 install.
And export your LLVM13 install to the `$LLVM_DIR` variable, with:

```bash
export LLVM_DIR=path/to/LLVM13
```

### VS Code Setup

1.) Install VS Code on your Distribution or get it from Microsoft.

<https://code.visualstudio.com/>

2.) I recommend you install the following extensions in vs code

clangd,
Clang-Format,
CodeLLDB,
Docker and
Remote Development

For a general C/C++ setup of VS Code (I consider good) see:
<https://ahemery.dev/2020/08/24/c-cpp-vscode/>

Most parts can be skipped, as they are already integrated in this Repo.

### Installing LLVM 13 on Mac OS X

On Darwin you can install LLVM 13 with [Homebrew](https://brew.sh/):

```bash
brew install llvm@13
```

If you already have an older version of LLVM installed, you can upgrade it to
LLVM 13 like this:

```bash
brew upgrade llvm
```

Once the installation (or upgrade) is complete, all the required header files,
libraries and tools will be located in `/usr/local/opt/llvm/`.

### Installing LLVM 13 on Ubuntu

On Ubuntu Bionic, you can [install modern
LLVM](https://blog.kowalczyk.info/article/k/how-to-install-latest-clang-6.0-on-ubuntu-16.04-xenial-wsl.html)
from the official [repository](http://apt.llvm.org/):

```bash
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-13 main"
sudo apt-get update
sudo apt-get install -y llvm-13 llvm-13-dev llvm-13-tools clang-13
```

This will install all the required header files, libraries and tools in
`/usr/lib/llvm-13/`.

### Building LLVM 13 From Sources

Building from sources can be slow and tricky to debug. It is not necessary, but
might be your preferred way of obtaining LLVM 13. The following steps will work
on Linux and Mac OS X:

```bash
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout release/13.x
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host -DLLVM_ENABLE_PROJECTS=clang <llvm-project/root/dir>/llvm/
cmake --build .
```

For more details read the [official
documentation](https://llvm.org/docs/CMake.html).

This will install all the required header files, libraries and tools in `your/llvm/build/path`.

### Setting up Docker

1.) install docker and VS Code on your Distribution.

<https://docs.docker.com/get-docker/>

<https://code.visualstudio.com/>

For this setup you cannot use the OSS version of VS code or the version from Snap, as the remote development extensions will not work.

2.) I recommend you install the following extensions in vs code

clangd,
Clang-Format,
CodeLLDB,
Docker and
Remote Development

For a general C/C++ setup of VS Code (I consider good) see:
<https://ahemery.dev/2020/08/24/c-cpp-vscode/>

Most of the setup can be skipped

3.) Use the helper script to build and run a Container

```bash
./helper.sh docker
```

This will build a docker image and run a Docker container with the current directory mounted.

The Docker container can later be started from the Docker VS Code extension.

4.) Attach VS Code to the container, in the Docker Tab, and start developing

## Debugging

When you are using VS Code you can simply use the Debugging Tab, I prepared a debug script for you.
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

### Initial Setup

```bash
./helper.sh all
```

To get a list of what the helper script can do simply type

```bash
./helper.sh 
```

### Run

Run the pass on a single test.
fft1 is recommended during development.

```bash
./helper.sh run fft1
```

### Eval

Runs the Pass on a set of tests and also prints the expected results.
This will be used to measure correctness of you implementation.

```bash
./helper.sh eval
```

## Use the Terminal (Obsolete if script is used)

This section is not needed, fi you are using the script but for the sake of completeness it is provided anyways.

Initial Setup:

```bash
mkdir build
cd build
cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../CacheAnalysisPass/
make
cd ..
```

Run:

```bash
opt -load-pass-plugin build/libCacheAnalysisPass.so \
-passes=lru-misses test/crc.ll
```
