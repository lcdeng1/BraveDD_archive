/*
 * -----------------------------------------------------------------------------
 *  N*M Sliding Puzzle Reachability and Distance Exploration using BDDs
 * -----------------------------------------------------------------------------
 *  Overview:
 *  This program explores the reachable states of the N*M Sliding Puzzle problem
 *  using Binary Decision Diagrams (BDDs). The sliding puzzle consists of N*M
 *  tiles numbered 1..N*M-1 and one empty cell, and states correspond to all
 *  possible tile configurations reachable by legal moves (sliding a tile into
 *  the empty cell).
 * 
 *  Additionally, the program supports:
 *      - Distance computation: calculate the minimal number of moves from the
 *        initial state to each reachable state. This is feasible only for small 
 *        to moderate size
 *      - Concretization of distances: as a post-process to further reduce BDD size
 *        for query-only purposes
 * 
 *  Users can specify:
 *      - The puzzle size (N, M)
 *      - The predefined BDD type to encode the 'state'
 *        (e.g., FBDD, CFBDD, SFBDD, CSFBDD, ZBDD, ESRBDD, CESRBDD, or REXBDD)
 *      - The predefined BMxD type to encode the 'transition'
 *        (e.g., FBMXD, IBMXD)
 *      - Optional distance computation and concretization
 * 
 * 
 * Author: Lichuan Deng
 * Version: 1.0
 * Last Update Date: Oct 27, 2025
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
  * 
  * [Default]:
  * The BDDs encoding a tiles configure:
  * levels[x~(x+bits-1)]=k: tile k at row r and column c,
  * where x = ((r-1)*M+(c-1))*bits+1, bits = log2(N*M).
  * 
  * e.g.:
  * levels[1~4]=2: tile 2 at row 1 and column 1;
  * levels[5~8]=5: tile 5 at row 1 and column 2;
  *  ...
  * levels[57~60]=12: tile 12 at row 4 and column 3;
  * levels[61~64]=4: tile 4 at row 4 and column 4;
  *
  * [Another] (if isEmptyTop):
  * Its BDDs representation from top to bottom is:
  * [16 | 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1]
  *  ^    ^                                              ^
  *  |    |______________________________________________|
  *  |                        |
  *  |        Expand 2-D vector into a 1-D vector
  *  |
  *  +-- top element: the postion of empty
  */

#include <cmath>
#include <iomanip>
#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;
using PuzzleState = std::vector<std::vector<uint16_t>>; // 2D representation of the puzzle

bool isHelp = 0;
// The size of the puzzle
uint16_t N;     // row
uint16_t M;     // column
uint16_t bits;  // bits per position
int     maxStep = 40;

// Timers
timer watch;
double timeLimit = 1800.0, newTimeLimit = timeLimit;
bool isTimeLimitGlobal = 0;
// program flags
bool isEmptyTop = 0;    // new way for encoding
bool isReportToFile = 0;
bool isComputeDistance = 0;
int concretization = 0;
bool isTestConcretize = 0;

// BDD setting
std::string setType, relType, newType;
// BDD forests
Forest* forest1;    // Set
Forest* forest2;    // Relation
Forest* forest3;    // MT or EV for set

// flags if reachable states are computed
bool getRes = 0;
// time (sec) of exploration
double time_explore = 0.0;
// number of reachable states
long num_states = 0;
// number of nodes
uint64_t num_nodes_final = 0, num_nodes_peak = 0, num_rel_nodes_final = 0, num_rel_nodes_peak = 0;

// internal flags
int algorithm = 0;
bool isComputeDistanceMR = 0;
bool isRelationUnion = 0;
bool isConvert = 0;

// set of states reachable to target at exact k steps
std::vector<Func> distance_ex;
// distribution for relations
std::vector<long> distribution_rel_state;
std::vector<long> distribution_rel_node;
// distribution for "exact" distance
std::vector<long> distribution_ex_state;
std::vector<long> distribution_ex_node;

// For concretization
std::vector<int> distance_permutation;
std::vector<Func> distance_ex_concretized;
Func distance_one_root, distance_concretized_one_root;
uint64_t num_nodes_distance_concretized = 0;
uint64_t num_nodes_distance_ex_concretized = 0;
timer watch_concretized;
double time_concretized = 0.0, time_ex_concretized = 0.0;

// For converting
Func distance_convert_ex;
Func distance_convert_ex_concretized;
uint64_t num_nodes_convert_distance = 0;
uint64_t num_nodes_convert_distance_concretized = 0;
double time_convert_concretized = 0.0;

// Helper function to print the puzzle state (configure)
void printPuzzleState(const PuzzleState& puzzle)
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

// Encode the puzzle state (configure) as a BDD
Func encodePuzzle2BDD(const PuzzleState& puzzle)
{
    /* Assuming forest1 is initialized and result starts from constant 1 */
    Func result(forest1);
    if (isComputeDistance) {
        result.constant(0);
    } else {
        result.trueFunc();
    }
    /* Intermediates */
    Func var(forest1);
    uint16_t row, col;
    /* Encoding */
    if (isEmptyTop) {
        bool foundEmpty = 0;
        for (uint16_t i=1; i<=N*M; i++) {
            row = (i%M==0) ? i/M-1 : i/M;
            col = (i%M==0) ? M-1 : i%M-1;
            // found the empty
            if (puzzle[row][col] == 0) {
                foundEmpty = 1;
                uint16_t emptyLvl = (isEmptyTop) ? (N*M-1)*bits+1 : 1;
                for (uint16_t b=0; b<bits; b++) {
                    if (isComputeDistance) {
                        // for distance
                        var.variable(emptyLvl + b, Value(SpecialValue::POS_INF), Value(0));
                        if (i & (0x01<<b)) {
                            apply(MAXIMUM, result, var, result);
                        } else {
                            Func negVar(forest1);
                            negVar.variable(emptyLvl + b, Value(0), Value(SpecialValue::POS_INF));
                            apply(MAXIMUM, result, negVar, result);
                        }
                    } else {
                        // for SSG
                        var.variable(emptyLvl + b, Value(0), Value(1));
                        if (i & (0x01<<b)) {
                            if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {
                                result &= var;
                            } else {
                                apply(MINIMUM, result, var, result);
                            }
                        } else {
                            if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {                    
                                result &= !var;
                            } else {
                                Func negVar(forest1);
                                negVar.variable(emptyLvl + b, Value(1), Value(0));
                                apply(MINIMUM, result, negVar, result);
                            }
                        }
                    }
                }
                continue;
            }
            uint16_t Lvl = (i-1)*bits+1;
            if (foundEmpty) Lvl -= bits;
            for (uint16_t b=0; b<bits; b++) {
                if (isComputeDistance) {
                    // for distance
                    var.variable(Lvl + b, Value(SpecialValue::POS_INF), Value(0));
                    if (puzzle[row][col] & (0x01<<b)) {
                        apply(MAXIMUM, result, var, result);
                    } else {
                        Func negVar(forest1);
                        negVar.variable(Lvl + b, Value(0), Value(SpecialValue::POS_INF));
                        apply(MAXIMUM, result, negVar, result);
                    }
                } else {
                    // for SSG
                    var.variable(Lvl + b, Value(0), Value(1));
                    if (puzzle[row][col] & (0x01<<b)) {
                        if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {
                            result &= var;
                        } else {
                            apply(MINIMUM, result, var, result);
                        }
                    } else {
                        if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {                    
                            result &= !var;
                        } else {
                            Func negVar(forest1);
                            negVar.variable(Lvl + b, Value(1), Value(0));
                            apply(MINIMUM, result, negVar, result);
                        }
                    }
                }
            }
        }
    } else {
        // Default encoding:
        for (uint16_t i=1; i<=N*M; i++) {
            row = (i%M==0) ? i/M-1 : i/M;
            col = (i%M==0) ? M-1 : i%M-1;
            for (uint16_t b=0; b<bits; b++) {
                if (isComputeDistance) {
                    var.variable((i-1)*bits+b+1, Value(SpecialValue::POS_INF), Value(0));
                    if (puzzle[row][col] & (0x01<<b)) {
                        apply(MAXIMUM, result, var, result);
                    } else {
                        Func negVar(forest1);
                        negVar.variable((i-1)*bits+b+1, Value(0), Value(SpecialValue::POS_INF));
                        apply(MAXIMUM, result, negVar, result);
                    }
                } else {
                    var.variable((i-1)*bits+b+1, Value(0), Value(1));
                    if (puzzle[row][col] & (0x01<<b)) {
                        if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {
                            result &= var;
                        } else {
                            apply(MINIMUM, result, var, result);
                        }
                    } else {
                        if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {                    
                            result &= !var;
                        } else {
                            Func negVar(forest1);
                            negVar.variable((i-1)*bits+b+1, Value(1), Value(0));
                            apply(MINIMUM, result, negVar, result);
                        }
                    }
                }
            }
        }
    }
    return result;
}

/** 
 * [Default]:
 * The MxDs encoding forward functions: down, up, left, right 
 * levels[x~(x+bits-1)]=k->0: transition from tile k to 0 at row r and column c,
 * levels[x~(x+bits-1)]=0->k: transition from tile 0 to k at row r and column c,
 * where x = ((r-1)*M+(c-1))*bits+1, bits = log2(N*M).
 * 
 */
// transitions with direction: 1-Down, 2-Up, 3-Left, 4-Right
Func trans(uint16_t from, char direction)  // position FROM and direction
{
    // The result, dependance vector, and position TO
    Func result(forest2);
    std::vector<bool> dependance(N*M*bits+1, 0);
    uint16_t to;
    switch (direction) {
        case 1: // Down
            if (from > (N-1)*M) {   // invalid down
                result.constant(0);
                return result;
            }
            to = from + M;  // position to
            break;
        case 2: // Up
            if (from <= M) {    // invalid up
                result.constant(0);
                return result;
            }
            to = from - M;  // position to
            break;
        case 3: // Left
            if ((from%M == 1) || (M==1)) {  // invalid left
                result.constant(0);
                return result;
            }
            to = from - 1;  // position to
            break;
        case 4: // Right
            if ((from%M == 0) || (M==1)) {  // invalid right
                result.constant(0);
                return result;
            }
            to = from + 1;  // position to
            break;
        default:
            std::cout << "Error: Unknown direction!" << std::endl;
            exit(1);
    }
    /* Intermediates and constant zero*/
    Func varTo(forest2), varFrom(forest2), zero(forest2);
    zero.constant(0);
    /* result starts from constant true */
    result.constant(1);
    if (isEmptyTop) {
        if ((direction == 3) || (direction == 4)) {
            //
        }
        /* Enabling */
        /* Left:    X_empty == from - 1 = to
         * Right:   X_empty == from + 1
         * Down:    X_empty == from + M
         * Up:      X_empty == from - M
         */
        Func empty(forest2);
        uint16_t emptyLvl = (isEmptyTop) ? (N*M-1)*bits+1 : 1;
        for (uint16_t i=0; i<bits; i++) {
            dependance[emptyLvl+i] = 1;
            empty.variable(emptyLvl+i, 0);
            if (!(to & (0x01<<i))) empty = !empty;
            result &= empty;
        }
        /* Firing */
        /* X'_empty = from
         * "X'_to = X_from":
         * Down:    X'_from = X_[to-M+1], X'_[from+1] = X_[to-M+2], ... X'_[to-1] = X_from
         * Up:      X'_to = X_[from-1], X'_[to+1] = X_to, ... X'_[from-1] = X_[to+M-2]
         */
        for (uint16_t i=0; i<bits; i++) {
            empty.variable(emptyLvl+i, 1);
            if (!(from & (0x01<<i))) empty = !empty;
            result &= empty;
        }
        if ((direction == 1) || (direction == 2)) { // Down or Up
            uint16_t startFrom = (direction == 1) ? (from)*bits+1 : (to-1)*bits+1;
            uint16_t startTo = (direction == 1) ? (from-1)*bits+1 : (to)*bits+1;
            // edge case
            for (uint16_t b=0; b<bits; b++) {
                uint16_t startFrom1 = (direction == 1) ? startFrom-bits : startFrom+(M-1)*bits;
                uint16_t startTo1 = (direction == 1) ? startTo+(M-1)*bits : startTo-bits;
                dependance[startFrom1+b] = 1;
                dependance[startTo1+b] = 1;
                varTo.variable(startTo1+b, 1);
                varFrom.variable(startFrom1+b, 0);
                result &= ((varTo & varFrom) | ((!varTo) & (!varFrom)));  // equivalence interface for performance TBD
            }
            for (uint16_t i=0; i<M-1; i++) {
                for (uint16_t b=0; b<bits; b++) {
                    dependance[startTo+i*bits+b] = 1;
                    dependance[startFrom+i*bits+b] = 1;
                    varTo.variable(startTo+i*bits+b, 1);
                    varFrom.variable(startFrom+i*bits+b, 0);
                    result &= ((varTo & varFrom) | ((!varTo) & (!varFrom)));  // equivalence interface for performance TBD
                }
            }
        }
    } else {
        // [Default]
        /* Position FROM and TO's levels start point */
        uint16_t startFrom = (from-1)*bits+1;
        uint16_t startTo = (to-1)*bits+1;
        /* Set dependance vector based on position FROM and TO*/
        for (int i=0; i<bits; i++) {
            dependance[startFrom+i] = 1;
            dependance[startTo+i] = 1;
        }
        /* Enabling */
        // position TO is empty: X_to == 0
        for (uint16_t i=0; i<bits; i++) {
            varTo.variable(startTo+i, 0);
            result &= ((varTo & zero) | ((!varTo) & (!zero)));    // equivalence interface for performance TBD
        }
        /* Firing */
        // sliding tile at position FROM to position TO: X'_to = X_from, X'_from = 0
        for (uint16_t i=0; i<bits; i++) {
            varTo.variable(startTo+i, 1);
            varFrom.variable(startFrom+i, 0);
            result &= ((varTo & varFrom) | ((!varTo) & (!varFrom)));  // equivalence interface for performance TBD
            varFrom.variable(startFrom+i, 1);
            result &= ((varFrom & zero) | ((!varFrom) & (!zero)));    // equivalence interface for performance TBD
        }
    }
    /* Identities (dependance) */
    zero.identity(dependance);
    return result & zero;
}

bool SSG_Frontier(const Func& initial, const std::vector<Func>& relations)
{
    Func curr = initial;
    distance_ex.push_back(curr);
    Func pre(forest1);
    pre.constant(0);
    int n = 0;
    while (true)
    {
        Func next(forest1);
        next.constant(0);
        if (isRelationUnion) {
            long num = 0;
            apply(CARDINALITY, curr, num);
            std::cout << "Frontier: " << n << " size: " << num << std::endl;
            // std::cout << "Frontier: " << n << std::endl;
            apply(POST_IMAGE, curr, relations[0], next);
        } else {
            long num = 0;
            apply(CARDINALITY, curr, num);
            std::cout << "Frontier: " << n << " size: " << num << std::endl;
            Func s_new(forest1);
            for (size_t i=0; i< relations.size(); i++) {
                apply(POST_IMAGE, curr, relations[i], s_new);
                next |= s_new;
            }
        }
        // next |= pre;
        // next = next ^ pre;
        next = next & !pre;
        distance_permutation.push_back(n);
        // check fix point
        if (next.getEdge().isConstantZero()) break;
        // push back
        distance_ex.push_back(next);
        // GC
        for (size_t i=0; i<distance_ex.size(); i++) {
            forest1->markNodes(distance_ex[i]);
        }
        forest1->markSweep();
        // update
        pre = curr;
        curr = next;
        n++;
    }
    return 1;
}

Edge convertEdge(const uint16_t lvl, const Edge& initial, const Edge& witness, const int& dis)
{
    // the final result
    Edge ans;
    // terminal case
    if (witness.getNodeLevel() == 0) {
        if (witness.isConstantOne()) {
            if (forest3->getSetting().getEncodeMechanism() == TERMINAL) {
                ans.setEdgeHandle(makeTerminal(dis));
            } else {
                ans.setEdgeHandle(makeTerminal(SpecialValue::OMEGA));
                ans.setValue(dis);
            }
            ans.setRule(RULE_X);
            return ans;
        } else {
            return initial;
        }
    }
    uint16_t highest = MAX(initial.getNodeLevel(), witness.getNodeLevel());
    std::vector<Edge> child(2);
    child[0] = convertEdge(highest-1,
                            forest3->cofact(highest, initial, 0),
                            forest1->cofact(highest, witness, 0),
                            dis);
    child[1] = convertEdge(highest-1,
                            forest3->cofact(highest, initial, 1),
                            forest1->cofact(highest, witness, 1),
                            dis);
    // reduce edge
    Edge result;
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    result = forest3->reduceEdge(lvl, root, highest, child);
    return result;
}

Func convert()
{
    // start from constant PosInf
    Func ans(forest3);
    ans.constant(SpecialValue::POS_INF);
    Edge converted = ans.getEdge();
    // converting
    for (size_t d=0; d<distance_ex.size()-1; d++) {
        converted = convertEdge(forest3->getSetting().getNumVars(), converted, distance_ex[d].getEdge(), d);
    }
    ans.setEdge(converted);
    return ans;
}

/* Test the correctness of concretization for multi-root BDD */
void testConcretizationMR(std::vector<Func>& mr)
{
    for (size_t d = 0; d<distance_permutation.size()-1; d++) {
        std::vector<bool> assignment(forest1->getSetting().getNumVars()+1, 0);
        for (unsigned long n=0; n<(0x01UL<<forest1->getSetting().getNumVars()); n++) {
            for (size_t k=1; k<=assignment.size()-1; k++) {
                assignment[k] = n & (1<<(k-1));
            }
            if (distance_ex[distance_permutation[d]].evaluate(assignment) != Value(1)) {continue;}
            bool before = 0;
            for (size_t i=0; i<d; i++) {
                if (mr[i].evaluate(assignment) == Value(1)) {
                    before = 1;
                    break;
                }
            }
            if ((distance_ex[distance_permutation[d]].evaluate(assignment) == Value(1)) && ((mr[d].evaluate(assignment) != Value(1)) || before)) {
                std::cout << "Error! Evaluation for concretization\n";
                exit(1);
            }
        }
    }
}

/* Test the correctness of concretization for one-root BDD */
void testConcretizationOR(Func& r)
{
    // test
    std::vector<bool> assignment(forest1->getSetting().getNumVars()+1, 0);
    for (unsigned long n=0; n<(0x01UL<<forest1->getSetting().getNumVars()); n++) {
        for (size_t k=1; k<=assignment.size()-1; k++) {
            assignment[k] = n & (1<<(k-1));
        }
        if (distance_one_root.evaluate(assignment) == Value(SpecialValue::POS_INF)) {continue;}
        if ((distance_one_root.evaluate(assignment) != Value(SpecialValue::POS_INF))
            && (distance_one_root.evaluate(assignment) != r.evaluate(assignment))) {
            std::cout << "Error! Evaluation for concretization\n";
            exit(1);
        }
    }
}

// Restrict
// ===============================================================================================================================================================
// for multi-root
Func concretizeFunc_rst(const Func& initial, const Func& dc)
{
    Func result(initial.getForest());
    apply(CONCRETIZE_RST, initial, dc, result);
    return result;
}
// for one-root
Func concretizeFunc_rst(const Func& initial)
{
    Func result(initial.getForest());
    apply(CONCRETIZE_RST, initial, Value(SpecialValue::POS_INF), result);
    return result;
}
// ===============================================================================================================================================================

// One-sided-match
// ===============================================================================================================================================================
// for multi-root
Func concretizeFunc_osm(const Func& initial, const Func& dc)
{
    Func result(initial.getForest());
    apply(CONCRETIZE_OSM, initial, dc, result);
    return result;
}
// for one-root
Func concretizeFunc_osm(const Func& initial)
{
    Func result(initial.getForest());
    apply(CONCRETIZE_OSM, initial, Value(SpecialValue::POS_INF), result);
    return result;
}
// ===============================================================================================================================================================

// Two-sided-match
// ===============================================================================================================================================================
// for multi-root
Func concretizeFunc_tsm(const Func& initial, const Func& dc)
{
    Func result(initial.getForest());
    apply(CONCRETIZE_TSM, initial, dc, result);
    return result;
}
// for one-root
Func concretizeFunc_tsm(const Func& initial)
{
    Func result(initial.getForest());
    apply(CONCRETIZE_TSM, initial, Value(SpecialValue::POS_INF), result);
    return result;
}
// ===============================================================================================================================================================

/* main function to concretize using different heuristics 
 * 0: Restrict; 1: One-sided-match; 2: Two-sided-match
 */
void concretize(const int heuristics)
{
    if (isComputeDistanceMR) {
        // update permutation
        std::sort(distance_permutation.begin(), distance_permutation.end(),
                    [&](size_t i, size_t j) { return distribution_ex_node[i] < distribution_ex_node[j]; });

        watch_concretized.reset();
        watch_concretized.note_time();
        /* for exact distance */
        Func dc = distance_ex.back();
        if (heuristics == 0) {
            distance_ex_concretized.push_back(concretizeFunc_rst(distance_ex[distance_permutation[0]], dc));
        } else if (heuristics == 1) {
            distance_ex_concretized.push_back(concretizeFunc_osm(distance_ex[distance_permutation[0]], dc));
        } else if (heuristics == 2) {
            distance_ex_concretized.push_back(concretizeFunc_tsm(distance_ex[distance_permutation[0]], dc));
        }
        for (size_t i=1; i<distance_permutation.size()-1; i++) {    // -1: remove the last one
            // update dc
            for (size_t k=0; k<i; k++) {
                dc |= distance_ex[distance_permutation[k]];
            }
            // concretizing
            if (heuristics == 0) {
                distance_ex_concretized.push_back(concretizeFunc_rst(distance_ex[distance_permutation[i]], dc));
            } else if (heuristics == 1) {
                distance_ex_concretized.push_back(concretizeFunc_osm(distance_ex[distance_permutation[i]], dc));
            } else if (heuristics == 2) {
                distance_ex_concretized.push_back(concretizeFunc_tsm(distance_ex[distance_permutation[i]], dc));
            }
        }
        watch_concretized.note_time();
        time_concretized = watch_concretized.get_last_seconds();
        num_nodes_distance_ex_concretized = forest1->getNodeManUsed(distance_ex_concretized);
        std::cout << "done concretization\n";
        // test
        if (isTestConcretize) testConcretizationMR(distance_ex_concretized);
    } else {
        watch_concretized.reset();
        watch_concretized.note_time();
        // for non multi-root
        if (heuristics == 0) {
            distance_concretized_one_root = concretizeFunc_rst(distance_one_root);
        } else if (heuristics == 1) {
            distance_concretized_one_root = concretizeFunc_osm(distance_one_root);
        } else if (heuristics == 2) {
            distance_concretized_one_root = concretizeFunc_tsm(distance_one_root);
        } else {
            std::cerr << "Unknown concretization heuristics!" << std::endl;
            return;
        }
        watch_concretized.note_time();
        time_concretized = watch_concretized.get_last_seconds();
        num_nodes_distance_ex_concretized = forest1->getNodeManUsed(distance_concretized_one_root);
        std::cout << "done concretization\n";
        // test
        if (isTestConcretize) testConcretizationOR(distance_concretized_one_root);
    }
}

void compute_saturation(const Func& target, const std::vector<Func>& relations)
{
    Func states_Sat(forest1);
    // Timer start
    watch.reset();
    watch.note_time();
    apply(SATURATE, target, relations, states_Sat);
    watch.note_time();
    getRes = 1;
    // save for concretization
    distance_one_root = states_Sat;
    // report cache
    // UOPs.reportCacheStat(std::cout);
    // BOPs.reportCacheStat(std::cout);
    // SOPs.reportCacheStat(std::cout);
    // record time
    time_explore = watch.get_last_seconds();
    // record #states
    apply(CARDINALITY, states_Sat, num_states);
    // record #nodes
    num_nodes_final = forest1->getNodeManUsed(states_Sat);
    num_nodes_peak = forest1->getNodeManPeak();

    forest1->registerFunc(states_Sat);
    // /* Update time limit */
    // if (!isTimeLimitGlobal) newTimeLimit = time_Sat;
}

void compute_BFS(const Func& target, const std::vector<Func>& relations)
{
    watch.reset();
    watch.note_time();
    getRes = SSG_Frontier(target, relations);
    watch.note_time();
    // report cache
    // UOPs.reportCacheStat(std::cout);
    // BOPs.reportCacheStat(std::cout);
    // record time
    time_explore = watch.get_last_seconds();
    // record #nodes
    num_nodes_final = forest1->getNodeManUsed(distance_ex);
    num_nodes_peak = forest1->getNodeManPeak();
    // compute distribution
    long num_state = 0;
    for (size_t i=0; i<distance_ex.size(); i++) {
        apply(CARDINALITY, distance_ex[i], num_state);
        distribution_ex_state.push_back(num_state);
        distribution_ex_node.push_back(forest1->getNodeManUsed(distance_ex[i]));
        num_states += num_state;
    }
    // push back don't care set
    Func all(forest1);
    all.constant(1);
    for (size_t i=0; i<distance_ex.size(); i++) {
        all = all & !distance_ex[i];
    }
    distance_ex.push_back(all);
    // push back #state and #node for don't-care set
    apply(CARDINALITY, distance_ex.back(), num_state);
    distribution_ex_state.push_back(num_state);
    distribution_ex_node.push_back(forest1->getNodeManUsed(distance_ex.back()));
}

int usage(const char* who)
{
    std::ostream& out = std::cerr;
    int align = 10;
    /* Strip leading directory, if any: */
    const char* name = who;
    for (const char* ptr=who; *ptr; ptr++) {
        if ('/' == *ptr) name = ptr+1;
    }
    out << std::left << std::setw(align) << "USAGE: " << name << " <N> <M>" << std::endl;
    out << std::endl;
    out << std::left << std::setw(align) << "OPTIONS: "<< std::endl;
    out << std::left << std::setw(2*align) << "  -help, -h " << "Display help message and usage" << std::endl;
    out << std::left << std::setw(2*align) << "  -setType, -st <string>" << std::endl;
    out << std::left << std::setw(2*align) << "" << "Select the predefined BDD type. Default: FBDD" << std::endl;
    out << std::left << std::setw(2*align) << "" << "Supported (for set): FBDD, CFBDD, SFBDD, CSFBDD, ZBDD, ESRBDD, CESRBDD, REXBDD" << std::endl;
    out << std::left << std::setw(2*align) << "" << "Supported (for distance): MTBDD EVFBDD" << std::endl;
    out << std::left << std::setw(2*align) << "  -relType, -rt <string>" << std::endl;
    out << std::left << std::setw(2*align) << "" << "Select the predefined BMxD type. Default: IBMxD" << std::endl;
    out << std::left << std::setw(2*align) << "" << "Supported: FBMxD, IBMxD" << std::endl;
    out << std::left << std::setw(2*align) << "  -distance, -d" << "Compute distance for each reachable state" << std::endl;
    out << std::left << std::setw(2*align) << "  -concretize, -cz <number>" << std::endl;
    out << std::left << std::setw(2*align) << "" << "Set the concretization heuristics. Default: 0" << std::endl;
    out << std::left << std::setw(2*align) << "" << "Supported: 0: No concretization; 1: Restrict; 2: One-sided-match (OSM); 3: Two-sided-match (TSM)" << std::endl;
    out << std::endl;
    out << std::left << std::setw(align) << "EXAMPLES: "<< std::endl;
    out << std::left << std::setw(align) << "" << name << " 2 3" << std::endl;
    out << std::left << std::setw(align) << "" << name << " 2 3 -st rexbdd" << std::endl;
    out << std::left << std::setw(align) << "" << name << " 2 3 -st mtbdd -d" << std::endl;
    return 1;
}

/* Help message */
int helpInfo(const char* who)
{
    std::ostream& out = std::cerr;
    int align = 10;
    out << std::left << std::setw(align) << "OVERVIEW: "
    << R"(
    This program encodes the states of an N*M-Sliding Puzzle using Binary Decision
    Diagrams (BDDs) and represents the corresponding transition relations with Binary
    Matrix Diagrams (BMxDs). It symbolically explores the reachable state space.
    Users can select different types of BDDs and BMxDs, as well as various heuristic 
    concretization algorithms, to futher compress the optionally generated BDDs that 
    represent distance functions.
    )"
    << std::endl;
    out << std::left << std::setw(align) << "NOTE: "
    << R"(
    For pure state space exploration and BDD generation, this program employs the 
    "Saturation" algorithm. For distance function generation, if the selected 
    BDD represents a Boolean-valued domain, this program uses "BFS" to construct 
    a node-sharing BDD "forest" with multiple root edges, where each root edge 
    corresponds to the set of states at a specific distance. Otherwise, this program
    also uses "Saturation" algorithm for computation. However, for a larger 
    puzzles (e.g., N*M >= 9), this program opts to first generate a multi-root 
    node-sharing BDD "forest" via BFS and subsequently convert it into the 
    corresponding selected non-Boolean domain BDD representation.
    )"
    << std::endl;
    out << "----------------------------------------------------------" << std::endl;
    return usage(who);
}

/* Parse the arguments */
bool processArgs(int argc, const char** argv)
{
    // default types
    setType = "FBDD";
    relType = "IBMXD";
    bool setN = 0, setM = 0;
    for (int i=1; i<argc; i++) {
        if ('-' == argv[i][0]) {
            // options other than size
            if ((strcmp("-help", argv[i])==0) || (strcmp("-h", argv[i])==0)) {
                isHelp = 1;
                return 1;
            }
            if ((strcmp("-setType", argv[i])==0) || (strcmp("-st", argv[i])==0)) {
                setType = argv[i+1];
                i++;
                continue;
            }
            if ((strcmp("-relType", argv[i])==0) || (strcmp("-rt", argv[i])==0)) {
                relType = argv[i+1];
                i++;
                continue;
            }
            if ((strcmp("-distance", argv[i])==0) || (strcmp("-d", argv[i])==0)) {
                isComputeDistance = 1;
                continue;
            }
            if ((strcmp("-concretize", argv[i])==0) || (strcmp("-cz", argv[i])==0)) {
                concretization = atoi(argv[i+1]);
                i++;
                continue;
            }
            /* Hide to users */
            // option for time out
            if (strcmp("-t", argv[i])==0) {
                timeLimit = std::stod(argv[i+1]);
                newTimeLimit = timeLimit;
                i++;
                continue;
            }
            if (strcmp("-tg", argv[i])==0) {
                isTimeLimitGlobal = 1;
                continue;
            }
            // option for report
            if (strcmp("-f", argv[i])==0) {
                isReportToFile = 1;
                continue;
            }
            // option for encoding
            if (strcmp("-et", argv[i])==0) {
                isEmptyTop = 1;
                continue;
            }
            // option for testing concretization result
            if (strcmp("-to", argv[i])==0) {
                isTestConcretize = 1;
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

    if (!setN || N<1 || !setM || M<1) return 1;
    return 0;
}

void report(std::ostream& out)
{
    int align = 36;
    std::string explore_alg = "";
    if (algorithm == 0) {
        explore_alg = "Saturation";
    } else if (algorithm == 1) {
        explore_alg = "Chain-search";
    } else if (algorithm == 2) {
        explore_alg = "BFS";
    }
    std::string concretization_alg = "";
    if (concretization == 1) {
        concretization_alg = "Restrict";
    } else if (concretization == 2) {
        concretization_alg = "OSM";
    } else if (concretization == 3) {
        concretization_alg = "TSM";
    }
    out << "=========================| Node |=========================" << std::endl;
    if (isRelationUnion) {
        out << std::left << std::setw(align/2) << "Relations " << std::setw(align/3) << "(Union):" << distribution_rel_node[0] << std::endl;
    } else {
        for (size_t i=0; i<distribution_rel_node.size(); i++) {
            out << std::left << std::setw(10) << "Relations " << std::setw(20) << i << distribution_rel_node[i] << std::endl;
        }
    }
    out << std::left << std::setw(align/1.2) << "Relations Final: " << num_rel_nodes_final << std::endl;
    out << std::left << std::setw(align/1.2) << "Relations Peak: " << num_rel_nodes_peak << std::endl;
    out << "----------------------------------------------------------" << std::endl;
    if (isComputeDistance || isComputeDistanceMR) {
        out << std::left << std::setw(align/1.2) << "Final: " << std::setw(align/3) << "" << ((isConvert) ? num_nodes_convert_distance : num_nodes_final) << std::endl;
        out << std::left << std::setw(align/1.2) << "Peak: " << std::setw(align/3) << ((isConvert) ? forest1->getSetting().getName() : "")<< num_nodes_peak << std::endl;
        if (concretization != 0) {
            out << std::left << std::setw(align/1.2) << "Concretization (exact)" << std::setw(align/3) << concretization_alg << ((isConvert) ? num_nodes_convert_distance_concretized : num_nodes_distance_ex_concretized) << std::endl;
        }
    } else {
        out << std::left << std::setw(align) << "Final: " << num_nodes_final << std::endl;
        out << std::left << std::setw(align) << "Peak: " << num_nodes_peak << std::endl;
    }
    out << "=========================| State |========================" << std::endl;
    out << std::left << std::setw(align) << explore_alg << num_states << std::endl;

    out << "=========================| Time |=========================" << std::endl;
    out << std::left << std::setw(align) << explore_alg << time_explore << std::endl;
    out << "----------------------------------------------------------" << std::endl;
    if (concretization != 0) {
        out << std::left << std::setw(align/1.2) << "Concretization (exact)" << std::setw(align/3) << concretization_alg << ((isConvert) ? time_convert_concretized : time_concretized) << std::endl;
    }
    /* the end */
    out << "**********************************************************" << std::endl;
}

int main(int argc, const char** argv)
{
    /* Process arguments and initialize BDD forests*/
    if (processArgs(argc, argv)) {
        if (isHelp) return helpInfo(argv[0]);
        return usage(argv[0]);
    }
    std::cout << "Solving " << N << " x " << M << "-sliding puzzle." << std::endl;
    std::cout << "Using " << getLibInfo(0) << std::endl;
    std::cout << "Target configure: " << std::endl;
    // default configure
    PuzzleState conf(N, std::vector<uint16_t>(M, 0));
    for (uint16_t i=1; i<N*M; i++) {
        conf[(i%M==0)?i/M-1:i/M][(i%M==0)?M-1:i%M-1] = i;
    }
    printPuzzleState(conf);
    // initialize forests
    bits = static_cast<uint16_t>(std::ceil(log2(N*M)));
    std::cout << "bits: " << bits << std::endl;
    uint16_t levels = bits * N * M;
    ForestSetting setting1(setType, levels);
    ForestSetting setting2(relType, levels);
    if (setting1.getRangeType() == BOOLEAN) {
        // compute distance?
        if (isComputeDistance) {
            isComputeDistance = 0;
            isComputeDistanceMR = 1;
            isRelationUnion = 1;
            algorithm = 2;
        }
    } else {
        // compute distance?
        if (isComputeDistance) {
            setting1.setPosInf(1);  // trun on to support special value positive infinity
            if (N*M >= 9) {
                // time is too long, converting from multi-root
                isComputeDistance = 0;
                isComputeDistanceMR = 1;
                isRelationUnion = 1;
                algorithm = 2;
                setting1 = ForestSetting(PredefForest::CSFBDD, levels);
                isConvert = 1;
                // setting for MT or EV that is converted from MR
                ForestSetting setting3(setType, levels);
                // setting3.setRangeType(INTEGER);
                setting3.setPosInf(1);
                forest3 = new Forest(setting3);
            }
        } else {
            std::cerr << "##[Error]: Inapproprite BDD type##" << std::endl;
            std::cerr << std::left << std::setw(11) << "" << "Supported: FBDD, CFBDD, SFBDD, CSFBDD, ZBDD, ESRBDD, CESRBDD, REXBDD" << std::endl;
            exit(1);
        }
    }
    forest1 = new Forest(setting1);
    forest2 = new Forest(setting2);

    /* Encode final target configure to BDD */
    Func target(forest1);
    target = encodePuzzle2BDD(conf);
    forest1->registerFunc(target);

    /* Build forward functions 
     * each position takes 4 slots for down, up, left, right
     * some may be constant 0 because of invalid forward direction
     */ 
    Func forward(forest2);
    std::vector<Func> relations;
    long numStateRel = 0;
    for (int position=1; position<=N*M; position++) {
        for (int direction = 1; direction<=4; direction++) {
            if (((direction == 1) && (position > (N-1)*M)) 
                || ((direction == 2) && (position <= M))
                || ((direction == 3) && ((position%M == 1) || (M==1)))
                || ((direction == 4) && ((position%M == 0) || (M==1)))) {
                //invalid relation
                continue;
            }
            forward = trans(position, direction);
            relations.push_back(forward);
            forest2->registerFunc(forward);
            distribution_rel_node.push_back(forest2->getNodeManUsed(forward));
            apply(CARDINALITY, forward, numStateRel);
            distribution_rel_state.push_back(numStateRel);
            // try to union relations
            if (isRelationUnion) {
                relations[0] |= forward;
            }
        }
    }
    std::cout << "Number of valid relations: " << relations.size() << std::endl;
    // count nodes
    num_rel_nodes_final = (isRelationUnion) ? forest2->getNodeManUsed(relations[0]) : forest2->getNodeManUsed(relations);
    num_rel_nodes_peak = forest2->getNodeManPeak();

    /* Explore reachable states */
    if (algorithm == 0) {
        compute_saturation(target, relations);
    } else if (algorithm == 1) {
        // compute_chain(target, relations);
    } else if (algorithm == 2) {
        compute_BFS(target, relations);
    }

    /* Concretization */
    if (isConvert) {
        distance_convert_ex = convert();
        num_nodes_convert_distance = forest3->getNodeManUsed(distance_convert_ex);
        if (concretization != 0) {
            // restrict
            watch_concretized.reset();
            watch_concretized.note_time();
            if (concretization == 1) {
                distance_convert_ex_concretized = concretizeFunc_rst(distance_convert_ex);
            } else if (concretization == 2) {
                distance_convert_ex_concretized = concretizeFunc_osm(distance_convert_ex);
            } else if (concretization == 3) {
                distance_convert_ex_concretized = concretizeFunc_tsm(distance_convert_ex);
            } else {
                std::cerr << "Unknown concretization heuristics!" << std::endl;
                return 1;
            }
            watch_concretized.note_time();
            time_convert_concretized = watch_concretized.get_last_seconds();
            num_nodes_convert_distance_concretized = forest3->getNodeManUsed(distance_convert_ex_concretized);
        }
    } else {
        if (concretization != 0) {
            concretize(concretization-1);
        }
    }    

    /* Report result */
    if (!isReportToFile) {
        std::cout << "Done!" << std::endl;
        std::cout << "**********************************************************" << std::endl;
        int align = 30;
        std::cout << std::left << std::setw(align) << "Sliding puzzle:" << N << " x " << M  << std::endl;
        std::cout << std::left << std::setw(align) << "Bits per position: " << bits << std::endl;
        std::cout << std::left << std::setw(align) << "Level:" << forest1->getSetting().getNumVars() << std::endl;
        std::cout << std::left << std::setw(align) << "The [Set] type:" << ((isConvert)? forest3 : forest1)->getSetting().getName() << std::endl;
        std::string encodingType = "Default";
        if (isEmptyTop) encodingType = "Empty_Top";
        std::cout << std::left << std::setw(align) << "Encoding type:" << encodingType << std::endl;
        std::cout << std::left << std::setw(align) << "The [Relation] type:" << forest2->getSetting().getName() << std::endl;
        report(std::cout);
    } else {
        std::cout << "**********************************************************" << std::endl;
        std::string fileName = ((isConvert) ? forest3 : forest1)->getSetting().getName();
        fileName += "_";
        fileName += forest2->getSetting().getName();
        fileName += "_";
        fileName += std::to_string(N);
        fileName += "_";
        fileName += std::to_string(M);
        if (isComputeDistance) {
            fileName += "_Dis";
        }
        if (isComputeDistanceMR) {
            fileName += "_MR";
        }
        if (isRelationUnion) {
            fileName += "_UR";
        }
        if (isEmptyTop) {
            fileName += "_ET";
        }
        fileName += ".txt";
        std::ofstream file(fileName, std::ios::app);
        if (!file) {
            std::cerr << "Failed to open file " << fileName << std::endl;
        } else {
            report(file);
            file.close();
        }
        std::cout << "Done!" << std::endl;
    }

    // clean
    delete forest1;
    delete forest2;
    if (forest3) delete forest3;
    return 0;
}