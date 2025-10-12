/* This is an example of building the set of configurations (states) reachable to target configure
 * up to k steps for user-specified sliding puzzle.
 * 
 */

/* The vector encoding a tiles configure: 
* conf[i][j] = k means that tile k on the position of row i and column j, 
* while k=0 for the empty.
* E.g., {
*          {1, 2, 3, 4},
*          {5, 6, 7, 8},
*          {9, 10, 11, 12},
*          {13, 14, 15, 0}
*       };
* position in [1, N*M]
* tile index in [1, N*M-1]
* empty position in [0, N*M-1]
*
* Its Bits representation is:
*  msb
*  |
* [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 | 15]
*  ^                                              ^    ^
*  |______________________________________________|    |
*                           |                          |
*           Expand 2-D vector into a 1-D vector        +-- last elmt: the postion of empty
*
* The max supported scale is 4 X 4
*/

#include <queue>
#include <cmath>

#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <future>
#include <atomic>
#include <cstdint>
#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;


using PuzzleState = std::vector<std::vector<uint16_t> >; // 2D representation of the puzzles
using State = uint64_t;
using Bound = uint8_t;

// The size of the puzzle
uint16_t N;         // row
uint16_t M;         // column
uint16_t BITS;      // bits used per position
uint16_t MSB;       // position of MSB
uint64_t SIZE;      // target size of state space
uint64_t MASK;      // mask for bits
uint64_t ROW_MASK;  // mask for M adjacent elements
uint64_t UC_MASK;   // mask for M-1 adjacent elements

// Mask for bounds
uint8_t LEFT_EXP = 0x01 << 3;
uint8_t RIGHT_EXP = 0x01 << 2;
uint8_t DOWN_EXP = 0x01 << 1;
uint8_t UP_EXP = 0x01;

// counter for total explored
size_t total = 0;

// number of threads
int numThreads = 1;

// for file output
std::string fileName;
std::ofstream outFile;

// flags
bool isSearch = 0;
bool isOutputTable = 0;
bool isPipe = 0;
bool isBuildBDD = 0;
bool isMultiRoot = 0;

std::string bdd = "fbdd";


/* 
Helper function to print the puzzle state (configure)
*/
void printPuzzleState(const std::vector<std::vector<uint16_t> >& puzzle)
{
    std::cerr << "\t";
    for (int i=0; i<M-1; i++) {
        std::cerr << "--------";
    }
    std::cerr << "-" << std::endl;
    for (const auto& row : puzzle) {
        for (uint16_t val : row) {
            if (val == 0) std::cerr << "\t  "; // empty space
            else std::cerr << "\t" << val;
        }
        std::cerr << std::endl;
    }
    std::cerr << "\t";
    for (int i=0; i<M-1; i++) {
        std::cerr << "--------";
    }
    std::cerr << "-" << std::endl;
    std::cerr << std::endl;
}
/*
Print a puzzle state in bits representation, for debug
*/
void printBits(const State puzzle)
{
    uint16_t count = 0;
    for (int i=63; i>=0; i--) {
        std::cerr << (bool)(puzzle & (0x01ULL<<i));
        count++;
        if (count == BITS) {
            std::cerr << "  ";
            count = 0;
        }
    }
    std::cerr << std::endl;
}
/*
Encode 64 bits as puzzle state
*/
std::vector<std::vector<uint16_t> > encodeBits2Puzzle(State puzzle)
{
    //
    std::vector<std::vector<uint16_t> > conf(N, std::vector<uint16_t>(M, 0));
    uint16_t eptPos = puzzle & MASK;
    std::cerr << "empty postion: " << eptPos << std::endl;
    uint16_t shift = MSB - BITS + 1;
    uint16_t row, col;
    for (uint16_t i=1; i<=N*M; i++) {
        row = (i%M==0) ? i/M-1 : i/M;
        col = (i%M==0) ? M-1 : i%M-1;
        if ((i - 1) == eptPos) {
            conf[row][col] = 0;
        } else {
            conf[row][col] = (puzzle & (MASK << shift)) >> shift;
            shift -= BITS;
        }
    }
    return conf;
}
/*
Encode the puzzle state as a 64 bits
*/
uint64_t encodePuzzle2Bits(const PuzzleState& puzzle)
{
    uint64_t ans = 0;
    uint16_t row, col;
    uint16_t shift = MSB-BITS+1;
    for (uint16_t i=1; i<=N*M; i++) {
        row = (i%M==0) ? i/M-1 : i/M;
        col = (i%M==0) ? M-1 : i%M-1;
        if (puzzle[row][col] == 0) {
            ans &= ~MASK;
            ans |= (i-1);
        } else {
            ans &= ~(MASK<<shift);
            ans |= ((uint64_t)puzzle[row][col]<<shift);
            shift -= BITS;
        }
    }
    return ans;
}
/*
Transition to get one neighbor by given a direction
    0: left;
    1: right;
    2: down;
    3: up
return 0: invalid direction
*/
State getNeighbor(const State initial, const int direction)
{
    State start = initial;
    State neighbor = 0;
    uint16_t eptPos = initial & MASK;
    // left
    if ((direction == 0) && (eptPos%M != 0) && (M != 1)) {
        return (initial-1);
    }
    // right
    if ((direction == 1) && (eptPos%M != (M-1)) && (M != 1)) {
        return (initial+1);
    }
    // down
    if ((direction == 2) && (eptPos < (N-1)*M)) {
        int shift = MSB - (eptPos+M-1)*BITS + 1;
        uint64_t unchange = initial & (UC_MASK << shift);
        uint64_t change = initial & (MASK << (shift-BITS));
        start &= ~(ROW_MASK << (shift-BITS));   // clear position
        start |= (unchange >> BITS);
        start |= (change << ((M-1)*BITS));
        start += M;
        return start;
    }
    // up
    if ((direction == 3) && (eptPos >= M)) {
        int shift = MSB - eptPos*BITS + 1;
        uint64_t unchange = initial & (UC_MASK << shift);
        uint64_t change = initial & (MASK << (shift+(M-1)*BITS));
        start &= ~(ROW_MASK << shift);   // clear position
        start |= (unchange << BITS);
        start |= (change >> ((M-1)*BITS));
        start -= M;
        return start;
    }
    return neighbor;
}
/*
transitions to get neighbors
*/
std::vector<State> getNeighbors(State initial)
{
    //
    State start = initial;
    std::vector<State> neighbors;
    uint16_t eptPos = initial & MASK;
    // left
    if ((eptPos%M != 0) && (M != 1)) {
        neighbors.push_back(initial-1);
    }
    // right
    if ((eptPos%M != (M-1)) && (M != 1)) {
        neighbors.push_back(initial+1);
    }
    // down
    if (eptPos < (N-1)*M) {
        int shift = MSB - (eptPos+M-1)*BITS + 1;
        uint64_t unchange = initial & (UC_MASK << shift);
        uint64_t change = initial & (MASK << (shift-BITS));
        start &= ~(ROW_MASK << (shift-BITS));   // clear position
        start |= (unchange >> BITS);
        start |= (change << ((M-1)*BITS));
        start += M;
        neighbors.push_back(start);
    }
    // up
    if (eptPos >= M) {
        int shift = MSB - eptPos*BITS + 1;
        uint64_t unchange = initial & (UC_MASK << shift);
        uint64_t change = initial & (MASK << (shift+(M-1)*BITS));
        start &= ~(ROW_MASK << shift);   // clear position
        start |= (unchange << BITS);
        start |= (change >> ((M-1)*BITS));
        start -= M;
        neighbors.push_back(start);
    }
    return neighbors;
}
/*
Copmute the expected size
*/
uint64_t expectedSize(unsigned n) {
    uint64_t result = 1;
    bool divided = false;
    for (unsigned i = 2; i <= n; ++i) {
        if (!divided && (i % 2 == 0)) {
            result *= (i / 2);  // divide one even number by 2
            divided = true;
        } else {
            result *= i;
        }
    }
    return result;
}
/*
Output distance table as pla format and compress as xz
*/
void outputPlaHeader(std::ostream& out)
{
    out << ".type f\n";
    out << ".i " << N*M*BITS << "\n";
    out << ".o " << 8 << "\n";     // 8 bits for distance is enough
    out << ".p " << SIZE << "\n";
}
/*
Output one state as pla format
*/
void outputPlaState(std::ostream& out, State& s, uint8_t& d)
{
    for (int i=MSB; i>=0; i--) {
        out << (bool)(s & (0x01ULL<<i));
    }
    out << " ";
    // distance in bits
    for (int i=7; i>=0; i--) {
        if (d & (0x01<<i)) {
            out << "1";
        } else {
            out << "~";
        }
    }
    out << "\n";
}

/*
Thread-safe parallel frontier search
*/
void parallelFrontier(State initial, size_t num_threads) {
    std::vector<std::pair<State, Bound>> frontier = {{initial, 0}};
    uint8_t depth = 0;

    while (!frontier.empty()) {
        std::cerr << "Depth " << (int)depth << ", frontier = " << frontier.size() << "\n";
        total += frontier.size();
        // output the frontier if pipeline allowed
        if (isOutputTable) {
            for (auto& d : frontier) {
                outputPlaState((isPipe) ? std::cout : outFile, d.first, depth);
            }
        }
        std::vector<std::thread> threads;

        std::vector<std::vector<std::pair<State, Bound>>> local_frontiers(num_threads);
        size_t chunk = (frontier.size() + num_threads - 1) / num_threads;
        // divide frontier into chunks;
        std::vector<std::vector<std::pair<State, Bound>>> chunks(num_threads);
        for (size_t t = 0; t < num_threads; ++t) {
            size_t start = t * chunk;
            if (start >= frontier.size()) break;
            size_t end = std::min(start + chunk, frontier.size());
            for (size_t i=start; i<end; i++) {
                chunks[t].push_back(frontier[i]);
            }
        }
        // release the memory of frontier
        frontier = std::vector<std::pair<State, Bound>>();

        for (size_t t = 0; t < num_threads; ++t) {
            threads.emplace_back([&, t]() {
                // std::vector<std::pair<State, Bound>> local_new;
                std::unordered_map<State, Bound> local_new;
                for (auto& s : chunks[t]) {
                    State ns;
                    Bound nb;
                    if (!(s.second & LEFT_EXP)) {   // Left direction not explored
                        ns = getNeighbor(s.first, 0);
                        nb = 0;
                        if (ns != 0) {
                            nb |= RIGHT_EXP;
                            // local_new.emplace_back(ns, nb);
                            local_new[ns] |= nb;
                        }
                    }
                    if (!(s.second & RIGHT_EXP)) {  // Right direction not explored
                        ns = getNeighbor(s.first, 1);
                        nb = 0;
                        if (ns != 0) {
                            nb |= LEFT_EXP;
                            // local_new.emplace_back(ns, nb);
                            local_new[ns] |= nb;
                        }
                    }
                    if (!(s.second & DOWN_EXP)) {   // Down direction not explored
                        ns = getNeighbor(s.first, 2);
                        nb = 0;
                        if (ns != 0) {
                            nb |= UP_EXP;
                            // local_new.emplace_back(ns, nb);
                            local_new[ns] |= nb;
                        }
                    }
                    if (!(s.second & UP_EXP)) {     // Up direction not explored
                        ns = getNeighbor(s.first, 3);
                        nb = 0;
                        if (ns != 0) {
                            nb |= DOWN_EXP;
                            // local_new.emplace_back(ns, nb);
                            local_new[ns] |= nb;
                        }
                    }
                }
                // release this chunk
                chunks[t] = std::vector<std::pair<State, Bound>>();
                // move local new explored state to global
                for (auto& s : local_new) {
                    local_frontiers[t].emplace_back(s.first, s.second);
                }
            });
        }
        // launch threads
        for (auto& th : threads) th.join();

        // Merge local frontiers and de-duplicate
        std::unordered_map<State, Bound> collect;
        for (auto& lf : local_frontiers) {
            for (auto& s : lf) {
                collect[s.first] |= s.second;
            }
            lf = std::vector<std::pair<State, Bound>>();
        }
        // update frontier
        for (auto& s : collect) {
            frontier.emplace_back(s.first , s.second);
        }
        ++depth;
    } // end while

    /* wrap up the output */
    if (isOutputTable) {
        if (isPipe) {
            std::cout << ".end";
        } else {
            outFile << ".end";
            outFile.close();
            std::string cmd = "xz ";
            cmd += fileName;
#ifdef _WIN32
            int result = system(("cmd /C " + cmd).c_str());
#else
            int result = system(("sh -c \"" + cmd + "\"").c_str());
#endif
            if (result) {
                std::cerr << "[BRAVE_DD] Error!\t Failed to run xz and build file: "<< fileName<< ".xz" << std::endl;
                return;
            }
        }
        std::cerr << "Done!" << std::endl;
    }
    std::cerr << "Total states discovered: " << total << "\n";
}

bool processArgs(int argc, const char** argv)
{
    bool setN = 0, setM = 0;
    for (int i=1; i<argc; i++) {
        if ('-' == argv[i][0]) {
            // options other than size
            if (strcmp("-nt", argv[i])==0) {
                numThreads = atoi(argv[i+1]);
                i++;
                continue;
            }
            if (strcmp("-s", argv[i])==0) {
                isSearch = 1;
                continue;
            }
            if (strcmp("-ot", argv[i])==0) {
                isOutputTable = 1;
                continue;
            }
            if (strcmp("-p", argv[i])==0) {
                isPipe = 1;
                continue;
            }
            if (strcmp("-bb", argv[i])==0) {
                isBuildBDD = 1;
                continue;
            }
            if (strcmp("-mr", argv[i])==0) {
                isMultiRoot = 1;
                continue;
            }
            if (strcmp("-bdd", argv[i])==0) {
                bdd = argv[i+1];
                i++;
                continue;
            }
        }
        if (!setN) {
            N = atoi(argv[i]);
            setN = 1;
            continue;
        }
        if (setN && !setM) {
            M = atoi(argv[i]);
            setM = 1;
        }
        if (setN && setM) continue;
    }

    if (!setN || N<1 || !setM || M<1) return 0;
    // default target configure

    return 1;
}

int usage(const char* who)
{
    /* Strip leading directory, if any: */
    const char* name = who;
    for (const char* ptr=who; *ptr; ptr++) {
        if ('/' == *ptr) name = ptr+1;
    }
    std::cerr << "Usage: " << name << "[options] N M\n" << std::endl;
    std::cerr << "\t            N: the number of board rows" << std::endl;
    std::cerr << "\t            M: the number of board columns" << std::endl;
    std::cerr << "\t[option]  -nt: followed with a number of threads to BFS distance (default: 1)" << std::endl;
    std::cerr << "\t[option]  -s: switch to turn ON parallel Frontier search" << std::endl;
    std::cerr << "\t[option]  -ot: switch to turn ON the output of result distance table" << std::endl;
    std::cerr << "\t[option]  -p: switch to turn ON pipeline to compress" << std::endl;
    std::cerr << "\t[option]  -bb: switch to turn ON the BDD building" << std::endl;
    return 1;
}

int main(int argc, const char** argv)
{
    
    /* Process arguments and initialize BDD forests*/
    if (!processArgs(argc, argv)) return usage(argv[0]);
    std::cerr << "Solving " << N << " x " << M << "-sliding puzzle." << std::endl;
    std::cerr << "Using " << getLibInfo(0) << std::endl;
    std::cerr << "Target configure: " << std::endl;
    /* The vector encoding a tiles configure: 
    * conf[i][j] = k means that tile k on the position of row i and column j, 
    * k=0 represents the empty position at row i and column j.
    * E.g., {
    *          {1, 2, 3, 4},
    *          {5, 6, 7, 8},
    *          {9, 10, 11, 12},
    *          {13, 14, 15, 0}
    *       };
    */
    std::vector<std::vector<uint16_t> > conf(N, std::vector<uint16_t>(M, 0));
    // default configure
    for (uint16_t i=1; i<N*M; i++) {
        conf[(i%M==0)?i/M-1:i/M][(i%M==0)?M-1:i%M-1] = i;
    }
    printPuzzleState(conf);
    // initialize header info
    BITS = static_cast<uint16_t>(std::ceil(log2(N*M)));
    std::cerr << "bits: " << BITS << std::endl;
    MSB = N*M*BITS-1;
    MASK = ((0x01ULL<<BITS) - 1);
    ROW_MASK = ((0x01ULL<<(M*BITS)) - 1);
    UC_MASK = (0x01ULL<<((M-1)*BITS)) - 1;
    SIZE = expectedSize(N*M);
    // output headers
    if (isOutputTable) {
        if (isPipe) {
            outputPlaHeader(std::cout);
        } else {
            // set the output file
            fileName = "puzzle";
            fileName += "_";
            fileName += std::to_string(N);
            fileName += "_";
            fileName += std::to_string(M);
            fileName += ".pla";
            outFile = std::ofstream(fileName, std::ios::app);
            if (!outFile) {
                std::cerr << "Failed to open file " << fileName << std::endl;
                return 1;
            }
            outputPlaHeader(outFile);
        }
    }
    // encode the initial state in bits
    State initial = encodePuzzle2Bits(conf);
    // search and output as a pla file
    // if (isSearch) parallelBFS_Rex(initial, numThreads);
    if (isSearch) parallelFrontier(initial, numThreads);
    // BFS(initial);

    if (isBuildBDD) {
        /* Parser to read */
        std::string tableName = "puzzle";
        tableName += "_";
        tableName += std::to_string(N);
        tableName += "_";
        tableName += std::to_string(M);
        tableName += ".pla.xz";
        FileReader FR(tableName.c_str());
        ParserPla parser(&FR);
        parser.readHeader();
        long numFun = parser.getNum();
        /* Initial forest */
        ForestSetting setting(bdd, parser.getInBits());
        setting.setValType(INT);
        if (!isMultiRoot) setting.setPosInf(1);
        setting.output(std::cerr);
        Forest* forest = new Forest(setting);

        /* ExplictFunc to store */
        ExplictFunc EGT;
        std::vector<bool> assignment(parser.getInBits());
        Value outcome;
        int maxOC = 0;
        for (;;) {
            int oc;
            if (!parser.readAssignment(assignment, oc)) break;
            if (oc > maxOC) maxOC = oc;
            // value mapping, TBD
            outcome.setValue(oc, INT);
            EGT.addAssignment(assignment, outcome);
        }
        std::cerr << "Max outcome: " << maxOC << std::endl;

        /* Build BDD */
        EGT.setDefaultValue((isMultiRoot) ? Value(0) : Value(SpecialValue::POS_INF));  // set default value
        // EGT.setDefaultValue(Value(0));
        std::cerr<<"build function\n";
        Func ans = EGT.buildFunc(forest);
        uint64_t numNodes = forest->getNodeManUsed(ans);
        std::cerr << "number of nodes: " << numNodes << std::endl;
        long numStates = 0;
        apply(CARDINALITY, ans, numStates);
        std::cerr << "number of states: " << numStates << std::endl;
        if (numStates != numFun) {
            std::cerr << "[BRAVE_DD] Error!\t Cardinality [" << numStates<<"] does not match number of assignments [" << numFun << "]!" << std::endl;
        }

        if (N<=2 && M<=2){
            DotMaker dot2(forest, "distance_explict");
            dot2.buildGraph(ans);
            dot2.runDot("pdf");
        }

        Func ans_rst(forest);
        apply(CONCRETIZE_RST, ans, Value(SpecialValue::POS_INF), ans_rst);
        std::cerr << "RST: number of nodes: " << forest->getNodeManUsed(ans_rst) << std::endl;

        Func ans_osm(forest);
        apply(CONCRETIZE_OSM, ans, Value(SpecialValue::POS_INF), ans_osm);
        std::cerr << "OSM: number of nodes: " << forest->getNodeManUsed(ans_osm) << std::endl;

        Func ans_tsm(forest);
        apply(CONCRETIZE_TSM, ans, Value(SpecialValue::POS_INF), ans_tsm);
        std::cerr << "TSM: number of nodes: " << forest->getNodeManUsed(ans_tsm) << std::endl;

        delete forest;
    }
    
}