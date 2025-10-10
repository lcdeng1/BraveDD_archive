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

// The size of the puzzle
uint16_t N;         // row
uint16_t M;         // column
uint16_t BITS;      // bits used per position
uint16_t MSB;       // position of MSB
uint64_t SIZE;      // target size of state space
uint64_t MASK;      // mask for bits
uint64_t ROW_MASK;  // mask for M adjacent elements
uint64_t UC_MASK;   // mask for M-1 adjacent elements

// Map to store the states in bits and distance
std::unordered_map<State, uint8_t> distance;

// number of threads
int numThreads = 1;

// flags
bool isOutputTable = 0;
bool isBuildBDD = 0;


/* 
Helper function to print the puzzle state (configure)
*/
void printPuzzleState(const std::vector<std::vector<uint16_t> >& puzzle)
{
    std::cout << "\t";
    for (int i=0; i<M-1; i++) {
        std::cout << "--------";
    }
    std::cout << "-" << std::endl;
    for (const auto& row : puzzle) {
        for (uint16_t val : row) {
            if (val == 0) std::cout << "\t  "; // empty space
            else std::cout << "\t" << val;
        }
        std::cout << std::endl;
    }
    std::cout << "\t";
    for (int i=0; i<M-1; i++) {
        std::cout << "--------";
    }
    std::cout << "-" << std::endl;
    std::cout << std::endl;
}
/*
Print a puzzle state in bits representation
*/
void printBits(const State puzzle)
{
    uint16_t count = 0;
    for (int i=63; i>=0; i--) {
        std::cout << (bool)(puzzle & (0x01ULL<<i));
        count++;
        if (count == BITS) {
            std::cout << "  ";
            count = 0;
        }
    }
    std::cout << std::endl;
}
/*
Encode 64 bits as puzzle state
*/
std::vector<std::vector<uint16_t> > encodeBits2Puzzle(State puzzle)
{
    //
    std::vector<std::vector<uint16_t> > conf(N, std::vector<uint16_t>(M, 0));
    uint16_t eptPos = puzzle & MASK;
    std::cout << "empty postion: " << eptPos << std::endl;
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
uint64_t encodePuzzle2Bits(const std::vector<std::vector<uint16_t> >& puzzle)
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
        neighbor = (initial-1);
    }
    // right
    if ((direction == 1) && (eptPos%M != (M-1)) && (M != 1)) {
        neighbor = (initial+1);
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
        neighbor = start;
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
        neighbor = start;
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

void BFS(State initial) {
    distance.reserve(SIZE);
    distance[initial] = 0;

    std::vector<State> frontier = {initial};
    uint8_t depth = 0;

    while (!frontier.empty()) {
        std::cout << "Depth " << (int)depth << ", frontier = " << frontier.size() << "\n";
        std::vector<State> local_new;
        for (auto &s : frontier) {
            for (auto &ns : getNeighbors(s)) {
                bool inserted = false;
                if (distance.find(ns) == distance.end()) {
                    distance[ns] = depth + 1;
                    inserted = true;
                }
                if (inserted) local_new.push_back(ns);
            }
        }

        // Merge local frontiers into global next frontier
        std::vector<State> next;
        next.insert(next.end(),
                    std::make_move_iterator(local_new.begin()),
                    std::make_move_iterator(local_new.end()));

        frontier.swap(next);
        ++depth;
    }
    std::cout << "Total states discovered: " << distance.size() << "\n";
}

/*
Thread-safe parallel BFS
*/
void parallelBFS(State initial, size_t num_threads) {
    distance.reserve(SIZE);
    distance[initial] = 0;

    std::vector<State> frontier = {initial};
    uint8_t depth = 0;

    while (!frontier.empty()) {
        std::cout << "Depth " << (int)depth << ", frontier = " << frontier.size() << "\n";
        std::vector<std::thread> threads;
        std::mutex dist_mutex;

        std::vector<std::unordered_map<State, uint8_t>> local_distances(num_threads);
        std::vector<std::vector<State>> local_frontiers(num_threads);
        size_t chunk = (frontier.size() + num_threads - 1) / num_threads;

        for (size_t t = 0; t < num_threads; ++t) {
            size_t start = t * chunk;
            if (start >= frontier.size()) break;
            size_t end = std::min(start + chunk, frontier.size());

            threads.emplace_back([&, start, end, t]() {
                std::vector<State> local_new;
                // local_new.reserve(chunk * 4);  // each node → 4 children
                std::unordered_map<State, uint8_t> local_distance;

                for (size_t i = start; i < end; ++i) {
                    State s = frontier[i];
                    for (State ns : getNeighbors(s)) {
                        // bool inserted = false;
                        // {
                            // std::lock_guard<std::mutex> lock(dist_mutex);
                            if (distance.find(ns) == distance.end()) {
                                local_distance[ns] = depth + 1;
                                local_new.push_back(ns);
                                // inserted = true;
                            }
                        // }
                        // if (inserted) local_new.push_back(ns);
                    }
                }
                local_frontiers[t] = std::move(local_new);
                local_distances[t] = std::move(local_distance);
            });
        }

        for (auto& th : threads) th.join();

        // Merge local frontiers and distance into global next frontier and distance
        std::vector<State> next;
        for (auto& lf : local_frontiers) {
            next.insert(next.end(),
                        std::make_move_iterator(lf.begin()),
                        std::make_move_iterator(lf.end()));
        }
        for (auto &ld : local_distances) {
            distance.insert(ld.begin(), ld.end());
        }

        frontier.swap(next);
        ++depth;
    }

    std::cout << "Total states discovered: " << distance.size() << "\n";
}

/*
Output distance table as pla format and compress as xz
*/
void outputPla()
{
    std::string fileName = "puzzle";
    fileName += "_";
    fileName += std::to_string(N);
    fileName += "_";
    fileName += std::to_string(M);
    fileName += ".pla";
    std::ofstream file(fileName, std::ios::app);
    if (!file) {
        std::cerr << "Failed to open file " << fileName << std::endl;
        return;
    }
    file << ".type f\n";
    file << ".i " << N*M*BITS << "\n";
    file << ".o " << 8 << "\n";     // 8 bits for distance is enough
    file << ".p " << distance.size() << "\n";
    for (auto& d : distance) {
        // std::cout << "distance: " << d.second << std::endl;
        // printPuzzleState(encodeBits2Puzzle(d.first));
        // state in bits
        for (int i=MSB; i>=0; i--) {
            file << (bool)(d.first & (0x01ULL<<i));
        }
        file << " ";
        // distance in bits
        for (int i=7; i>=0; i--) {
            if (d.second & (0x01<<i)) {
                file << "1";
            } else {
                file << "~";
            }
        }
        file << "\n";
    }
    file << ".end";
    file.close();
    std::string cmd = "xz ";
    cmd += fileName;
#ifdef _WIN32
    int result = system(("cmd /C " + cmd).c_str());
#else
    int result = system(("sh -c \"" + cmd + "\"").c_str());
#endif
    if (result) {
        std::cout << "[BRAVE_DD] Error!\t Failed to run xz and build file: "<< fileName<< ".xz" << std::endl;
        return;
    }
    std::cout << "Done!" << std::endl;
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
            if (strcmp("-ot", argv[i])==0) {
                isOutputTable = 1;
                continue;
            }
            if (strcmp("-bb", argv[i])==0) {
                isBuildBDD = 1;
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
    std::cout << "Usage: " << name << "[options] N M\n" << std::endl;
    std::cout << "\t            N: the number of board rows" << std::endl;
    std::cout << "\t            M: the number of board columns" << std::endl;
    std::cout << "\t[option]  -nt: followed with a number of threads to BFS distance (default: 1)" << std::endl;
    std::cout << "\t[option]  -bb: switch to turn ON the BDD building" << std::endl;
    return 1;
}

int main(int argc, const char** argv)
{
    
    /* Process arguments and initialize BDD forests*/
    if (!processArgs(argc, argv)) return usage(argv[0]);
    std::cout << "Solving " << N << " x " << M << "-sliding puzzle." << std::endl;
    std::cout << "Using " << getLibInfo(0) << std::endl;
    std::cout << "Target configure: " << std::endl;
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
    std::cout << "bits: " << BITS << std::endl;
    MSB = N*M*BITS-1;
    MASK = ((0x01ULL<<BITS) - 1);
    ROW_MASK = ((0x01ULL<<(M*BITS)) - 1);
    UC_MASK = (0x01ULL<<((M-1)*BITS)) - 1;
    SIZE = expectedSize(N*M);
    // encode in bits
    State initial = encodePuzzle2Bits(conf);
    // search and output as a pla file
    parallelBFS(initial, numThreads);
    // BFS(initial);
    if (isOutputTable) outputPla();

    if (isBuildBDD) {
        /* Parser to read */
        std::string fileName = "puzzle";
        fileName += "_";
        fileName += std::to_string(N);
        fileName += "_";
        fileName += std::to_string(M);
        fileName += ".pla.xz";
        FileReader FR(fileName.c_str());
        ParserPla parser(&FR);
        parser.readHeader();
        long numFun = parser.getNum();
        /* Initial forest */
        ForestSetting setting("fbdd", parser.getInBits());
        setting.setValType(INT);
        setting.setPosInf(1);
        setting.output(std::cout);
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
        std::cout << "Max outcome: " << maxOC << std::endl;

        /* Build BDD */
        EGT.setDefaultValue(Value(SpecialValue::POS_INF));  // set default value
        // EGT.setDefaultValue(Value(0));
        std::cout<<"build function\n";
        Func ans = EGT.buildFunc(forest);
        uint64_t numNodes = forest->getNodeManUsed(ans);
        std::cout << "number of nodes: " << numNodes << std::endl;
        long numStates = 0;
        apply(CARDINALITY, ans, numStates);
        std::cout << "number of states: " << numStates << std::endl;
        if (numStates != numFun) {
            std::cout << "[BRAVE_DD] Error!\t Cardinality [" << numStates<<"] does not match number of assignments [" << numFun << "]!" << std::endl;
        }

        if (N<=2 && M<=2){
            DotMaker dot2(forest, "distance_explict");
            dot2.buildGraph(ans);
            dot2.runDot("pdf");
        }
        delete forest;
    }
    
}