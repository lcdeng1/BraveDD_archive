***OLD CODE FROM GANDALF 2023 PAPER***

**BDD SRCs**

*Repo basics*

In this directory is all the code and results used for the Gandalf2023 paper. 
The three "src" repositories are nearly identical (and should eventually be merged), and 
each contains a seperate BDD-library for MTBDDs, EV+BDDs, and EV%BDDs, each with the ablility 
to utilize swap flags (found in the def #WITH_SWAPS in bdd.h). Additionally, the mtbdd can use
level shifters, which were not included in the Gandalf2023 paper. 

*src files*

bdd.cc/bdd.h - these are the main BDD files which handle BDD behavior  
build_bdd.cc - this file is effectively the 'main' file for BDD creation  
alg_osm/alg_tsm/alg_restr.cc - these files handle the three concretization techniques.   
switches.h/switches.cc - file which reads in the desired BDD properties from user.   
  
ct.h/ct.cc - the compute table handler. Mostly deals with hashing on operations.  
operation.h/operation.cc - object which defines operations on BDDs   
  
parser.h/parser.cc - reads the input data files and creates minterms for the buffer to read  
buffer.h/buffer.cc - reads minterms and turns them into BDDs.   
  
read_bdd/read_bdds.cc - reads the files created by build_bdd and creates unmutable fixed BDD. read_bdd.cc is especially useful for quick manual bug testing!  
fixed.h/fixed.cc - simplified version of bdd.h/bdd.cc which is unmutable but fast to build/query  

**BDD BUILDING**

To build a BDD, the code currently reads several lines from the data-file (determined by the buffer-size). 
A minterm is them generated from these lines, and read in as an unreduced MTBDD (For EV+ and EV%, this is 
a non-canonical BDD where all edge values are on the bottom level). A recursive algorithm (for each reduction) 
is then run on these MTBDDs, turning them into a reduced and canonical structure. 

Finally, this new canonical BDD structure is stored using bdd.h/bdd.cc, and unioned with all previously read minterms. 
This process is repeated until the full file is read.
