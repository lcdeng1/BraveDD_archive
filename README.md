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

Right now this is under development ...

## Building the library

[CMake](https://cmake.org/) is required to build.
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


##### Acknowledgments
This project has been supported in part by the [National Science Foundation](http://www.nsf.gov) under grants CCF-2212142.