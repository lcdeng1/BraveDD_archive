/* This is an example of building the set of configurations (states) reachable to target configure for
 * user-specified sliding puzzle.
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
  * BDD construction:
  *  ...
  * levels[5~8]=0101 (5 in binary)
  *      N8[0] -> N7, N8[1] -> T0
  *      N7[1] -> N6, N7[0] -> T0
  *      N6[0] -> N5, N6[1] -> T0
  *      N5[1] -> N4, N5[0] -> T0
  * levels[1~4]=0010 (2 in binary)
  *      N4[0] -> N3, N4[1] -> T0
  *      N3[0] -> N2, N3[1] -> T0
  *      N2[1] -> N1, N2[0] -> T0
  *      N1[0] -> T1, N1[1] -> T0
  *
  * [Another] (if isEmptyTop or isEmptyBot):
  * Its BDDs representation from bottom up is:
  * [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 | 16]
  *  ^                                              ^    ^
  *  |______________________________________________|    |
  *                           |                          |
  *           Expand 2-D vector into a 1-D vector        +-- last elmt: the postion of empty (if isEmptyTop)
  */

#include <queue>
#include <cmath>
#include <iomanip>
#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;

// The size of the puzzle
uint16_t N;     // row
uint16_t M;     // column
uint16_t bits;  // bits per position
int     maxStep = 40;

using PuzzleState = std::vector<std::vector<uint16_t> >; // 2D representation of the puzzle

// Timers
timer watchSat, watchChain, watchBFS, watchBFS0, watchBFS_ex;
double timeLimit = 1800.0;
bool isTimeLimitGlobal = 0;
// program flags
bool isReportToFile = 0;
bool isComputeDistance = 0;
bool isComputeDistanceBFS = 0;
bool isRelationUnion = 0;
bool isEmptyTop = 0;    // new way for encoding
bool isEmptyBot = 0;    // ...
bool isConcretize = 0;
bool isTestConcretize = 1;
bool isConvert = 0;

/* flags if results are computed */
bool getRes_Sat = 0, getRes_BFS = 0, getRes_chain = 0, getRes_correct = 0;
/* time (sec) of each method */
double time_Sat = 0.0, time_BFS = 0.0, time_chain = 0.0, time_correct = 0.0, newTimeLimit = timeLimit;
/* #state of each methods */
long state_Sat = 0, state_BFS = 0, state_chain = 0, state_correct = 0;
/* #nodes of each methods */
uint64_t nodes_Sat = 0, nodes_BFS = 0, nodes_chain = 0, nodes_correct = 0, nodes_rel = 0;
/* peak number of nodes */
uint64_t nodes_Sat_Peak = 0, nodes_BFS_Peak = 0, nodes_chain_Peak = 0, nodes_rel_Peak = 0;

// set of states reachable to target up to k steps
std::vector<Func> distance;
// set of states reachable to target at exact k steps
std::vector<Func> distance_ex;
// distribution for relations
std::vector<long> distribution_rel_state;
std::vector<long> distribution_rel_node;
// distribution for "up-to" distance
std::vector<long> distribution_state;
std::vector<long> distribution_node;
// distribution for "exact" distance
std::vector<long> distribution_ex_state;
std::vector<long> distribution_ex_node;
// number of final nodes for "up-to" distance
uint64_t nodes_dis_BFS = 0;
uint64_t nodes_dis_BFS_peak = 0;
// number of final nodes for "exact" distance
uint64_t nodes_dis_ex_BFS = 0;
uint64_t nodes_dis_ex_BFS_peak = 0;
// time for "exact" distance
double time_BFS_ex = 0.0;

// For concretization
std::vector<int> distance_permutation;
std::vector<int> distance_permutation_opt;
std::vector<int> distance_permutation_ex_opt;
std::vector<Func> distance_rst;
std::vector<Func> distance_ex_rst;
std::vector<Func> distance_osm;
std::vector<Func> distance_ex_osm;
std::vector<Func> distance_tsm;
std::vector<Func> distance_ex_tsm;
Func distance_one_root, distance_one_root_rst, distance_one_root_osm, distance_one_root_tsm;
uint64_t nodes_dis_rst = 0;
uint64_t nodes_dis_osm = 0;
uint64_t nodes_dis_tsm = 0;
uint64_t nodes_dis_ex_rst = 0;
uint64_t nodes_dis_ex_osm = 0;
uint64_t nodes_dis_ex_tsm = 0;
timer watch_rst, watch_osm, watch_tsm;
double time_rst = 0.0;
double time_osm = 0.0;
double time_tsm = 0.0;
double time_ex_rst = 0.0;
double time_ex_osm = 0.0;
double time_ex_tsm = 0.0;

// For converting
Func distance_convert_ex;
Func distance_convert_ex_rst;
Func distance_convert_ex_osm;
Func distance_convert_ex_tsm;
uint64_t nodes_convert_dis_ex = 0;
uint64_t nodes_convert_dis_ex_rst = 0;
uint64_t nodes_convert_dis_ex_osm = 0;
uint64_t nodes_convert_dis_ex_tsm = 0;
timer watch_convert_rst, watch_convert_osm, watch_convert_tsm;
double time_convert_rst = 0.0;
double time_convert_osm = 0.0;
double time_convert_tsm = 0.0;

/* shape of state space */
int radius = 0;
int last_width = 0;
int depth = 0;
long max_width = 0;

// BDD setting
std::string setType, relType, newType;

// BDD forests
Forest* forest1;    // Set
Forest* forest2;    // Relation
Forest* forest3;    // MT or EV for set

// Helper function to print the puzzle state (configure)
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

// Encode the puzzle state (configure) as a BDD
Func encodePuzzle2BDD(const std::vector<std::vector<uint16_t> >& puzzle)
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
    if (isEmptyTop || isEmptyBot) {
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
            if (isEmptyBot) Lvl += bits;
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
    if (isEmptyTop || isEmptyBot) {
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
            if (isEmptyBot) {
                startFrom += bits;
                startTo += bits;
            }
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

bool evalTrans(const std::vector<Func>& relations)
{
    // constant 0
    Func result(forest2);
    result.constant(0);

    uint16_t id = 0;
    for (int from=1; from<=N*M; from++) {
        for (int direction = 1; direction<=4; direction++) {
            uint16_t to = 0;
            id = 4*(from-1)+direction-1;
            switch (direction) {
                case 1: // Down
                    if (from > (N-1)*M) {   // invalid down
                        return relations[id] == result;
                    }
                    to = from + M;  // position to
                    break;
                case 2: // Up
                    if (from <= M) {    // invalid up
                        return relations[id] == result;
                    }
                    to = from - M;  // position to
                    break;
                case 3: // Left
                    if (from%M == 1) {  // invalid left
                        return relations[id] == result;
                    }
                    to = from - 1;  // position to
                    break;
                case 4: // Right
                    if (from%M == 0) {  // invalid right
                        return relations[id] == result;
                    }
                    to = from + 1;  // position to
                    break;
                default:
                    std::cout << "Error: Unknown direction!" << std::endl;
                    exit(1);
            }
            /* Position FROM and TO's levels start point */
            uint16_t startFrom = (from-1)*bits+1;
            uint16_t startTo = (to-1)*bits+1;
            /* check card first */
            long card = 0;
            long A = (1 << ((from < to) ? startFrom-1 : startTo-1));
            long B = (1 << ((from < to) ? startTo-startFrom-bits : startFrom-startTo-bits));
            long C = (1 << ((from < to) ? N*M*bits-startTo-bits+1 : N*M*bits-startFrom-bits+1));
            apply(CARDINALITY, relations[id], card);
            if (card != A*B*C*(0x01 << bits)) {
                std::cout << "Error: relation cardinality wrong! It was " << card << ", should be " << (0x01 << bits) << std::endl;
                std::cout << "\tPosition from: " << from << ", to: " << to << std::endl;
                if ((N==2) && (from==1) && (direction==1)) {
                    DotMaker dot0(forest2, "err_relation");
                    dot0.buildGraph(relations[id]);
                    dot0.runDot("pdf");
                }
                exit(0);
            }
            /* evaluation */
            std::vector<bool> assignFrom(N*M*bits+1, 0);
            std::vector<bool> assignTo(N*M*bits+1, 0);
            for (int tile=0; tile<(0x01<<bits); tile++) {
                for (int i=0; i<bits; i++) {
                    if (tile & (0x01<<i)) {
                        assignFrom[startFrom+i] = 1;
                        assignTo[startTo+i] = 1;
                    }
                }
                Value val = relations[id].evaluate(assignFrom, assignTo);
                int valInt;
                val.getValueTo(&valInt, INT);
                if (valInt != 1) {
                    std::cout << "Error: relation evaluated wrong!" << std::endl;
                    std::cout << "\tPosition from: " << from << ", to: " << to << "; tile: " << tile << std::endl;
                    exit(0);
                }
            }
        }
    }
    return 1;
}

bool testImage(const Func& set, const std::vector<Func>& rels, const Func& res)
{
    bool isPass = 0;
    uint16_t num = N*M*bits;
    // variables
    std::vector<bool> from(num+1, 0);
    std::vector<bool> to(num+1, 0);
    // evaluation
    bool hasTo = 0;
    for (long long i=0; i<(1<<num); i++) {
        // to variables
        for (uint16_t k=1; k<=num; k++) {
            to[k] = i&(1<<(k-1));
        }
        int valIntRes;
        Value evalRes = res.evaluate(to);
        evalRes.getValueTo(&valIntRes, INT);
        // check if any 'from' state go to 'to' state
        for (long long j=0; j<(1<<num); j++) {
            // from variables
            for (uint16_t k=1; k<=num; k++) {
                from[k] = j&(1<<(k-1));
            }
            int valIntS = 0, valIntR = 0;
            Value evalS = set.evaluate(from);
            evalS.getValueTo(&valIntS, INT);
            if (valIntS != 1) continue;
            // from state exists
            for (size_t n=0; n<rels.size(); n++) {
                Value evalR = rels[n].evaluate(from, to);
                evalR.getValueTo(&valIntR, INT);
                if (valIntR == 1) break;
            }
            hasTo |= (bool)valIntR;
            if (hasTo) break;
        }
        if (hasTo != (bool)valIntRes) {
            std::cout<<"testImage(): Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
            std::cout<<"\t assignment (from): ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << from[k] << " ";
            }
            std::cout<<"\n\t assignment (to): ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << to[k] << " ";
            }
            std::cout << "\nvalue shoud be: " << hasTo << "; was: " << valIntRes << std::endl;

            std::cout << std::endl;

            // DotMaker dot0(set, "error_from");
            // dot0.buildGraph(functionS);
            // dot0.runDot("pdf");
            // DotMaker dot1(forestS, "error_to");
            // dot1.buildGraph(functionS1);
            // dot1.runDot("pdf");
            // DotMaker dot2(forestR, "error_rel");
            // dot2.buildGraph(functionR);
            // dot2.runDot("pdf");
            return 0;
        }
        hasTo = 0;
    }
    isPass = 1;

    return isPass;
}

bool SSG_BFS(const Func& initial, const std::vector<Func>& relations, Func& target, const double time)
{
    Func curr = initial;
    if (isComputeDistanceBFS) distance.push_back(curr);
    Func nextStep(forest1);
    Func FF(forest1);
    FF.falseFunc();
    Func SS = FF;
    int n = 0;
    // long card_SS = 0, card_curr = 0;
    while (true) {
        if (!isRelationUnion) {
            for (size_t i=0; i<relations.size(); i++) {
                std::cout << "BFS image process: " << n << " : (" << i << "/" << relations.size() << ")" << std::endl;
                // change for distance computing
                apply(POST_IMAGE, curr, relations[i], nextStep);
                if (isComputeDistance) {
                    apply(MINIMUM, FF, nextStep, FF);
                } else {
                    if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {
                        FF |= nextStep;
                    } else {
                        apply(MAXIMUM, FF, nextStep, FF);
                    }
                }
            }
        } else {
            std::cout << "BFS image process: " << n << std::endl;
            // change for distance computing
            // if (isEmptyBot || isEmptyTop) {
            //     apply(POST_IMAGE, curr, relations[0], nextStep);
            //     FF |= nextStep;
            //     apply(POST_IMAGE, curr, relations[1], nextStep);
            //     FF |= nextStep;
            // } else {
            //     apply(POST_IMAGE, curr, relations[0], FF);
            // }
            apply(POST_IMAGE, curr, relations[0], FF);
        }

        distance_permutation.push_back(n);
        n++;
        if (isComputeDistance) {
            apply(MINIMUM, curr, FF, SS);
        } else {
            if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {
                if (n==1) SS = curr | FF;
                else SS = FF;
            } else {
                apply(MAXIMUM, curr, FF, SS);
            }
        }

        // apply(CARDINALITY, SS, card_SS);
        // apply(CARDINALITY, curr, card_curr);
        // std::cout << "BFS SS ";
        // SS.getEdge().print(std::cout);
        // std::cout << " card: " << card_SS << std::endl;
        // std::cout << "BFS curr ";
        // curr.getEdge().print(std::cout);
        // std::cout << " card: " << card_curr << std::endl;
        // std::cout << "BFS SS num: " << forest1->getNodeManUsed(SS) << std::endl;
        // std::cout << "BFS curr num: " << forest1->getNodeManUsed(curr) << std::endl;

        // check fixpoint
        if (SS == curr) break;
        curr = SS;
        if (isComputeDistanceBFS) distance.push_back(curr);
        // GC
        if (isComputeDistanceBFS) {
            for (size_t i=0; i<distance.size(); i++) {
                forest1->markNodes(distance[i]);
            }
            forest1->markSweep();
        } else {
            forest1->registerFunc(curr);
            std::cout << "forest1 mark nodes; num funcs: " << forest1->numFuncs() << std::endl;
            forest1->markAllFuncs();
            std::cout << "forest1 mark and sweep: " << std::endl;
            forest1->markSweep();
            forest1->deregisterFunc(curr);
        }

        FF.falseFunc();

        // check if time out
        watchBFS.note_time();
        if (watchBFS.get_last_seconds() >= time) {
            target = SS;
            return 0;
        }
    }
    target = SS;
    return 1;
}

bool SSG_chain(const Func& initial, const std::vector<Func>& relations, Func& target, const double time)
{
    Func curr = initial;
    Func nextStep(forest1);
    Func SS(forest1);
    SS.falseFunc();
    unsigned long n = 0;
    long card_SS = 0, card_curr = 0;

    while (true) {
        for (size_t i=0; i<relations.size(); i++) {
            std::cout << "chain image process: " << n << " : (" << i << "/" << relations.size() << ")" << std::endl;
            // chenge for distance computing
            apply(POST_IMAGE, curr, relations[i], nextStep);
            if (isComputeDistance) {
                apply(MINIMUM, curr, nextStep, curr);
            } else {
                if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {
                    curr |= nextStep;
                } else {
                    apply(MAXIMUM, curr, nextStep, curr);
                }
            }
        }
        n++;
        apply(CARDINALITY, SS, card_SS);
        apply(CARDINALITY, curr, card_curr);
        std::cout << "SS card: " << card_SS << std::endl;
        std::cout << "curr card: " << card_curr << std::endl;
        std::cout << "SS num: " << forest1->getNodeManUsed(SS) << std::endl;
        std::cout << "curr num: " << forest1->getNodeManUsed(curr) << std::endl;

        if (SS == curr) break;
        if (isComputeDistance) {
            apply(MINIMUM, curr, SS, SS);
        } else {
            if ((forest1->getSetting().getRangeType() == BOOLEAN) && (forest1->getSetting().getEncodeMechanism() == TERMINAL)) {
                SS |= curr;
            } else {
                apply(MAXIMUM, curr, SS, SS);
            }
        }
        curr = SS;
        // GC
        forest1->registerFunc(curr);
        forest1->registerFunc(SS);
        forest1->markAllFuncs();
        forest1->markSweep();
        forest1->deregisterFunc(curr);
        forest1->deregisterFunc(SS);
        forest2->markAllFuncs();
        forest2->markSweep();

        // check if time out
        watchChain.note_time();
        if (watchChain.get_last_seconds() >= time) {
            // result the explored so far
            target = SS;
            return 0;
        }
    }
    target = SS;
    return 1;
}

// the correct answer
bool SSG(bool isPrint, Func& target, const double time)
{
    // BDD
    Func states(forest1);
    states.falseFunc();
    // directions
    std::vector<std::vector<int> > directions(4, std::vector<int>(2, 0));
    directions[0][0] = -1; // up
    directions[1][0] = 1; // down
    directions[2][1] = -1; // left
    directions[3][1] = 1; // right
    // initial state
    PuzzleState initial(N, std::vector<uint16_t>(M, 0));
    for (uint16_t i=1; i<N*M; i++) {
        initial[(i%M==0)?i/M-1:i/M][(i%M==0)?M-1:i%M-1] = i;
    }
    if (isPrint) printPuzzleState(initial);
    // queue to move
    std::queue<PuzzleState> q;
    q.push(initial);

    // number of states
    long num = 1;
    while (!q.empty()) {
        PuzzleState current = q.front();
        q.pop();
        states |= encodePuzzle2BDD(current);

        // find the empty tile position
        int x = 0, y = 0;
        bool found = 0;
        for (size_t i = 0; i < current.size(); ++i) {
            for (size_t j = 0; j < current[i].size(); ++j) {
                if (current[i][j] == 0) {
                    x = i;
                    y = j;
                    found = 1;
                    break;
                }
            }
            if (found) break;
        }

        // try all possible moves
        for (size_t i=0; i<directions.size(); i++) {
            int newX = x + directions[i][0], newY = y + directions[i][1];
            if (newX >= 0 && newX < N && newY >= 0 && newY < M) {
                // Swap empty tile with adjacent tile
                PuzzleState next = current;
                SWAP(next[x][y], next[newX][newY]);
                // check if this state was visited
                Func currStates = states;
                currStates |= encodePuzzle2BDD(next);
                if (currStates.getEdge() != states.getEdge()) {
                    // new state
                    num++;
                    q.push(next);
                    if (isPrint) printPuzzleState(next);
                    // trace
                    // std::cout << "\tid: " << num << std::endl;
                    long c = 0;
                    apply(CARDINALITY, currStates, c);
                    // std::cout << "\t#states: " << c << std::endl;
                    if (num != c) {
                        DotMaker dot0(forest1, "currStates");
                        dot0.buildGraph(states);
                        dot0.runDot("pdf");
                        DotMaker dot1(forest1, "next");
                        dot1.buildGraph(encodePuzzle2BDD(next));
                        dot1.runDot("pdf");
                        DotMaker dot2(forest1, "States");
                        dot2.buildGraph(currStates);
                        dot2.runDot("pdf");
                        exit(0);
                    }
                    // update
                    states = currStates;
                }
            }
        }

        // GC
        forest1->registerFunc(states);
        forest1->markAllFuncs();
        forest1->markSweep();
        forest1->deregisterFunc(states);

        // check if time out
        watchBFS0.note_time();
        if (watchBFS0.get_last_seconds() >= time) {
            target = states;
            return 0;
        }
    }
    // long card = 0;
    // apply(CARDINALITY, states, card);
    // std::cout << "correct answer: ";
    // states.getEdge().print(std::cout);
    // std::cout << std::endl;
    // std::cout << "\tnumber of nodes: " << forest1->getNodeManUsed(states) << std::endl;
    // std::cout << "\tnumber of states: " << num << std::endl;
    // std::cout << "\tnumber of states(card): " << card << std::endl;

    // long card_test = 0;
    // num = N*M*bits;
    // std::vector<bool> assign(num+1, 0);
    // for (long i=0; i<(1<<num); i++) {
    //     for (uint16_t k=1; k<=num; k++) {
    //         assign[k] = i&(1<<(k-1));
    //     }
    //     int valInt;
    //     Value evalRes = states.evaluate(assign);
    //     evalRes.getValueTo(&valInt, INT);
    //     if (valInt == 1) card_test++;
    // }
    // std::cout << "number of states(card_test): " << card_test << std::endl;
    target = states;
    return 1;
}

// for test
bool hasConstantPosInf(const Edge& e)
{
    if (e.getNodeLevel() == 0) {
        if (e.isConstantPosInf()) {
            return true;
        } else {
            return false;
        }
    }
    Edge lc = forest1->cofact(e.getNodeLevel(), e, 0);
    Edge hc = forest1->cofact(e.getNodeLevel(), e, 1);
    bool ans = false;
    if (hasConstantPosInf(lc)) {ans = true;}
    if (!ans && hasConstantPosInf(hc)) {ans = true;}
    return ans;
}
bool hasConstantPosInfValue(const Edge& e)
{
    if (e.getNodeLevel() == 0) {
        if (e.isConstantPosInf() && (e.getValue() != Value(0))) {
            return true;
        } else {
            return false;
        }
    }
    Edge lc = forest1->cofact(e.getNodeLevel(), e, 0);
    Edge hc = forest1->cofact(e.getNodeLevel(), e, 1);
    bool ans = false;
    if (hasConstantPosInfValue(lc)) {ans = true;}
    if (!ans && hasConstantPosInfValue(hc)) {ans = true;}
    return ans;
}
bool hasBothChildPosInf(const Edge& e)
{
    if (e.getNodeLevel() == 0) {return false;}
    Edge lc = forest1->cofact(e.getNodeLevel(), e, 0);
    Edge hc = forest1->cofact(e.getNodeLevel(), e, 1);
    bool ans = false;
    if (lc.isConstantPosInf() && hc.isConstantPosInf()) {
        ans = true;
    } else {
        ans = hasBothChildPosInf(lc) || hasBothChildPosInf(hc);
    }
    return ans;
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
        // std::cout << "start distance: " << d << std::endl;
        converted = convertEdge(forest3->getSetting().getNumVars(), converted, distance_ex[d].getEdge(), d);
        std::cout << "done distance: " << d << std::endl;
    }
    ans.setEdge(converted);
    return ans;
}

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

void testConcretizationOR(Func& r)
{
    // test posinf
    if (hasConstantPosInf(r.getEdge())) {
        std::cout << "testConcretizationOR: result has PosInf\n";
        exit(1);
    }
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
// concretize a given edge with corresponding don't-care edge in forest1
Edge concretize_rst(const uint16_t level, const Edge& initial, const Edge& dc)
{
    // terminal case
    if (initial.getNodeLevel() == 0) {
        return initial;
    }
    // only for reduction rules
    // if (dc.isConstantOne()) {
    //     Edge ans(makeTerminal(0), Value(0));
    //     return ans;
    // }
    uint16_t lvl = (initial.getNodeLevel() >= dc.getNodeLevel()) ? initial.getNodeLevel() : dc.getNodeLevel();
    Edge lc = forest1->cofact(lvl, initial, 0);
    Edge hc = forest1->cofact(lvl, initial, 1);
    Edge lc_dc = forest1->cofact(lvl, dc, 0);
    Edge hc_dc = forest1->cofact(lvl, dc, 1);
    if (lc_dc.isConstantOne()) {
        return concretize_rst(lvl-1, hc, hc_dc);
    } else if (hc_dc.isConstantOne()) {
        return concretize_rst(lvl-1, lc, lc_dc);
    }
    // recursively compute
    std::vector<Edge> child(2);
    child[0] = concretize_rst(lvl-1, lc, lc_dc);
    child[1] = concretize_rst(lvl-1, hc, hc_dc);
    // reduce edge
    Edge result;
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    result = forest1->reduceEdge(level, root, lvl, child);
    return result;
}
Edge concretize_rst(const uint16_t level, const Edge& initial)
{
    // terminal case
    if ((initial.getNodeLevel() == 0)) {
        return initial;
    }
    uint16_t lvl = initial.getNodeLevel();
    Edge lc = forest1->cofact(lvl, initial, 0);
    Edge hc = forest1->cofact(lvl, initial, 1);
    if (lc.isConstantPosInf()) {
        return concretize_rst(lvl-1, hc);
    } else if (hc.isConstantPosInf()) {
        return concretize_rst(lvl-1, lc);
    }
    // recursively compute
    std::vector<Edge> child(2);
    child[0] = concretize_rst(lvl-1, lc);
    child[1] = concretize_rst(lvl-1, hc);
    // reduce edge
    Edge result;
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    result = forest1->reduceEdge(level, root, lvl, child);
    return result;
}
// for multi-root
Func concretizeFunc_rst(const Func& initial, const Func& dc)
{
    Func result(initial.getForest());
    // result.setEdge(concretize_rst(forest1->getSetting().getNumVars(), initial.getEdge(), dc.getEdge()));
    apply(CONCRETIZE_RST, initial, dc, result);
    return result;
}
// for non multi-root
Func concretizeFunc_rst(const Func& initial)
{
    Func result(initial.getForest());
    // result.setEdge(concretize_rst(forest1->getSetting().getNumVars(), initial.getEdge()));
    apply(CONCRETIZE_RST, initial, Value(SpecialValue::POS_INF), result);
    return result;
}

// main function to concretize using restrict
void concretize_rst()
{
    if (isComputeDistanceBFS) {
        watch_rst.reset();
        watch_rst.note_time();
        // for up-to
        distance_rst.push_back(concretizeFunc_rst(distance[distance_permutation[0]], distance_ex.back()));
        Func dc = distance_ex.back();
        for (size_t i=1; i<distance_permutation.size()-1; i++) {    // -1: remove the last one
            // update dc
            for (size_t k=0; k<i; k++) {
                dc |= distance[distance_permutation[k]];
            }
            // concretizing
            distance_rst.push_back(concretizeFunc_rst(distance[distance_permutation[i]], dc));
        }
        watch_rst.note_time();
        time_rst = watch_rst.get_last_seconds();
        nodes_dis_rst = forest1->getNodeManUsed(distance_rst);
        distance_permutation_opt = distance_permutation;

        // update permutation
        for (size_t i=0; i<distance_permutation.size(); i++) {
            distance_permutation[i] = i;
        }
        std::sort(distance_permutation.begin(), distance_permutation.end(),
                    [&](size_t i, size_t j) { return distribution_ex_node[i] < distribution_ex_node[j]; });

        watch_rst.reset();
        watch_rst.note_time();
        // for exact
        distance_ex_rst.push_back(concretizeFunc_rst(distance_ex[distance_permutation[0]], distance_ex.back()));
        dc = distance_ex.back();
        for (size_t i=1; i<distance_permutation.size()-1; i++) {    // -1: remove the last one
            // update dc
            for (size_t k=0; k<i; k++) {
                dc |= distance_ex[distance_permutation[k]];
            }
            // concretizing
            distance_ex_rst.push_back(concretizeFunc_rst(distance_ex[distance_permutation[i]], dc));
        }
        watch_rst.note_time();
        time_ex_rst = watch_rst.get_last_seconds();
        nodes_dis_ex_rst = forest1->getNodeManUsed(distance_ex_rst);
        distance_permutation_ex_opt = distance_permutation;
        std::cout << "done rst\n";
        // test
        if (isTestConcretize) testConcretizationMR(distance_ex_rst);
    } else {
        watch_rst.reset();
        watch_rst.note_time();
        // for non multi-root
        distance_one_root_rst = concretizeFunc_rst(distance_one_root);
        watch_rst.note_time();
        time_ex_rst = watch_rst.get_last_seconds();
        nodes_dis_ex_rst = forest1->getNodeManUsed(distance_one_root_rst);
        std::cout << "done rst\n";
        // test
        if (isTestConcretize) testConcretizationOR(distance_one_root_rst);
    }
}
// ===============================================================================================================================================================

// One-sided-match
// ===============================================================================================================================================================
unsigned char compare_osm(const Edge& e1, const Edge& e2, const Edge& dc1, const Edge& dc2)
{
    // terminal cases
    if (dc1.isConstantOne()) {return '>';}
    if (dc2.isConstantOne()) {return '<';}
    // if (e1 == e2) {return '=';}
    if ((e1 == e2) && (dc1.isConstantZero() && dc2.isConstantZero())) {return '=';}
    // if ((e1.getNodeLevel() == e2.getNodeLevel()) && (e1.getNodeLevel() == 0)) {return '!';}   // both different terminal value and not dc
    if ((e1.getNodeLevel() == e2.getNodeLevel()) && (e1.getNodeLevel() == 0) && (dc1.isConstantZero() && dc2.isConstantZero())) {return '!';}   // both different terminal value and not dc

    // the highest level
    uint16_t highest = e1.getNodeLevel();
    if (e2.getNodeLevel() > highest) {highest = e2.getNodeLevel();}
    if (dc1.getNodeLevel() > highest) {highest = dc1.getNodeLevel();}
    if (dc2.getNodeLevel() > highest) {highest = dc2.getNodeLevel();}

    // recursively compare
    unsigned char ans, comp0, comp1;
    comp0 = compare_osm(forest1->cofact(highest, e1, 0),
                        forest1->cofact(highest, e2, 0),
                        forest1->cofact(highest, dc1, 0),
                        forest1->cofact(highest, dc2, 0));
    comp1 = compare_osm(forest1->cofact(highest, e1, 1),
                        forest1->cofact(highest, e2, 1),
                        forest1->cofact(highest, dc1, 1),
                        forest1->cofact(highest, dc2, 1));
    
    if ((comp0 == '=') || (comp0 == comp1)) {
        ans = comp1;
    } else if (comp1 == '=') {
        ans = comp0;
    } else {
        ans = '!';
    }
    return ans;
}
unsigned char compare_osm(const Edge& e1, const Edge& e2)
{
    // terminal cases
    if (e1.isConstantPosInf()) {return '>';}
    if (e2.isConstantPosInf()) {return '<';}
    if (e1 == e2) {return '=';}
    if ((e1.getNodeLevel() == e2.getNodeLevel()) && (e1.getNodeLevel() == 0)) {return '!';}   // both different terminal value and not dc

    // the highest level
    uint16_t highest = e1.getNodeLevel();
    if (e2.getNodeLevel() > highest) {highest = e2.getNodeLevel();}

    // recursively compare
    unsigned char ans, comp0, comp1;
    comp0 = compare_osm(forest1->cofact(highest, e1, 0),
                        forest1->cofact(highest, e2, 0));
    comp1 = compare_osm(forest1->cofact(highest, e1, 1),
                        forest1->cofact(highest, e2, 1));
    
    if ((comp0 == '=') || (comp0 == comp1)) {
        ans = comp1;
    } else if (comp1 == '=') {
        ans = comp0;
    } else {
        ans = '!';
    }
    return ans;
}
Edge concretize_osm(const uint16_t level, const Edge& initial, const Edge& dc)
{
    // terminal case
    if (initial.getNodeLevel() == 0) {
        return initial;
    }
    uint16_t lvl = MAX(initial.getNodeLevel(), dc.getNodeLevel());
    Edge lc = forest1->cofact(lvl, initial, 0);
    Edge hc = forest1->cofact(lvl, initial, 1);
    Edge lc_dc = forest1->cofact(lvl, dc, 0);
    Edge hc_dc = forest1->cofact(lvl, dc, 1);
    if (lc_dc.isConstantOne()) {
        return concretize_osm(lvl-1, hc, hc_dc);
    } else if (hc_dc.isConstantOne()) {
        return concretize_osm(lvl-1, lc, lc_dc);
    }
    // compare child edges
    unsigned comp = compare_osm(lc, hc, lc_dc, hc_dc);
    if (comp == '<') {
        return concretize_osm(lvl-1, lc, lc_dc);
    }
    if (comp == '>') {
        return concretize_osm(lvl-1, hc, hc_dc);
    }
    // new node
    std::vector<Edge> child(2);
    child[0] = concretize_osm(lvl-1, lc, lc_dc);
    child[1] = concretize_osm(lvl-1, hc, hc_dc);
    // reduce edge
    Edge result;
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    result = forest1->reduceEdge(level, root, lvl, child);
    return result;
}
Edge concretize_osm(const uint16_t level, const Edge& initial)
{
    // terminal case
    if ((initial.getNodeLevel() == 0)) {
        return initial;
    }
    uint16_t lvl = initial.getNodeLevel();
    Edge lc = forest1->cofact(lvl, initial, 0);
    Edge hc = forest1->cofact(lvl, initial, 1);
    if (lc.isConstantPosInf()) {
        return concretize_osm(lvl-1, hc);
    } else if (hc.isConstantPosInf()) {
        return concretize_osm(lvl-1, lc);
    }
    // compare child edges
    unsigned comp = compare_osm(lc, hc);
    if (comp == '<') {
        return concretize_osm(lvl-1, lc);
    }
    if (comp == '>') {
        return concretize_osm(lvl-1, hc);
    }
    // new node
    std::vector<Edge> child(2);
    child[0] = concretize_osm(lvl-1, lc);
    child[1] = concretize_osm(lvl-1, hc);
    // reduce edge
    Edge result;
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    result = forest1->reduceEdge(level, root, lvl, child);
    return result;
}
Func concretizeFunc_osm(const Func& initial, const Func& dc)
{
    Func result(initial.getForest());
    // result.setEdge(concretize_osm(forest1->getSetting().getNumVars(), initial.getEdge(), dc.getEdge()));
    apply(CONCRETIZE_OSM, initial, dc, result);
    return result;
}
Func concretizeFunc_osm(const Func& initial)
{
    Func result(initial.getForest());
    // result.setEdge(concretize_osm(forest1->getSetting().getNumVars(), initial.getEdge()));
    apply(CONCRETIZE_OSM, initial, Value(SpecialValue::POS_INF), result);
    return result;
}

// main function to concretize using one-sided-match
void concretize_osm()
{
    if (isComputeDistanceBFS) {
        // initial permutation
        for (size_t i=0; i<distance_permutation.size(); i++) {
            distance_permutation[i] = i;
        }
        watch_osm.reset();
        watch_osm.note_time();
        // for up-to
        distance_osm.push_back(concretizeFunc_osm(distance[distance_permutation[0]], distance_ex.back()));
        Func dc = distance_ex.back();
        for (size_t i=1; i<distance_permutation.size()-1; i++) {    // -1: remove the last one
            // update dc
            for (size_t k=0; k<i; k++) {
                dc |= distance[distance_permutation[k]];
            }
            // concretizing
            distance_osm.push_back(concretizeFunc_osm(distance[distance_permutation[i]], dc));
        }
        watch_osm.note_time();
        time_osm = watch_osm.get_last_seconds();
        nodes_dis_osm = forest1->getNodeManUsed(distance_osm);
        distance_permutation_opt = distance_permutation;

        // update permutation
        for (size_t i=0; i<distance_permutation.size(); i++) {
            distance_permutation[i] = i;
        }
        std::sort(distance_permutation.begin(), distance_permutation.end(),
                    [&](size_t i, size_t j) { return distribution_ex_node[i] < distribution_ex_node[j]; });

        watch_osm.reset();
        watch_osm.note_time();
        // for exact
        distance_ex_osm.push_back(concretizeFunc_osm(distance_ex[distance_permutation[0]], distance_ex.back()));
        dc = distance_ex.back();
        for (size_t i=1; i<distance_permutation.size()-1; i++) {    // -1: remove the last one
            // update dc
            for (size_t k=0; k<i; k++) {
                dc |= distance_ex[distance_permutation[k]];
            }
            // concretizing
            distance_ex_osm.push_back(concretizeFunc_osm(distance_ex[distance_permutation[i]], dc));
        }
        watch_osm.note_time();
        time_ex_osm = watch_osm.get_last_seconds();
        nodes_dis_ex_osm = forest1->getNodeManUsed(distance_ex_osm);
        distance_permutation_ex_opt = distance_permutation;
        std::cout << "done osm\n";
        // test
        if (isTestConcretize) testConcretizationMR(distance_ex_osm);
    } else {
        watch_osm.reset();
        watch_osm.note_time();
        // for non multi-root
        distance_one_root_osm = concretizeFunc_osm(distance_one_root);
        watch_osm.note_time();
        time_ex_osm = watch_osm.get_last_seconds();
        nodes_dis_ex_osm = forest1->getNodeManUsed(distance_one_root_osm);
        std::cout << "done osm\n";
        // test
        if (isTestConcretize) testConcretizationOR(distance_one_root_osm);
    }
}
// ===============================================================================================================================================================

// Two-sided-match
// ===============================================================================================================================================================
bool has_common_tsm(const Edge& e1, const Edge& e2, const Edge& dc1, const Edge& dc2)
{
    // terminal cases
    if (dc1.isConstantOne()) {return true;}
    if (dc2.isConstantOne()) {return true;}
    if (e1 == e2) {return true;}
    if ((e1.getNodeLevel() == e2.getNodeLevel()) && (e1.getNodeLevel() == 0)) {return false;}   // both different terminal value and not dc

    // the highest level
    uint16_t highest = e1.getNodeLevel();
    if (e2.getNodeLevel() > highest) {highest = e2.getNodeLevel();}
    if (dc1.getNodeLevel() > highest) {highest = dc1.getNodeLevel();}
    if (dc2.getNodeLevel() > highest) {highest = dc2.getNodeLevel();}

    // recursively check
    bool cmp0, cmp1;
    cmp0 = has_common_tsm(forest1->cofact(highest, e1, 0),
                            forest1->cofact(highest, e2, 0),
                            forest1->cofact(highest, dc1, 0),
                            forest1->cofact(highest, dc2, 0));
    cmp1 = has_common_tsm(forest1->cofact(highest, e1, 1),
                            forest1->cofact(highest, e2, 1),
                            forest1->cofact(highest, dc1, 1),
                            forest1->cofact(highest, dc2, 1));
    return (cmp0 && cmp1);
}
bool has_common_tsm(const Edge& e1, const Edge& e2)
{
    // terminal cases
    if (e1.isConstantPosInf()) {return true;}
    if (e2.isConstantPosInf()) {return true;}
    if (e1 == e2) {return true;}
    if ((e1.getNodeLevel() == e2.getNodeLevel()) && (e1.getNodeLevel() == 0)) {return false;}   // both different terminal value and not dc

    // the highest level
    uint16_t highest = e1.getNodeLevel();
    if (e2.getNodeLevel() > highest) {highest = e2.getNodeLevel();}

    // recursively check
    bool cmp0, cmp1;
    cmp0 = has_common_tsm(forest1->cofact(highest, e1, 0),
                            forest1->cofact(highest, e2, 0));
    cmp1 = has_common_tsm(forest1->cofact(highest, e1, 1),
                            forest1->cofact(highest, e2, 1));
    return (cmp0 && cmp1);
}
Edge common_tsm(const uint16_t level, const Edge& e1, const Edge& e2, const Edge& dc1, const Edge& dc2)
{
    // terminal cases
    if (dc1.isConstantOne()) {return e2;}
    if (dc2.isConstantOne()) {return e1;}
    if (e1 == e2) {return e1;}

    // the highest level
    uint16_t highest = e1.getNodeLevel();
    if (e2.getNodeLevel() > highest) {highest = e2.getNodeLevel();}
    if (dc1.getNodeLevel() > highest) {highest = dc1.getNodeLevel();}
    if (dc2.getNodeLevel() > highest) {highest = dc2.getNodeLevel();}

    std::vector<Edge> child(2);
    child[0] = common_tsm(highest - 1,
                        forest1->cofact(highest, e1, 0),
                        forest1->cofact(highest, e2, 0),
                        forest1->cofact(highest, dc1, 0),
                        forest1->cofact(highest, dc2, 0));
    child[1] = common_tsm(highest - 1,
                        forest1->cofact(highest, e1, 1),
                        forest1->cofact(highest, e2, 1),
                        forest1->cofact(highest, dc1, 1),
                        forest1->cofact(highest, dc2, 1));
    // reduce edge
    Edge result;
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    result = forest1->reduceEdge(level, root, highest, child);
    return result;
}
Edge common_tsm(const uint16_t level, const Edge& e1, const Edge& e2)
{
    // terminal cases
    if (e1.isConstantPosInf()) {return e2;}
    if (e2.isConstantPosInf()) {return e1;}
    if (e1 == e2) {return e1;}

    // the highest level
    uint16_t highest = e1.getNodeLevel();
    if (e2.getNodeLevel() > highest) {highest = e2.getNodeLevel();}

    std::vector<Edge> child(2);
    child[0] = common_tsm(highest - 1,
                        forest1->cofact(highest, e1, 0),
                        forest1->cofact(highest, e2, 0));
    child[1] = common_tsm(highest - 1,
                        forest1->cofact(highest, e1, 1),
                        forest1->cofact(highest, e2, 1));
    // reduce edge
    Edge result;
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    result = forest1->reduceEdge(level, root, highest, child);
    return result;
}
Edge concretize_tsm(const uint16_t level, const Edge& initial, const Edge& dc)
{
    // terminal case
    if (initial.getNodeLevel() == 0) {
        return initial;
    }
    uint16_t lvl = (initial.getNodeLevel() >= dc.getNodeLevel()) ? initial.getNodeLevel() : dc.getNodeLevel();
    Edge lc = forest1->cofact(lvl, initial, 0);
    Edge hc = forest1->cofact(lvl, initial, 1);
    Edge lc_dc = forest1->cofact(lvl, dc, 0);
    Edge hc_dc = forest1->cofact(lvl, dc, 1);
    if (lc_dc.isConstantOne()) {
        return concretize_tsm(level, hc, hc_dc);
    } else if (hc_dc.isConstantOne()) {
        return concretize_tsm(level, lc, lc_dc);
    }
    // check child edges
    if (has_common_tsm(lc, hc, lc_dc, hc_dc)) {
        Edge child_common = common_tsm(lvl - 1, lc, hc, lc_dc, hc_dc);
        if ((lvl > initial.getNodeLevel())) {
            // new dc is lc_dc & hc_dc !!!
            Func flcdc(forest1, lc_dc), fhcdc(forest1, hc_dc);
            flcdc &= fhcdc;
            return concretize_tsm(lvl-1, child_common, flcdc.getEdge());
        }
        return concretize_tsm(level, child_common, dc);
    }
    // new node
    std::vector<Edge> child(2);
    child[0] = concretize_tsm(lvl-1, lc, lc_dc);
    child[1] = concretize_tsm(lvl-1, hc, hc_dc);
    // reduce edge
    Edge result;
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    result = forest1->reduceEdge(level, root, lvl, child);
    return result;
}
Edge concretize_tsm(const uint16_t level, const Edge& initial)
{
    // terminal case
    if (initial.getNodeLevel() == 0) {
        return initial;
    }
    uint16_t lvl = initial.getNodeLevel();
    Edge lc = forest1->cofact(lvl, initial, 0);
    Edge hc = forest1->cofact(lvl, initial, 1);
    if (lc.isConstantPosInf()) {
        return concretize_tsm(level, hc);
    } else if (hc.isConstantPosInf()) {
        return concretize_tsm(level, lc);
    }
    // check child edges
    if (has_common_tsm(lc, hc)) {
        Edge child_common = common_tsm(lvl - 1, lc, hc);
        return concretize_tsm(level, child_common);
    }
    // new node
    std::vector<Edge> child(2);
    child[0] = concretize_tsm(lvl-1, lc);
    child[1] = concretize_tsm(lvl-1, hc);
    // reduce edge
    Edge result;
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    result = forest1->reduceEdge(level, root, lvl, child);
    return result;
}
Func concretizeFunc_tsm(const Func& initial, const Func& dc)
{
    Func result(initial.getForest());
    // result.setEdge(concretize_tsm(forest1->getSetting().getNumVars(), initial.getEdge(), dc.getEdge()));
    apply(CONCRETIZE_TSM, initial, dc, result);
    return result;
}
Func concretizeFunc_tsm(const Func& initial)
{
    Func result(initial.getForest());
    // result.setEdge(concretize_tsm(forest1->getSetting().getNumVars(), initial.getEdge()));
    apply(CONCRETIZE_TSM, initial, Value(SpecialValue::POS_INF), result);
    return result;
}

// main function to concretize using two-sided-match
void concretize_tsm()
{
    if (isComputeDistanceBFS) {
        // initial permutation
        for (size_t i=0; i<distance_permutation.size(); i++) {
            distance_permutation[i] = i;
        }
        watch_tsm.reset();
        watch_tsm.note_time();
        // for up-to
        distance_tsm.push_back(concretizeFunc_tsm(distance[distance_permutation[0]], distance_ex.back()));
        Func dc = distance_ex.back();
        for (size_t i=1; i<distance_permutation.size()-1; i++) {    // -1: remove the last one
            // update dc
            for (size_t k=0; k<i; k++) {
                dc |= distance[distance_permutation[k]];
            }
            // concretizing
            distance_tsm.push_back(concretizeFunc_tsm(distance[distance_permutation[i]], dc));
        }
        watch_tsm.note_time();
        time_tsm = watch_tsm.get_last_seconds();
        nodes_dis_tsm = forest1->getNodeManUsed(distance_tsm);
        distance_permutation_opt = distance_permutation;

        // update permutation
        for (size_t i=0; i<distance_permutation.size(); i++) {
            distance_permutation[i] = i;
        }
        std::sort(distance_permutation.begin(), distance_permutation.end(),
                    [&](size_t i, size_t j) { return distribution_ex_node[i] < distribution_ex_node[j]; });

        watch_tsm.reset();
        watch_tsm.note_time();
        // for exact
        distance_ex_tsm.push_back(concretizeFunc_tsm(distance_ex[distance_permutation[0]], distance_ex.back()));
        dc = distance_ex.back();
        for (size_t i=1; i<distance_permutation.size()-1; i++) {    // -1: remove the last one
            // update dc
            for (size_t k=0; k<i; k++) {
                dc |= distance_ex[distance_permutation[k]];
            }
            // concretizing
            distance_ex_tsm.push_back(concretizeFunc_tsm(distance_ex[distance_permutation[i]], dc));
        }
        watch_tsm.note_time();
        time_ex_tsm = watch_tsm.get_last_seconds();
        nodes_dis_ex_tsm = forest1->getNodeManUsed(distance_ex_tsm);
        distance_permutation_ex_opt = distance_permutation;
        std::cout << "done tsm\n";
        // test
        if (isTestConcretize) testConcretizationMR(distance_ex_tsm);
    } else {
        watch_tsm.reset();
        watch_tsm.note_time();
        // for non multi-root
        distance_one_root_tsm = concretizeFunc_tsm(distance_one_root);
        watch_tsm.note_time();
        time_ex_tsm = watch_tsm.get_last_seconds();
        nodes_dis_ex_tsm = forest1->getNodeManUsed(distance_one_root_tsm);
        std::cout << "done tsm\n";
        // test
        if (isTestConcretize) testConcretizationOR(distance_one_root_tsm);
    }
}
// ===============================================================================================================================================================

bool processArgs(int argc, const char** argv)
{
    // default types
    setType = "RexBDD";
    // relType = "MTBMxD";
    relType = "FBMXD";
    bool setN = 0, setM = 0;
    for (int i=1; i<argc; i++) {
        if ('-' == argv[i][0]) {
            // options other than size
            if (strcmp("-st", argv[i])==0) {
                setType = argv[i+1];
                i++;
                continue;
            }
            if (strcmp("-rt", argv[i])==0) {
                relType = argv[i+1];
                i++;
                continue;
            }
            if (strcmp("-ct", argv[i])==0) {
                newType = argv[i+1];
                isConvert = 1;
                i++;
                continue;
            }
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
            if (strcmp("-d", argv[i])==0) {
                isComputeDistance = 1;
                continue;
            }
            if (strcmp("-mr", argv[i])==0) {
                isComputeDistanceBFS = 1;
                continue;
            }
            if (strcmp("-ru", argv[i])==0) {
                isRelationUnion = 1;
                continue;
            }
            if (strcmp("-et", argv[i])==0) {
                isEmptyTop = 1;
                continue;
            }
            if (strcmp("-eb", argv[i])==0) {
                isEmptyBot = 1;
                continue;
            }
            if (strcmp("-cz", argv[i])==0) {
                isConcretize = 1;
                continue;
            }
            if (strcmp("-to", argv[i])==0) {
                isTestConcretize = 0;
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
    std::cout << "\t[option]  -st: followed with the predefined type name of set BDD (dimention 1)" << std::endl;
    std::cout << "\t[option]  -rt: followed with the predefined type name of relation BDD (dimention 2)" << std::endl;
    std::cout << "\t[option]  -t: followed with a run time threshold in seconds (default: 1800)" << std::endl;
    std::cout << "\t[option]  -tg: switch to turn ON the run time threshold for every methods" << std::endl;
    std::cout << "\t[option]  -f: switch to turn ON the result report to a file" << std::endl;
    std::cout << "\t[option]  -d: switch to turn ON the computing of distance" << std::endl;
    return 1;
}

void report(std::ostream& out)
{
    int align = 36;
    if (isComputeDistanceBFS) {
        out << "=========================| Node |=========================" << std::endl;
        if (isRelationUnion) {
            out << std::left << std::setw(align/2) << "Relations" << std::setw(align/3) << "(Union):" << distribution_rel_node[0] << std::endl;
        } else {
            for (size_t i=0; i<distribution_rel_node.size(); i++) {
                out << std::left << std::setw(15) << "Relations " << std::setw(10) << i << distribution_rel_node[i] << std::endl;
            }
        }
        if (isConvert) {
            out << "----------------------------------------------------------" << std::endl;
            out << std::left << std::setw(align/1.2) << "convert_distance (final)" << std::setw(align/3) << "(exact):" << nodes_convert_dis_ex << std::endl;
            out << std::left << std::setw(align/1.2) << "*Restrict" << std::setw(align/3) << "(exact):" << nodes_convert_dis_ex_rst << std::endl;
            out << std::left << std::setw(align/1.2) << "*One-sided-match" << std::setw(align/3) << "(exact):" << nodes_convert_dis_ex_osm << std::endl;
            out << std::left << std::setw(align/1.2) << "*Two-sided-match" << std::setw(align/3) << "(exact):" << nodes_convert_dis_ex_tsm << std::endl;
        } else {
            out << "----------------------------------------------------------" << std::endl;
            out << std::left << std::setw(align/1.2) << "BFS_distance (final)" << std::setw(align/3) << "(up-to):" << nodes_dis_BFS << std::endl;
            out << std::left << std::setw(align/1.2) << "BFS_distance (peak)" << std::setw(align/3) << "(up-to):" << nodes_dis_BFS_peak << std::endl;
            out << std::left << std::setw(align/1.2) << "*Restrict" << std::setw(align/3) << "(up-to):" << nodes_dis_rst << std::endl;
            out << std::left << std::setw(align/1.2) << "*One-sided-match" << std::setw(align/3) << "(up-to):" << nodes_dis_osm << std::endl;
            out << std::left << std::setw(align/1.2) << "*Two-sided-match" << std::setw(align/3) << "(up-to):" << nodes_dis_tsm << std::endl;
            out << "----------------------------------------------------------" << std::endl;
            out << std::left << std::setw(align/1.2) << "BFS_distance (final)" << std::setw(align/3) << "(exact):" << nodes_dis_ex_BFS << std::endl;
            out << std::left << std::setw(align/1.2) << "BFS_distance (peak)" << std::setw(align/3) << "(exact):" << nodes_dis_ex_BFS_peak << std::endl;
            out << std::left << std::setw(align/1.2) << "*Restrict" << std::setw(align/3) << "(exact):" << nodes_dis_ex_rst << std::endl;
            out << std::left << std::setw(align/1.2) << "*One-sided-match" << std::setw(align/3) << "(exact):" << nodes_dis_ex_osm << std::endl;
            out << std::left << std::setw(align/1.2) << "*Two-sided-match" << std::setw(align/3) << "(exact):" << nodes_dis_ex_tsm << std::endl;
            if (isConcretize) {
                for (size_t i=0; i<distance_permutation_ex_opt.size()-1; i++) {
                    out << distance_permutation_ex_opt[i] << " ";
                }
                out << std::endl;
            }
        }

        out << "====================| Distribution |======================" << std::endl;
        out << std::left << std::setw(15) << "Up-to" << std::left << std::setw(15) << "#state" << std::left << std::setw(15) << "#node" << std::endl;
        for (size_t i=0; i<distance.size(); i++) {
            out << std::left << std::setw(15) << i 
            << std::left << std::setw(15) << distribution_state[i] 
            << std::left << std::setw(15) << distribution_node[i] << std::endl;
        }
        out << "----------------------------------------------------------" << std::endl;
        out << std::left << std::setw(15) << "Exact" << std::left << std::setw(15) << "#state" << std::left << std::setw(15) << "#node" << std::endl;
        for (size_t i=0; i<distance.size(); i++) {
            out << std::left << std::setw(15) << i 
            << std::left << std::setw(15) << distribution_ex_state[i] 
            << std::left << std::setw(15) << distribution_ex_node[i] << std::endl;
        }
        out << "----------------------------------------------------------" << std::endl;
        out << std::left << std::setw(15) << "Don't-care"
            // << std::left << std::setw(15) << distribution_ex_state.back()
            << std::left << std::setw(15) << ((0x01UL<<forest1->getSetting().getNumVars()) - distribution_state.back())
            << std::left << std::setw(15) << distribution_ex_node.back() << std::endl;
        out << "=========================| Time |=========================" << std::endl;
        out << std::left << std::setw(align) << "BFS_distance (up-to):" << time_BFS << std::endl;
        out << std::left << std::setw(align) << "BFS_distance (exact):" << time_BFS_ex << std::endl;
        out << "----------------------------------------------------------" << std::endl;
        if (isConvert) {
            out << std::left << std::setw(align/1.2) << "*Restrict" << std::setw(align/3) << "(exact):" << time_convert_rst << std::endl;
            out << std::left << std::setw(align/1.2) << "*One-sided-match" << std::setw(align/3) << "(exact):" << time_convert_osm << std::endl;
            out << std::left << std::setw(align/1.2) << "*Two-sided-match" << std::setw(align/3) << "(exact):" << time_convert_tsm << std::endl;
        } else {
            out << std::left << std::setw(align/1.2) << "*Restrict" << std::setw(align/3) << "(up-to):" << time_rst << std::endl;
            out << std::left << std::setw(align/1.2) << "*One-sided-match" << std::setw(align/3) << "(up-to):" << time_osm << std::endl;
            out << std::left << std::setw(align/1.2) << "*Two-sided-match" << std::setw(align/3) << "(up-to):" << time_tsm << std::endl;
            out << "----------------------------------------------------------" << std::endl;
            out << std::left << std::setw(align/1.2) << "*Restrict" << std::setw(align/3) << "(exact):" << time_ex_rst << std::endl;
            out << std::left << std::setw(align/1.2) << "*One-sided-match" << std::setw(align/3) << "(exact):" << time_ex_osm << std::endl;
            out << std::left << std::setw(align/1.2) << "*Two-sided-match" << std::setw(align/3) << "(exact):" << time_ex_tsm << std::endl;
        }
    } else {
        /* report time */
        out << "=========================| Time |=========================" << std::endl;
        out << std::left << std::setw(align);
        if (getRes_Sat) {
            out << "Saturation:" << time_Sat << " seconds" << std::endl;
        } else {
            out << "Saturation:" << "TIME OUT" << std::endl;
        }
        out << std::left << std::setw(align);
        if (getRes_chain) {
            out << "chain:" << time_chain << " seconds" << std::endl;
        } else {
            out << "chain:" << "TIME OUT" << std::endl;
        }
        out << std::left << std::setw(align);
        if (getRes_BFS) {
            out << "BFS:" << time_BFS << " seconds" << std::endl;
        } else {
            out << "BFS:" << "TIME OUT" << std::endl;
        }
        out << std::left << std::setw(30);
        out << "" << "[TIME OUT: " << newTimeLimit << " seconds]" << std::endl;
        if (isConcretize) {
            out << "----------------------------------------------------------" << std::endl;
            out << std::left << std::setw(align/1.2) << "*Restrict" << std::setw(align/3) << "(exact):" << time_ex_rst << std::endl;
            out << std::left << std::setw(align/1.2) << "*One-sided-match" << std::setw(align/3) << "(exact):" << time_ex_osm << std::endl;
            out << std::left << std::setw(align/1.2) << "*Two-sided-match" << std::setw(align/3) << "(exact):" << time_ex_tsm << std::endl;
        }

        /* report expored #states */
        out << "=========================| #States |======================" << std::endl;
        out << std::left << std::setw(align) << "Saturation:" << state_Sat << std::endl;
        out << std::left << std::setw(align) << "chain:" << state_chain << std::endl;
        out << std::left << std::setw(align) << "BFS:" << state_BFS << std::endl;
        // out << std::left << std::setw(align) << "BFS (w/o image):" << state_correct << std::endl;
        
        /* report nodes */
        out << "=========================| Node |=========================" << std::endl;
        out << std::left << std::setw(align) << "Saturation (final):" << nodes_Sat << std::endl;
        out << std::left << std::setw(align) << "Saturation (peak):" << nodes_Sat_Peak << std::endl;
        out << std::left << std::setw(align) << "chain (final):" << nodes_chain << std::endl;
        out << std::left << std::setw(align) << "chain (peak):" << nodes_chain_Peak << std::endl;
        out << std::left << std::setw(align) << "BFS (final):" << nodes_BFS << std::endl;
        out << std::left << std::setw(align) << "BFS (peak):" << nodes_BFS_Peak << std::endl;
        // out << std::left << std::setw(align) << "BFS (w/o image):" << nodes_correct << std::endl;
        out << std::left << std::setw(align) << "Relations:" << nodes_rel << std::endl;
        out << "----------------------------------------------------------" << std::endl;
        out << std::left << std::setw(align/2) << "*Restrict" << std::setw(align/3) << "(exact):" << nodes_dis_ex_rst << std::endl;
        out << std::left << std::setw(align/2) << "*One-sided-match" << std::setw(align/3) << "(exact):" << nodes_dis_ex_osm << std::endl;
        out << std::left << std::setw(align/2) << "*Two-sided-match" << std::setw(align/3) << "(exact):" << nodes_dis_ex_tsm << std::endl;
    }

    /* the end */
    out << "**********************************************************" << std::endl;
}

void compute_saturation(const Func& target, const std::vector<Func>& relations)
{
    Func states_Sat(forest1);
    // Timer start
    watchSat.reset();
    watchSat.note_time();
    apply(SATURATE, target, relations, states_Sat);
    watchSat.note_time();
    getRes_Sat = 1;
    // save for concretization
    if (hasBothChildPosInf(states_Sat.getEdge())) {
        std::cout << "compute_saturation: states_Sat has both child PosInf\n";
        delete forest1;
        delete forest2;
        exit(1);
    }
    if (hasConstantPosInfValue(states_Sat.getEdge())) {
        std::cout << "compute_saturation: states_Sat has PosInf edge with >0 value\n";
        delete forest1;
        delete forest2;
        exit(1);
    }
    distance_one_root = states_Sat;
    // report cache
    UOPs.reportCacheStat(std::cout);
    BOPs.reportCacheStat(std::cout);
    SOPs.reportCacheStat(std::cout);
    // record time
    time_Sat = watchSat.get_last_seconds();
    // record #states
    // if (!isComputeDistance) apply(CARDINALITY, states_Sat, state_Sat);
    apply(CARDINALITY, states_Sat, state_Sat);
    // record #nodes
    nodes_Sat = forest1->getNodeManUsed(states_Sat);
    nodes_Sat_Peak = forest1->getNodeManPeak();

    forest1->registerFunc(states_Sat);
    // report
    int align0 = 30;
    std::cout << "=========================| Time |=========================" << std::endl;
    std::cout << std::left << std::setw(align0);
    std::cout << "Saturation:" << time_Sat << " seconds" << std::endl;
    std::cout << "=========================| #States |======================" << std::endl;
    std::cout << std::left << std::setw(align0) << "Saturation:" << state_Sat << std::endl;
    std::cout << "=========================| Node |=========================" << std::endl;
    std::cout << std::left << std::setw(align0) << "Saturation (final):" << nodes_Sat << std::endl;
    std::cout << std::left << std::setw(align0) << "Saturation (peak):" << nodes_Sat_Peak << std::endl;

    if (N==2 && M==2){
        DotMaker dot2(forest1, "distance_sat");
        dot2.buildGraph(states_Sat);
        dot2.runDot("pdf");
    }
    // reset the forest
    if (!isComputeDistance) {
        forest1->deregisterFunc();
        forest1->registerFunc(target);
        forest1->markAllFuncs();
        forest1->markSweep();
    }

    // STOP here if distance
    // if (isComputeDistance) {
    //     // report result
    //     if (!isReportToFile) {
    //         std::cout << "Done!" << std::endl;
    //         std::cout << "**********************************************************" << std::endl;
    //         int align = 30;
    //         std::cout << std::left << std::setw(align) << "Sliding puzzle:" << N << " x " << M  << std::endl;
    //         std::cout << std::left << std::setw(align) << "Bits per position: " << bits << std::endl;
    //         std::cout << std::left << std::setw(align) << "Level:" << forest1->getSetting().getNumVars() << std::endl;
    //         std::cout << std::left << std::setw(align) << "The [Set] type:" << forest1->getSetting().getName() << std::endl;
    //         std::cout << std::left << std::setw(align) << "The [Relation] type:" << forest2->getSetting().getName() << std::endl;
    //         report(std::cout);
    //     } else {
    //         std::cout << "**********************************************************" << std::endl;
    //         std::string fileName = forest1->getSetting().getName();
    //         fileName += "_";
    //         fileName += forest2->getSetting().getName();
    //         fileName += "_";
    //         fileName += std::to_string(N);
    //         fileName += "_";
    //         fileName += std::to_string(M);
    //         fileName += ".txt";
    //         std::ofstream file(fileName, std::ios::app);
    //         if (!file) {
    //             std::cerr << "Failed to open file " << fileName << std::endl;
    //         } else {
    //             report(file);
    //             file.close();
    //         }
    //         std::cout << "Done!" << std::endl;
    //     }
    //     delete forest1;
    //     delete forest2;
    //     return;
    // }

    /* Update time limit */
    if (!isTimeLimitGlobal) newTimeLimit = time_Sat;
}

void compute_chain(const Func& target, const std::vector<Func>& relations)
{
    Func states_chain(forest1);
    // Timer start
    watchChain.reset();
    watchChain.note_time();
    getRes_chain = SSG_chain(target, relations, states_chain, newTimeLimit);
    watchChain.note_time();
    // report cache
    UOPs.reportCacheStat(std::cout);
    BOPs.reportCacheStat(std::cout);
    // record time
    time_chain = watchChain.get_last_seconds();
    // record #states
    apply(CARDINALITY, states_chain, state_chain);
    // record #nodes
    nodes_chain = forest1->getNodeManUsed(states_chain);
    nodes_chain_Peak = forest1->getNodeManPeak();

    forest1->registerFunc(states_chain);
    // reset the forest
    forest1->deregisterFunc();
    forest1->registerFunc(target);
    forest1->markAllFuncs();
    forest1->markSweep();

    /* Update time limit */
    // if (!isTimeLimitGlobal) newTimeLimit = time_chain;
}

void compute_BFS(const Func& target, const std::vector<Func>& relations)
{
    Func states_BFS(forest1);
    // Timer start
    watchBFS.reset();
    watchBFS.note_time();
    getRes_BFS = SSG_BFS(target, relations, states_BFS, newTimeLimit);
    watchBFS.note_time();
    // report cache
    UOPs.reportCacheStat(std::cout);
    BOPs.reportCacheStat(std::cout);
    // record time
    time_BFS = watchBFS.get_last_seconds();
    // record #states
    apply(CARDINALITY, states_BFS, state_BFS);
    // record #nodes
    nodes_BFS = forest1->getNodeManUsed(states_BFS);
    nodes_BFS_Peak = forest1->getNodeManPeak();

    forest1->registerFunc(states_BFS);

    // compute multi-root distance
    if (isComputeDistanceBFS) {
        // get number of final nodes
        nodes_dis_BFS = forest1->getNodeManUsed(distance);
        nodes_dis_BFS_peak = nodes_BFS_Peak;
        long num_state = 0;
        // compute distance at step k
        distance_ex.push_back(distance[0]);

        watchBFS_ex.reset();
        watchBFS_ex.note_time();
        for (size_t i=1; i<distance.size(); i++) {
            distance_ex.push_back(distance[i] ^ distance[i-1]);
        }
        watchBFS_ex.note_time();
        time_BFS_ex = watchBFS_ex.get_last_seconds();

        nodes_dis_ex_BFS = forest1->getNodeManUsed(distance_ex);
        nodes_dis_ex_BFS_peak = forest1->getNodeManPeak();

        // compute distribution
        for (size_t i=0; i<distance.size(); i++) {
            apply(CARDINALITY, distance[i], num_state);
            distribution_state.push_back(num_state);
            distribution_node.push_back(forest1->getNodeManUsed(distance[i]));

            apply(CARDINALITY, distance_ex[i], num_state);
            distribution_ex_state.push_back(num_state);
            distribution_ex_node.push_back(forest1->getNodeManUsed(distance_ex[i]));
        }

        // push back don't care set
        Func all(forest1);
        all.constant(1);
        distance_ex.push_back(all ^ distance.back());
        // push back #state and #node for don't-care set
        apply(CARDINALITY, distance_ex.back(), num_state);
        distribution_ex_state.push_back(num_state);
        distribution_ex_node.push_back(forest1->getNodeManUsed(distance_ex.back()));

        if (M == 2 && N == 2) {
            DotMaker dot1(forest1, "BFS_distance");
            dot1.buildGraph(distance_ex);
            dot1.runDot("pdf");
        }

        // choose one to remove and concretize
        // TBD
    }

    // reset the forest
    // forest1->deregisterFunc();
    // forest1->registerFunc(target);
    // forest1->markAllFuncs();
    // forest1->markSweep();
}

int main(int argc, const char** argv)
{
    /* Process arguments and initialize BDD forests*/
    if (!processArgs(argc, argv)) return usage(argv[0]);
    std::cout << "Solving " << N << " x " << M << "-sliding puzzle." << std::endl;
    std::cout << "The [Set] BDD type: " << setType << "\t The [Relation] BDD type: " << relType << std::endl;
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
    // initialize forests
    bits = static_cast<uint16_t>(std::ceil(log2(N*M)));
    std::cout << "bits: " << bits << std::endl;
    uint16_t levels = bits * N * M;
    ForestSetting setting1(setType, levels);
    ForestSetting setting2(relType, levels);
    // for distance computing
    if (isComputeDistance) {
        setting1.setRangeType(INTEGER);
        setting1.setPosInf(1);
        // setting1.setMaxRange(maxStep);
        if (setting1.getEncodeMechanism() == EDGE_PLUSMOD) {
            setting1.setMaxRange(maxStep);
        }
    } else {
        setting1.setRangeType(BOOLEAN);
        if (setting1.getEncodeMechanism() == EDGE_PLUSMOD) {
            setting1.setMaxRange(maxStep);
        }
    }
    setting1.output(std::cout);
    forest1 = new Forest(setting1);
    forest2 = new Forest(setting2);
    // setting for MT or EV that is converted from MR
    if (isConvert) {
        ForestSetting setting3(newType, levels);
        setting3.setRangeType(INTEGER);
        setting3.setPosInf(1);
        forest3 = new Forest(setting3);
    }
    /* Encode final target configure to BDD */
    Func target(forest1);
    target = encodePuzzle2BDD(conf);
    // print
    // if (isEmptyBot || isEmptyTop) {
    //     target.getEdge().print(std::cout);
    //     DotMaker dot1(forest1, "target_Conf");
    //     dot1.buildGraph(target);
    //     dot1.runDot("pdf");

    //     delete forest1;
    //     delete forest2;
    //     return 0;
    // }

    forest1->registerFunc(target);
    // std::cout << "encode puzzle done" << std::endl;
    // std::cout << "\tnumber of nodes: " << forest1->getNodeManUsed(target) << std::endl;
    // long num = 0;
    // apply(CARDINALITY, target, num);
    // std::cout << "\tnumber of states: " << num << std::endl;
    /* Build forward functions */
    // each position takes 4 slots for down, up, left, right
    // some may be constant 0 because of invalid forward direction
    Func forward(forest2);
    // std::vector<Func> relations(4*M*N, Func(forest2));
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
            std::cout << "trans: position = " << position  << " direction: " << direction << std::endl;
            forward = trans(position, direction);
            relations.push_back(forward);
            // relations[4*(position-1)+direction-1] = forward;
            forest2->registerFunc(forward);
            // std::cout << "trans done, number of nodes: " << forest2->getNodeManUsed(relations[4*(position-1)+direction-1]) << std::endl;
            std::cout << "trans done, number of nodes: " << forest2->getNodeManUsed(forward) << std::endl;
            distribution_rel_node.push_back(forest2->getNodeManUsed(forward));
            apply(CARDINALITY, forward, numStateRel);
            distribution_rel_state.push_back(numStateRel);
            forward.getEdge().print(std::cout);
            std::cout << std::endl;
            // try to union relations differently
            if (isRelationUnion) {
                // if (isEmptyBot || isEmptyTop) {
                //     if (direction == 1 || direction == 2) { // up and down
                //         relations[0] |= forward;
                //     } else {    // left and right
                //         relations[1] |=forward;
                //     }
                // } else {
                //     relations[0] |= forward;
                // }
                relations[0] |= forward;
            }
        }
    }
    std::cout << "Number of valid relations: " << relations.size() << std::endl;
    // union relations to relations[0]
    // if (isRelationUnion) {
    //     for (size_t i=0; i<relations.size(); i++) {
    //         relations[0] |= relations[i];
    //     }
    //     forest2->deregisterFunc();
    //     forest2->registerFunc(relations[0]);
    //     distribution_rel_node[0] = forest2->getNodeManUsed(relations[0]);
    //     apply(CARDINALITY, relations[0], numStateRel);
    //     distribution_rel_state[0] = numStateRel;
    // }

    // DotMaker dot1(forest2, "relations");
    // dot1.buildGraph(relations);
    // dot1.runDot("pdf");

    // evaluate forward relation
    // if (!evalTrans(relations)) {
    //     exit(0);
    // }
    // count nodes
    nodes_rel = forest2->getNodeManUsed(relations);
    nodes_rel_Peak = forest2->getNodeManPeak();

    /* ========================================================================================================
        Compute state space using saturation 
       =======================================================================================================*/
    if (!isComputeDistanceBFS) compute_saturation(target, relations);

    /* ========================================================================================================
        Compute state space using chain search
       =======================================================================================================*/
    if (!isComputeDistanceBFS && !isComputeDistance) compute_chain(target, relations);

    /* ========================================================================================================
        Compute state space using BFS
       =======================================================================================================*/
    if (!isComputeDistance) compute_BFS(target, relations);

    // /* Compute state space using BFS without image */
    // Func states(forest1);
    // watchBFS0.reset();
    // watchBFS0.note_time();
    // getRes_correct = SSG(!(N>2 || M>2), states, newTimeLimit);
    // watchBFS0.note_time();
    // // record time
    // time_correct = watchBFS0.get_last_seconds();
    // // record #states
    // // apply(CARDINALITY, states, state_correct);
    // // record #nodes
    // nodes_correct = forest1->getNodeManUsed(states);

    /* ========================================================================================================
        Convert to target forest
       =======================================================================================================*/
    /* ========================================================================================================
        Concretization
       =======================================================================================================*/
    if (isConvert) {
        distance_convert_ex = convert();
        nodes_convert_dis_ex = forest3->getNodeManUsed(distance_convert_ex);
        if (isConcretize) {
            // restrict
            watch_convert_rst.reset();
            watch_convert_rst.note_time();
            distance_convert_ex_rst = concretizeFunc_rst(distance_convert_ex);
            watch_convert_rst.note_time();
            time_convert_rst = watch_convert_rst.get_last_seconds();
            std::cout << "done rst!\n";
            nodes_convert_dis_ex_rst = forest3->getNodeManUsed(distance_convert_ex_rst);
            // one-sided-match
            watch_convert_osm.reset();
            watch_convert_osm.note_time();
            distance_convert_ex_osm = concretizeFunc_osm(distance_convert_ex);
            watch_convert_osm.note_time();
            time_convert_osm = watch_convert_osm.get_last_seconds();
            std::cout << "done osm!\n";
            nodes_convert_dis_ex_osm = forest3->getNodeManUsed(distance_convert_ex_osm);
            //two-sided-match
            watch_convert_tsm.reset();
            watch_convert_tsm.note_time();
            distance_convert_ex_tsm = concretizeFunc_tsm(distance_convert_ex);
            watch_convert_tsm.note_time();
            time_convert_tsm = watch_convert_tsm.get_last_seconds();
            std::cout << "done tsm!\n";
            nodes_convert_dis_ex_tsm = forest3->getNodeManUsed(distance_convert_ex_tsm);
        }
    } else {
        if (isConcretize) {
            concretize_rst();
            concretize_osm();
            concretize_tsm();
        }
    }    

    /* ========================================================================================================
        Report result
       =======================================================================================================*/
    if (!isReportToFile) {
        std::cout << "Done!" << std::endl;
        std::cout << "**********************************************************" << std::endl;
        int align = 30;
        std::cout << std::left << std::setw(align) << "Sliding puzzle:" << N << " x " << M  << std::endl;
        std::cout << std::left << std::setw(align) << "Bits per position: " << bits << std::endl;
        std::cout << std::left << std::setw(align) << "Level:" << forest1->getSetting().getNumVars() << std::endl;
        std::cout << std::left << std::setw(align) << "The [Set] type:" << forest1->getSetting().getName() << std::endl;
        std::string encodingType = "Default";
        if (isEmptyBot) encodingType = "Empty_Bot";
        if (isEmptyTop) encodingType = "Empty_Top";
        std::cout << std::left << std::setw(align) << "Encoding type:" << encodingType << std::endl;
        std::cout << std::left << std::setw(align) << "The [Relation] type:" << forest2->getSetting().getName() << std::endl;
        report(std::cout);
    } else {
        std::cout << "**********************************************************" << std::endl;
        std::string fileName = forest1->getSetting().getName();
        fileName += "_";
        fileName += forest2->getSetting().getName();
        fileName += "_";
        fileName += std::to_string(N);
        fileName += "_";
        fileName += std::to_string(M);
        if (isComputeDistance) {
            fileName += "_Dis";
        }
        if (isComputeDistanceBFS) {
            fileName += "_MR";
        }
        if (isRelationUnion) {
            fileName += "_UR";
        }
        if (isEmptyTop) {
            fileName += "_ET";
        }
        if (isEmptyBot) {
            fileName += "_EB";
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