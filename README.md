# 👑 BraveDD
***Binary Reduction and Value on Edge Decision Diagram library***

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.17485574.svg)](https://doi.org/10.5281/zenodo.17485574)

🔗 Available in [GitHub](https://github.com/lcdeng1/BraveDD_archive)

## 💡 Overview
This artifact accompanies the paper:
> **"BraveDD: Binary Reduction and Value on Edge Decision Diagram Library"**
> Submitted to: *TACAS 2026* (Tool-Demonstration Track)

BraveDD is a C++ library of various forms of Binary Decision Diagrams to encode functions over binary variables with various ranges,
various (including dynamic) reduction rules, possibly with edge values, swap flags, and complement flags.
This supports the following types of BDDs:
- Bryant's fully-reduced BDDs ([FBDDs](https://doi.org/10.1109/TC.1986.1676819))
- Zero suppressed BDDs ([ZBDDs](https://doi.org/10.1007/s100090100038))
- Multi-terminal BDDs ([MTBDDs](https://link.springer.com/article/10.1023/A:1008699807402))
- Edge-valued BDDs  ([EVBDDs](https://ieeexplore.ieee.org/document/485378))
- BDDs with Edge-Specified Reductions ([ESRBDDs](https://link.springer.com/chapter/10.1007/978-3-030-17465-1_17))
- BDDs with Complemented edges and Edge-Specified Reductions ([CESRBDDs](https://dl.acm.org/doi/10.1007/s10009-021-00640-0))
- Reduction-on-Edge Complement-and-Swap BDDs ([RexBDDs](https://dl.acm.org/doi/10.1145/3649329.3656533))

## 🧩 Folder Structure
- `developers`: Documentation of status progress to BraveDD developers
- `doxygen/`: Generated doxygen docs from source code
- `examples/`: Stand-alone applications that use the library
- `src/`: Library source code
- `tests/`: Source code used for developers unit testing

## 📦 Installation
> **[Required]** [CMake](https://cmake.org/)
- Run
    ```
    build.sh
    ```
    to set up the build files and make target in `build/` by default, or run
    ```
    build.sh /other/install/location
    ```
    to select an alternate location.

The Library will be in `lib/`, include files in `includes/`, sample applications in `examples/`. This may be sufficient for most users.

## 🚀 Features and Usage
- **Predefined BDD**
  | `PredefForest` | Reductions | Comp. | Swap | Range |
  |:----|:-----|:----:|:----:|:---------:|
  |`FBDD`   | $\mathtt{X}$  | ❌ | ❌ | Boolean |
  |`CFBDD`  | $\mathtt{X}$  | ✅ | ❌ | Boolean |
  |`SFBDD`  | $\mathtt{X}$  | ❌ | ✅ | Boolean |
  |`CSFBDD` | $\mathtt{X}$  | ✅ | ✅ | Boolean |
  |`ZBDD`   | $\mathtt{EH}_0$ | ❌ | ❌ | Boolean |
  |`ESRBDD` | $\mathtt{X}, \mathtt{EH}_0, \mathtt{EL}_0$ | ❌ | ❌ | Boolean |
  |`CESRBDD`| $\mathtt{X}, \mathtt{EH}_0, \mathtt{EH}_1, \mathtt{EL}_0, \mathtt{EL}_1$ | ✅ | ❌ | Boolean |
  |`REXBDD` | $\mathtt{X}, \mathtt{EL}_0, \mathtt{EL}_1, \mathtt{EH}_0, \mathtt{EH}_1, \\ \mathtt{AL}_0, \mathtt{AL}_1, \mathtt{AH}_0, \mathtt{AH}_1$  | ✅ | ✅ | Boolean |
  |`MTBDD` | $\mathtt{X} $ | ❌ | ❌ | Non-Boolean |
  |`EVFBDD` | $\mathtt{X} $ | ❌ | ❌ | Non-Boolean |
  |`FBMXD` | $\mathtt{X} $ | ❌ | ❌ | Boolean |
  |`IBMXD` | $\mathtt{I} $ | ❌ | ❌ | Boolean |
  <!-- |`SMTBDD` | $\mathtt{X} $ | ❌ | ✅ | Non-Boolean |
  |`SEVFBDD`| $\mathtt{X} $ | ❌ | ✅ | Non-Boolean | -->

- **Usage**
  ```
  /**
   * Forest Setting Construction
   * (name of predefined BDD, number of variables)
   * Note: this will construct the setting as predefined BDD
   */
  ForestSetting setting(PredefForest::FBDD, 4);
  /* Declare and construct Forest */
  Forest* forest = new Forest(setting);
  /* Declare BDD roots attached to the Forest */
  Func x1(forest), x2(forest), x3(forest), x4(forest);
  /* Initialize BDD roots as variables */
  x1.variable(1);
  x2.variable(2);
  x3.variable(3);
  x4.variable(4);
  /* Build the target logic expression */
  Func target = ((!x4) & (!x1)) | (x3 & x2);
  ```


## 🧪 Example Demonstrations

> In `build/examples`

- #### Example 0: Hello World
  ```
  ./00_hello_world
  ```

  This example will output the basic version information:

  ```
  **************************************[BraveDD]**************************************
  Version: 	0.1.1
  Homepage: 	https://github.com/asminer/brave_dd
  Copyright: 	Copyright (C) 2024, Lichuan Deng, Andrew S. Miner, Gianfranco Ciardo.
  License: 	Apache License, version 2.0
  Last Update: 	Oct 22, 2025
  *************************************************************************************
  ```
- #### Example 1: Logic Expression
  ```
  ./01_logic_expression
  ```

  This example demonstrates how to initialize a BDD `ForestSetting` and `Forest`, construct `FBDD` and `REXBDD` representations of a logic expression: $(\neg x_4 \land \neg x_1)\lor(x_3\land x_2)$, and compare the `REXBDD` obtained by translating from `FBDD` with the directly generated `REXBDD` to demonstrate the correctness and effectiveness of the conversion.
  It will output the following message:
  ```
  BDD name:                           FBDD
  Number of variables:                4
  Number of nodes:                    6
  Cardinality:                        7
  BDD name:                           RexBDD
  Number of variables:                4
  Number of nodes:                    5
  Cardinality:                        7
  ----------------------------------------------------------
  Translated to BDD:                  RexBDD
  Number of variables:                4
  Number of nodes:                    5
  Cardinality:                        7
  ```
  > **[Optional]** Visualize the result BDD in a `.pdf` file
  > - [Graphviz](https://graphviz.org/download/) is **Required**
  ```
  ./01_logic_expression -v
  ```
  This will generate a `.gv` file and compile it to the corresponding `.pdf` file as a result of the target BDD representation.

- #### More Examples
  - N-Queens: `02_queens`
  - N-Queens Cover: `03_queens_cover`
  - Sliding Puzzle: `04_sliding_puzzle`
  - Dining Philosophers Model: `05_dining_philosophers`

  Please run
  ```
  ./<example> -help
  ```
  for more information and usage in details, e.g., `./02_queens -help`

## 📮 Contact
- Maintainer: Lichuan Deng
- Advisors: Dr. Andrew S. Miner, Dr. Gianfranco Ciardo
- Email: lcdeng@iastate.edu
- Affiliation: Department of Computer Science, Iowa State University

##### Acknowledgments
This project has been supported in part by the [National Science Foundation](http://www.nsf.gov) under grants CCF-2212142.