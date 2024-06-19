# brave_dd
Binary, reduction and value on edge, Decision Diagram library.

This is a C++ implementation of various forms of Binary Decision Diagrams
to encode functions over binary variables with various ranges,
various (including dynamic) reduction rules, possibly with edge values,
swap flags, and complement flags.
This supports the following types of BDDs:
    - Bryant's fully-reduced BDDs (standard ROBDDs)
    - Zero suppressed BDDs
    - Multi-terminal BDDs
    - Edge-valued BDDs
    - ESR, CESR, and RexBDDs


## Current status

Under development ... :-)

## Building the library

[CMake](https://cmake.org/) is required to build.

0. Building as preset:
    ```
    cmake . --preset==release
    ```
    or
    ```
    cmake . --preset==debug
    cd build-debug
    make
    ```
OR:
1. Set up the building location:
    ```
    $ mkdir build
    $ cd build
    $ cmake ..
    ```
2. Now start to build:
   ```
   $ make
   ```
   Library will be in ```build/src/```. Test and example applications in ```build/tests/``` and ```build/examples/```. This may be sufficient for most users.
3. Remove the results of compilation:
   ```
   $ make clean
   ```
## Running examples
The ```examples/example_0.cc``` is a simple example to use this library. You can run it as follows:

```
cd build/examples
./example_0
```
For now, it will output the basic information:
```
****************************[BraveDD]****************************
Version: 	0.0.1
Homepage: 	https://github.com/asminer/brave_dd
Copyright: 	Copyright (C) 2024, .
License: 	Apache License, version 2.0
Last Update: 	Jun 14, 2024
*****************************************************************
```


##### Acknowledgments
This project has been supported in part by the [National Science Foundation](http://www.nsf.gov) under grants CCF-2212142.