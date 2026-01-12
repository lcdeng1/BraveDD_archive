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
 * Last Update Date: Nov 10, 2025
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
#include <numeric>
#include <iomanip>
#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;
using PuzzleState = std::vector<std::vector<uint16_t>>; // 2D representation of the puzzle

bool isHelp = 0;
// The size of the puzzle
uint16_t N;     // row
uint16_t M;     // column
uint16_t BITS;  // bits per position
int     maxStep = 40;

PuzzleState positionMap;

// Timers
timer watch;
double timeLimit = 1800.0, newTimeLimit = timeLimit;
bool isTimeLimitGlobal = 0;
// program flags
/* [Encoding index]:
 *  0: position as index: which tile for position;
 *  1: tile as index: which position for tile;
 *  2: store empty separately: seq of tiles, empty tile's position TOP
 *  3: store empty separately: seq of tiles, empty tile's position BOT
 */
int encodingMethod = 0;
/* [Scanning patterns]
 *  0: standard: left to right and top to bottom
 *  1: wave
 *  2: spiral
 */
int scanningPattern = 0;
/* [Initial position] */
int initialPosition = 1;
/* [Direction]
 *  0: horizontal
 *  1: vertical
 *  2: clockwise
 *  3: counterclockwise
 *  4: diagonal-right
 *  5: diagonal-left
 */
int scanningDirection = 0;
bool isScanningReversed = 0;
bool isBitsReversed = 0;


bool isEmptyTop = 0;    // [Variable Ordering] new way for encoding
bool isEmptyBot = 0;
bool isReportToFile = 0;
bool isComputeDistance = 0;
bool isRelationUnion = 0;
bool isRelationGroup = 0;
bool isUptoDistance = 0;
bool isShowSize = 0;
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
double timeExplore = 0.0;
// number of reachable states
long num_states = 0;
// number of nodes
uint64_t num_nodes_final = 0, num_nodes_peak = 0, num_rel_nodes_final = 0, num_rel_nodes_peak = 0;

// internal flags
int algorithm = 0;
bool isComputeDistanceMR = 0;
bool isConvert = 0;

// set of states reachable to target at exact k steps
std::vector<Func> distance;
// distribution for relations
std::vector<long> distribution_rel_state;
std::vector<long> distribution_rel_node;
// distribution for "exact" distance
std::vector<long> distribution_state;
std::vector<long> distribution_node;

// For concretization
std::vector<int> distance_permutation;
std::vector<Func> distance_concretized;
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

uint16_t nextRow(const uint16_t& row, const uint16_t& col)
{
    uint16_t ans = row;
    uint16_t initialRow = (initialPosition-1) / M;
    uint16_t initialCol = (initialPosition-1) % M;
    if (scanningPattern == 0) {
        // standard
        if (scanningDirection == 0) {
            if (col == (M-1 - initialCol)) ans = (initialRow == 0) ? ans+1 : ans-1;
        } else if (scanningDirection == 1) {
            if (initialRow == 0) {
                ans = (ans+1) % N;
            } else {
                ans = (ans == 0) ? N-1 : ans - 1;
            }
        } else if (scanningDirection == 4) {
            //
        } else if (scanningDirection == 5) {
            //
        } else {
            std::cerr << "Illegal scanning direction for \"Standard\"." << std::endl;
            exit(1);
        }
    } else if (scanningPattern == 1) {
        // wave
        if (scanningDirection == 0) {
            uint16_t diffRow = (row > initialRow) ? row - initialRow : initialRow - row;
            if (((diffRow % 2 == 0) && (col == (M-1-initialCol))) || ((diffRow % 2 == 1) && (col == initialCol))) ans = (initialRow == 0) ? ans+1 : ans-1;
        } else if (scanningDirection == 1) {
            uint16_t diffCol = (col > initialCol) ? col -initialCol : initialCol -col;
            if ((diffCol % 2 == 0) && (row != (N-1-initialRow))) {
                ans = (initialRow == 0) ? ans+1 : ans-1;
            } else if ((diffCol % 2 == 1) && (row != initialRow)) {
                ans = (initialRow == 0) ? ans-1 : ans+1;
            }
        } else if (scanningDirection == 4) {
            //
        } else if (scanningDirection == 5) {
            //
        } else {
            std::cerr << "Illegal scanning direction for \"Wave\"." << std::endl;
            exit(1);
        }
    } else if (scanningPattern == 2) {
        uint16_t layer = MIN(MIN(row, (uint16_t)(N-1-row)), MIN(col, (uint16_t)(M-1-col)));
        initialRow = (initialRow == 0) ? initialRow+layer : initialRow-layer;
        initialCol = (initialCol == 0) ? initialCol+layer : initialCol-layer;
        // single row
        if (2*initialRow == N-1) return ans;
        // spiral
        if (scanningDirection == 2) {
            // go to next layer
            if ((initialRow == layer) && (initialCol == layer)) {
                if ((row == initialRow+1) && (col == initialCol)) return ans;
            } else if ((initialRow == layer) && (initialCol == M-1-layer)) {
                if ((row == initialRow) && (col == initialCol-1)) return ans+1;
            } else if ((initialRow == N-1-layer) && (initialCol == layer)) {
                if ((row == initialRow) && (col == initialCol+1)) return ans-1;
            } else if ((initialRow == N-1-layer) && (initialCol == M-1-layer)) {
                if ((row == initialRow-1) && (col == initialCol)) return ans;
            }
            if ((col == M-1-layer) && (row != N-1-layer)) {
                ans++;
            } else if ((col == layer) && (row != layer)) {
                ans--;
            }
        } else if (scanningDirection == 3) {
            // go to next layer
            if ((initialRow == layer) && (initialCol == layer)) {
                if ((row == initialRow) && (col == initialCol+1)) return ans+1;
            } else if ((initialRow == layer) && (initialCol == M-1-layer)) {
                if ((row == initialRow+1) && (col == initialCol)) return ans;
            } else if ((initialRow == N-1-layer) && (initialCol == layer)) {
                if ((row == initialRow-1) && (col == initialCol)) return ans;
            } else if ((initialRow == N-1-layer) && (initialCol == M-1-layer)) {
                if ((row == initialRow) && (col == initialCol-1)) return ans-1;
            }
            if ((col == M-1-layer) && (row != layer)) {
                ans--;
            } else if ((col == layer) && (row != N-1-layer)) {
                ans++;
            }
        } else {
            std::cerr << "Illegal scanning direction for \"Spiral\"." << std::endl;
            exit(1);
        }
    } else {
        std::cerr << "Unknown scanning pattern." << std::endl;
        exit(1);
    }
    return ans;
}

uint16_t nextCol(const uint16_t& row, const uint16_t& col)
{
    uint16_t ans = col;
    uint16_t initialRow = (initialPosition-1) / M;
    uint16_t initialCol = (initialPosition-1) % M;
    if (scanningPattern == 0) {
        // standard
        if (scanningDirection == 0) {
            if (initialCol == 0) {
                ans = (ans+1) % M;
            } else {
                ans = (ans == 0) ? M-1 : ans - 1;
            }
        } else if (scanningDirection == 1) {
            if (row == (N-1 - initialRow)) ans = (initialCol == 0) ? ans+1 : ans-1;
        } else {
            std::cerr << "Illegal scanning direction for \"Standard\"." << std::endl;
            exit(1);
        }
    } else if (scanningPattern == 1) {
        // wave
        if (scanningDirection == 0) {
            uint16_t diffRow = (row > initialRow) ? row -initialRow : initialRow -row;
            if ((diffRow % 2 == 0) && (col != (M-1-initialCol))) {
                ans = (initialCol == 0) ? ans+1 : ans-1;
            } else if ((diffRow % 2 == 1) && (col != initialCol)) {
                ans = (initialCol == 0) ? ans-1 : ans+1;
            }
        } else if (scanningDirection == 1) {
            uint16_t diffCol = (col > initialCol) ? col - initialCol : initialCol - col;
            if (((diffCol % 2 == 0) && (row == (N-1-initialRow))) || ((diffCol % 2 == 1) && (row == initialRow))) ans = (initialCol == 0) ? ans+1 : ans-1;
        } else {
            std::cerr << "Illegal scanning direction for \"Wave\"." << std::endl;
            exit(1);
        }
    } else if (scanningPattern == 2) {
        uint16_t layer = MIN(MIN(row, (uint16_t)(N-1-row)), MIN(col, (uint16_t)(M-1-col)));
        initialRow = (initialRow == 0) ? initialRow+layer : initialRow-layer;
        initialCol = (initialCol == 0) ? initialCol+layer : initialCol-layer;
        if (2*initialCol == M-1) return ans;
        // spiral
        if (scanningDirection == 2) {
            // go to next layer
            if ((initialRow == layer) && (initialCol == layer)) {
                if ((row == initialRow+1) && (col == initialCol)) return ans+1;
            } else if ((initialRow == layer) && (initialCol == M-1-layer)) {
                if ((row == initialRow) && (col == initialCol-1)) return ans;
            } else if ((initialRow == N-1-layer) && (initialCol == layer)) {
                if ((row == initialRow) && (col == initialCol+1)) return ans;
            } else if ((initialRow == N-1-layer) && (initialCol == M-1-layer)) {
                if ((row == initialRow-1) && (col == initialCol)) return ans-1;
            }
            if ((row == N-1-layer) && (col != layer)) {
                ans--;
            } else if ((row == layer) && (col != M-1-layer)) {
                ans++;
            }
        } else if (scanningDirection == 3) {
            // go to next layer
            if ((initialRow == layer) && (initialCol == layer)) {
                if ((row == initialRow) && (col == initialCol+1)) return ans;
            } else if ((initialRow == layer) && (initialCol == M-1-layer)) {
                if ((row == initialRow+1) && (col == initialCol)) return ans-1;
            } else if ((initialRow == N-1-layer) && (initialCol == layer)) {
                if ((row == initialRow-1) && (col == initialCol)) return ans+1;
            } else if ((initialRow == N-1-layer) && (initialCol == M-1-layer)) {
                if ((row == initialRow) && (col == initialCol-1)) return ans;
            }
            if ((row == N-1-layer) && (col != M-1-layer)) {
                ans++;
            } else if ((row == layer) && (col != layer)) {
                ans--;
            }
        } else {
            std::cerr << "Illegal scanning direction for \"Spiral\"." << std::endl;
            exit(1);
        }
    } else {
        std::cerr << "Unknown scanning pattern." << std::endl;
        exit(1);
    }
    return ans;
}

// Get the sequence of values to be encoded
std::vector<uint16_t> mapping(const PuzzleState& puzzle)
{
    // illegal initial position
    if (!((initialPosition % M == 1 || initialPosition % M == 0) && ((initialPosition-1) / M == 0 || (initialPosition-1) / M == N-1))) {
        std::cerr << "Illegal initial position to scan." << std::endl;
        exit(1);
    }
    positionMap = std::vector<std::vector<uint16_t>>(N, std::vector<uint16_t>(M,0));
    std::vector<uint16_t> map(N*M, 0);
    std::vector<uint16_t> positions;
    std::vector<uint16_t> tiles;
    std::vector<std::vector<bool>> explored(N, std::vector<bool>(M,0));
    uint16_t initialRow = (initialPosition-1) / M;
    uint16_t initialCol = (initialPosition-1) % M;
    uint16_t row = initialRow, col = initialCol;
    // scanning
    for (uint16_t i=1; i<=N*M; i++) {
        // std::cerr << "(rwo, col): " << row << ", " << col << "\n";
        if (!explored[row][col]) {
            explored[row][col] = 1;
            positionMap[row][col] = (isScanningReversed) ? N*M+1-i : i;
            positions.push_back(i-1);
            tiles.push_back(puzzle[row][col]);
            // update row and col
            uint16_t preRow = row, preCol = col;
            row = nextRow(preRow, preCol);
            col = nextCol(preRow, preCol);
        } else {
            break;
        }
    }
    // print for test
    std::cerr << std::left << std::setw(18) << "Encoding method: " << encodingMethod << "\n";
    std::cerr << std::left << std::setw(18) << "Pattern: " << scanningPattern << "\n";
    std::cerr << std::left << std::setw(18) << "Initial position: " << initialPosition << "\n";
    std::cerr << std::left << std::setw(18) << "Direction: " << scanningDirection << "\n";
    std::cerr << "Position map:\n";
    printPuzzleState(positionMap);
    std::cerr << std::left << std::setw(12) << "Positions: ";
    for (size_t i=0; i<positions.size(); i++) {
        std::cerr << positions[i]+1 << " ";
    }
    std::cerr << "\n";
    std::cerr << std::left << std::setw(12) << "Tiles: ";
    for (size_t i=0; i<positions.size(); i++) {
        std::cerr << tiles[i] << " ";
    }
    std::cerr << "\n";

    if (isScanningReversed) std::reverse(positions.begin(), positions.end());
    // sort
    if (encodingMethod == 0) {
        // position as index
        map = tiles;
        if (isScanningReversed) std::reverse(map.begin(), map.end());
    } else if (encodingMethod == 1) {
        // tile as index
        std::vector<size_t> idx(positions.size());
        std::iota(idx.begin(), idx.end(), 0);

        // sort indices based on A
        std::sort(idx.begin(), idx.end(),
                [&](size_t i, size_t j) {
                    return tiles[i] < tiles[j];
                });
        for (size_t i=0; i<idx.size(); i++) {
            map[i] = positions[idx[i]];
        }
    } else {
        // empty position separately
        map = tiles;
        auto empty = std::find(map.begin(), map.end(), 0);
        int index = std::distance(map.begin(), empty);
        map.erase(empty);
        if (isScanningReversed) std::reverse(map.begin(), map.end());
        if (encodingMethod == 2) {
            map.push_back(positions[index]);
        } else {
            map.insert(map.begin(), positions[index]);
        }
    }
    std::cerr << std::left << std::setw(12) << "Maps: ";
    for (size_t i=0; i<map.size(); i++) {
        std::cerr << map[i] << " ";
    }
    std::cerr << "\n";
    return map;
}

Func encodePuzzle2BDD(const PuzzleState& puzzle)
{
    Func result(forest1);
    if (isComputeDistance) {
        result.constant(0);
    } else {
        result.trueFunc();
    }
    std::vector<uint16_t> map = mapping(puzzle);
    for (size_t i=0; i<map.size(); i++) {
        for (uint16_t b=0; b<BITS; b++) {
            Func var(forest1);
            Level lvl = (isBitsReversed) ? i*BITS+1+b : i*BITS+BITS-b;
            if (isComputeDistance) {
                // for distance
                var.variable(lvl, Value(SpecialValue::POS_INF), Value(0));
                if (map[i] & (0x01<<b)) {
                    apply(MAXIMUM, result, var, result);
                } else {
                    Func negVar(forest1);
                    negVar.variable(lvl, Value(0), Value(SpecialValue::POS_INF));
                    apply(MAXIMUM, result, negVar, result);
                }
            } else {
                // for SSG
                var.variable(lvl, Value(0), Value(1));
                if (map[i] & (0x01<<b)) {
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
                        negVar.variable(lvl, Value(1), Value(0));
                        apply(MINIMUM, result, negVar, result);
                    }
                }
            }
        }
    }
    return result;
}

Func different(const Level& start, const Level& end)
{
    uint16_t tiles = ((end-start)/M) + 1;
    Func result(forest2);
    result.constant(1);
    for (uint16_t i=0; i<tiles; i++) {
        for (uint16_t j=i+1; j<tiles; j++) {
            Func diffBit(forest2);
            diffBit.constant(0);
            for (uint16_t b=0; b<BITS; b++) {
                Func vari(forest2), varj(forest2);
                vari.variable(start+i*BITS+b, 0);
                varj.variable(start+j*BITS+b, 0);
                diffBit |= (vari ^ varj);
            }
            result &= diffBit;
        }
    }
    return result;
}

// transitions with direction: 1-Down, 2-Up, 3-Left, 4-Right
Func tran(const uint16_t& from, const char& direction)
{
    uint16_t row = (from-1) / M;
    uint16_t col = (from-1) % M;
    uint16_t fromPosition = positionMap[row][col];
    if (direction == 1) {
        row++;
    } else if (direction == 2) {
        row--;
    } else if (direction == 3) {
        col--;
    } else if (direction == 4) {
        col++;
    }
    uint16_t toPosition = positionMap[row][col];
    Func result(forest2), zero(forest2);
    result.constant(1);
    zero.constant(0);
    std::vector<bool> dependance(N*M*BITS+1, 0);
    if (encodingMethod == 0) {
        Level fromLvl = (fromPosition-1)*BITS+1, toLvl = (toPosition-1)*BITS+1;
        for (uint16_t b=0; b<BITS; b++) {
            dependance[fromLvl+b] = 1;
            dependance[toLvl+b] = 1;
        }
        /* Enabling: X_to = 0 */
        for (uint16_t b=0; b<BITS; b++) {
            Func var(forest2);
            Level lvl = (isBitsReversed) ? toLvl+b : toLvl+(BITS-1)-b;
            var.variable(lvl, 0);
            result &= !var;
        }
        Func notZero(forest2);
        notZero.constant(0);
        for (uint16_t b=0; b<BITS; b++) {
            Func var(forest2);
            Level lvl = (isBitsReversed) ? fromLvl+b : fromLvl+(BITS-1)-b;
            var.variable(lvl, 0);
            notZero |= var;
        }
        result &= notZero;
        /* Firing: X'_to = X_from, X'_from = 0 */
        for (uint16_t b=0; b<BITS; b++) {
            Func varTo(forest2), varFrom(forest2);
            Level lvlTo = (isBitsReversed) ? toLvl+b : toLvl+(BITS-1)-b;
            Level lvlFrom = (isBitsReversed) ? fromLvl+b : fromLvl+(BITS-1)-b;
            varTo.variable(lvlTo, 1);
            varFrom.variable(lvlFrom, 0);
            result &= ((varTo & varFrom) | ((!varTo) & (!varFrom)));
            varFrom.variable(lvlFrom, 1);
            result &= !varFrom;
        }
        zero.identity(dependance);
        result &= zero;
    } else if (encodingMethod == 1) {
        result.constant(0);
        Level emptyLvl = 1;
        for (uint16_t tile=1; tile<N*M; tile++) {
            Func subrelation(forest2);
            subrelation.constant(1);
            dependance = std::vector<bool>(N*M*BITS+1, 0);
            /* Enabling: X_empty = toPosition, X_[tile_from] = fromPosition */
            Level tileFromLvl = tile*BITS+1;
            for (uint16_t b=0; b<BITS; b++) {
                Func var(forest2);
                Level lvl = (isBitsReversed) ? emptyLvl+b : emptyLvl+(BITS-1)-b;
                dependance[lvl] = 1;
                var.variable(lvl, 0);
                if (!((toPosition-1) & (0x01 << b))) var = !var;
                subrelation &= var;
                lvl = (isBitsReversed) ? tileFromLvl+b : tileFromLvl+(BITS-1)-b;
                dependance[lvl] = 1;
                var.variable(lvl, 0);
                if (!((fromPosition-1) & (0x01 << b))) var = !var;
                subrelation &= var;
            }
            /* Firing: X'_empty = fromPosition, X'_[tile_from] = toPosition */
            for (uint16_t b=0; b<BITS; b++) {
                Func var(forest2);
                Level lvl = (isBitsReversed) ? emptyLvl+b : emptyLvl+(BITS-1)-b;
                dependance[lvl] = 1;
                var.variable(lvl, 1);
                if (!((fromPosition-1) & (0x01 << b))) var = !var;
                subrelation &= var;
                lvl = (isBitsReversed) ? tileFromLvl+b : tileFromLvl+(BITS-1)-b;
                dependance[lvl] = 1;
                var.variable(lvl, 1);
                if (!((toPosition-1) & (0x01 << b))) var = !var;
                subrelation &= var;
            }
            zero.identity(dependance);
            subrelation &= zero;
            result |= subrelation;
        }
    } else {
        // distance
        uint16_t diff = (toPosition > fromPosition) ? toPosition - fromPosition : fromPosition - toPosition;
        // empty level
        Level emptyLvl = (encodingMethod == 2) ? (N*M-1)*BITS+1 : 1;
        /* Enabling: X_empty = toPosition */
        for (uint16_t b=0; b<BITS; b++) {
            Func var(forest2);
            Level lvl = (isBitsReversed) ? emptyLvl+b : emptyLvl+(BITS-1)-b;
            dependance[lvl] = 1;
            var.variable(lvl, 0);
            if (!((toPosition-1) & (0x01 << b))) var = !var;
            result &= var;
        }
        /* Firing: X'_empty = fromPosition */
        for (uint16_t b=0; b<BITS; b++) {
            Func var(forest2);
            Level lvl = (isBitsReversed) ? emptyLvl+b : emptyLvl+(BITS-1)-b;
            var.variable(lvl, 1);
            if (!((fromPosition-1) & (0x01 << b))) var = !var;
            result &= var;
        }
        /* Firing: X'_{from,...to-1} or X'_{to,...,from-1} */
        if (diff != 1) {
            if (toPosition > fromPosition) {
                Level startLvl = (encodingMethod == 2) ? (fromPosition-1)*BITS+1 : fromPosition*BITS+1;
                Level endLvl = (encodingMethod == 2) ? (toPosition-2)*BITS+1 : (toPosition-1)*BITS+1;
                // X'_[to-1] = X_from
                for (uint16_t b=0; b<BITS; b++) {
                    Func varTo(forest2), varFrom(forest2);
                    Level lvlTo = (isBitsReversed) ? endLvl+b : endLvl+(BITS-1)-b;
                    Level lvlFrom = (isBitsReversed) ? startLvl+b : startLvl+(BITS-1)-b;
                    dependance[lvlTo] = 1;
                    dependance[lvlFrom] = 1;
                    varTo.variable(lvlTo, 1);
                    varFrom.variable(lvlFrom, 0);
                    result &= ((varTo & varFrom) | ((!varTo) & (!varFrom)));
                }
                // X'_from = X_[from+1],...,X'_[from+distance-1] = X_[to-1]
                for (uint16_t d=0; d<diff-1; d++) {
                    for (uint16_t b=0; b<BITS; b++) {
                        Func varLeft(forest2), varRight(forest2);
                        Level lvlLeft = (isBitsReversed) ? startLvl+d*BITS+b : startLvl+d*BITS+(BITS-1)-b;
                        Level lvlRight = (isBitsReversed) ? startLvl+(d+1)*BITS+b : startLvl+(d+1)*BITS+(BITS-1)-b;
                        dependance[lvlLeft] = 1;
                        dependance[lvlRight] = 1;
                        varLeft.variable(lvlLeft, 1);
                        varRight.variable(lvlRight, 0);
                        result &= ((varLeft & varRight) | ((!varLeft) & (!varRight)));
                    }
                }
                // result &= different(startLvl, endLvl);
            } else {
                Level startLvl = (encodingMethod == 2) ? (toPosition-1)*BITS+1 : toPosition*BITS+1;
                Level endLvl = (encodingMethod == 2) ? (fromPosition-2)*BITS+1 : (fromPosition-1)*BITS+1;
                // X'_to = X_[from-1]
                for (uint16_t b=0; b<BITS; b++) {
                    Func varTo(forest2), varFrom(forest2);
                    Level lvlTo = (isBitsReversed) ? startLvl+b : startLvl+(BITS-1)-b;
                    Level lvlFrom = (isBitsReversed) ? endLvl+b : endLvl+(BITS-1)-b;
                    dependance[lvlTo] = 1;
                    dependance[lvlFrom] = 1;
                    varTo.variable(lvlTo, 1);
                    varFrom.variable(lvlFrom, 0);
                    result &= ((varTo & varFrom) | ((!varTo) & (!varFrom)));
                }
                // X'_[to+1] = X_to,...,X'_[from-1] = X_[to+distance-1]
                for (uint16_t d=0; d<diff-1; d++) {
                    for (uint16_t b=0; b<BITS; b++) {
                        Func varLeft(forest2), varRight(forest2);
                        Level lvlLeft = (isBitsReversed) ? startLvl+(d+1)*BITS+b : startLvl+(d+1)*BITS+(BITS-1)-b;
                        Level lvlRight = (isBitsReversed) ? startLvl+d*BITS+b : startLvl+d*BITS+(BITS-1)-b;
                        dependance[lvlLeft] = 1;
                        dependance[lvlRight] = 1;
                        varLeft.variable(lvlLeft, 1);
                        varRight.variable(lvlRight, 0);
                        result &= ((varLeft & varRight) | ((!varLeft) & (!varRight)));
                    }
                }
                // result &= different(startLvl, endLvl);
            }
        }
        zero.identity(dependance);
        result &= zero;
    }
    // print to check
    if (N==2 && M==2 && from == 1 && direction == 1) {
        DotMaker dot(forest2);
        std::string name = forest2->getSetting().getName();
        name += "_";
        name += std::to_string(N);
        name += "_";
        name += std::to_string(M);
        name += "_";
        name += "From";
        name += "_";
        name += std::to_string(from);
        name += "_";
        if (direction == 1) name += "D";
        else if (direction == 2) name += "U";
        else if (direction == 3) name += "L";
        else if (direction == 4) name += "R";
        name += "_";
        name += std::to_string(encodingMethod);
        dot.hideTerminalZero();
        dot.buildGraph(result, name);
        dot.runDot(name, "pdf");
    }
    return result;
}

std::vector<Func> trans()
{
    std::vector<Func> results;
    for (uint16_t from=1; from<=N*M; from++) {
        for (char direction=1; direction<=4; direction++) {
            if (((from > (N-1)*M) && (direction == 1)) ||
                ((from <= M) && (direction == 2)) ||
                (((from%M == 1) || (M==1)) && (direction == 3)) ||
                (((from%M == 0) || (M==1)) && (direction == 4))) continue;
            Func relation = tran(from, direction);
            results.push_back(relation);
            // try to union relations
            if (isRelationUnion) {
                results[0] |= relation;
            }
        }
    }
    std::sort(results.begin(), results.end(), [](const Func& a, const Func& b) {
        // sorting logic
        Edge ae = a.getEdge(), be = b.getEdge();
        bool aIsTop = (ae.getNodeLevel() == a.getForest()->getSetting().getNumVars());
        bool bIsTop = (be.getNodeLevel() == b.getForest()->getSetting().getNumVars());
        if (aIsTop && bIsTop) {
            return ae.getNodeHandle() < be.getNodeHandle();
        } else if (aIsTop) {
            return true;
        } else if (bIsTop) {
            return false;
        } else {
            if (ae.getRule() != be.getRule()) {
                return ae.getRule() < be.getRule();
            } else if (ae.getNodeLevel() != be.getNodeLevel()) {
                return ae.getNodeLevel() > be.getNodeLevel();
            } else {
                return ae.getNodeHandle() > be.getNodeHandle();
            }
        }
    });
    for (size_t i=0; i<results.size(); i++) {
        results[i].getEdge().print(std::cerr);
        std::cerr << std::endl;
    }
    if (isRelationGroup) {
        std::vector<Func> groups;
        Func rel = results[0];
        Level top = rel.getEdge().getNodeLevel();
        for (uint16_t i=1; i<results.size(); i++) {
            if (i == (results.size()-1)) {
                rel |= results[i];
                groups.push_back(rel);
            }
            if (results[i].getEdge().getNodeLevel() != top) {
                groups.push_back(rel);
                rel = results[i];
                top = rel.getEdge().getNodeLevel();
            } else {
                rel |= results[i];
            }
        }
        std::cerr << "-----------------------------------" << std::endl;
        for (size_t i=0; i<groups.size(); i++) {
            groups[i].getEdge().print(std::cerr);
            std::cerr << std::endl;
        }
        return groups;
    }
    return results;
}

bool SSG_Frontier_upto(const Func& initial, const std::vector<Func>& relations)
{
    Func curr = initial;
    distance.push_back(curr);
    Func next(forest1);
    next.constant(0);
    int n = 0;
    while (true)
    {
        if (isRelationUnion) {
            std::cout << "Frontier: " << n;
            if (isShowSize) {
                long num = 0;
                apply(CARDINALITY, curr, num);
                std::cout << " size: " << num;
            }
            std::cout << std::endl;
            apply(POST_IMAGE, curr, relations[0], next);
        } else {
            std::cout << "Frontier: " << n;
            if (isShowSize) {
                long num = 0;
                apply(CARDINALITY, curr, num);
                std::cout << " size: " << num;
            }
            std::cout << std::endl;
            Func s_new(forest1);
            Func next_new(forest1);
            next_new.constant(0);
            for (size_t i=0; i< relations.size(); i++) {
                apply(POST_IMAGE, curr, relations[i], s_new);
                next_new |= s_new;
            }
            next |= next_new;
        }
        if (n==0) {
            next |= curr;
        }
        distance_permutation.push_back(n);
        // check fix point
        if (next == curr) break;
        // push back
        distance.push_back(next);
        // GC
        for (size_t i=0; i<distance.size(); i++) {
            forest1->markNodes(distance[i]);
        }
        forest1->markSweep();
        // update
        curr = next;
        n++;
    }
    return 1;
}

bool SSG_Frontier(const Func& initial, const std::vector<Func>& relations)
{
    Func curr = initial;
    distance.push_back(curr);
    Func pre(forest1);
    pre.constant(0);
    int n = 0;
    while (true)
    {
        Func next(forest1);
        next.constant(0);
        if (isRelationUnion) {
            std::cout << "Frontier: " << n;
            if (isShowSize) {
                long num = 0;
                apply(CARDINALITY, curr, num);
                std::cout << " size: " << num;
            }
            std::cout << std::endl;
            apply(POST_IMAGE, curr, relations[0], next);
        } else {
            std::cout << "Frontier: " << n;
            if (isShowSize) {
                long num = 0;
                apply(CARDINALITY, curr, num);
                std::cout << " size: " << num;
            }
            std::cout << std::endl;
            Func s_new(forest1);
            Func next_new(forest1);
            next_new.constant(0);
            for (size_t i=0; i< relations.size(); i++) {
                apply(POST_IMAGE, curr, relations[i], s_new);
                next_new |= s_new;
            }
            next |= next_new;
        }
        // next |= pre;
        // next = next ^ pre;
        next = next & !pre;
        distance_permutation.push_back(n);
        // check fix point
        if (next.getEdge().isConstantZero()) break;
        // push back
        distance.push_back(next);
        // GC
        for (size_t i=0; i<distance.size(); i++) {
            forest1->markNodes(distance[i]);
        }
        forest1->markSweep();
        // update
        pre = curr;
        curr = next;
        n++;
    }
    return 1;
}

Func BFS_distance(const Func& initial, const std::vector<Func>& relations)
{
    Func curr = initial;
    Func pre(forest1);
    pre.constant(SpecialValue::POS_INF);
    Func one(forest1);
    one.constant(1);
    int n = 0;
    while (true)
    {
        Func next(forest1);
        next.constant(0);
        if (isRelationUnion) {
            std::cout << "BFS: " << n;
            if (isShowSize) {
                long num = 0;
                apply(CARDINALITY, curr, num);
                std::cout << " size: " << num;
            }
            std::cout << std::endl;
            apply(POST_IMAGE, curr, relations[0], next);
            apply(PLUS, next, one, next);
        } else {
            std::cout << "BFS: " << n;
            if (isShowSize) {
                long num = 0;
                apply(CARDINALITY, curr, num);
                std::cout << " size: " << num;
            }
            std::cout << std::endl;
            Func s_new(forest1);
            Func next_new(forest1);
            next_new.constant(SpecialValue::POS_INF);
            for (size_t i=0; i< relations.size(); i++) {
                apply(POST_IMAGE, curr, relations[i], s_new);
                apply(MINIMUM, next_new, s_new, next_new);
            }
            apply(PLUS, next_new, one, next);
            // apply(MINIMUM, next, next_new, next);
        }
        apply(MINIMUM, curr, next, next);
        // next |= pre;
        // next = next ^ pre;
        // next = next & !pre;
        // check fix point
        if (next == curr) break;
        // GC
        for (size_t i=0; i<distance.size(); i++) {
            forest1->markNodes(distance[i]);
        }
        // forest1->markNodes(curr);
        forest1->markNodes(next);
        forest1->markSweep();
        // update
        pre = curr;
        curr = next;
        n++;
    }
    return curr;
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
    for (size_t d=0; d<distance.size()-1; d++) {
        converted = convertEdge(forest3->getSetting().getNumVars(), converted, distance[d].getEdge(), d);
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
            if (distance[distance_permutation[d]].evaluate(assignment) != Value(1)) {continue;}
            bool before = 0;
            for (size_t i=0; i<d; i++) {
                if (mr[i].evaluate(assignment) == Value(1)) {
                    before = 1;
                    break;
                }
            }
            if ((distance[distance_permutation[d]].evaluate(assignment) == Value(1)) && ((mr[d].evaluate(assignment) != Value(1)) || before)) {
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
                    [&](size_t i, size_t j) { return distribution_node[i] < distribution_node[j]; });

        watch_concretized.reset();
        watch_concretized.note_time();
        /* for exact distance */
        Func dc = distance.back();
        if (heuristics == 0) {
            distance_concretized.push_back(concretizeFunc_rst(distance[distance_permutation[0]], dc));
        } else if (heuristics == 1) {
            distance_concretized.push_back(concretizeFunc_osm(distance[distance_permutation[0]], dc));
        } else if (heuristics == 2) {
            distance_concretized.push_back(concretizeFunc_tsm(distance[distance_permutation[0]], dc));
        }
        for (size_t i=1; i<distance_permutation.size()-1; i++) {    // -1: remove the last one
            // update dc
            for (size_t k=0; k<i; k++) {
                dc |= distance[distance_permutation[k]];
            }
            // concretizing
            if (heuristics == 0) {
                distance_concretized.push_back(concretizeFunc_rst(distance[distance_permutation[i]], dc));
            } else if (heuristics == 1) {
                distance_concretized.push_back(concretizeFunc_osm(distance[distance_permutation[i]], dc));
            } else if (heuristics == 2) {
                distance_concretized.push_back(concretizeFunc_tsm(distance[distance_permutation[i]], dc));
            }
        }
        watch_concretized.note_time();
        time_concretized = watch_concretized.get_last_seconds();
        num_nodes_distance_ex_concretized = forest1->getNodeManUsed(distance_concretized);
        std::cout << "done concretization\n";
        // test
        if (isTestConcretize) testConcretizationMR(distance_concretized);
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
    timeExplore = watch.get_last_seconds();
    // record #states
    apply(CARDINALITY, states_Sat, num_states);
    // record #nodes
    num_nodes_final = forest1->getNodeManUsed(states_Sat);
    num_nodes_peak = forest1->getNodeManPeak();

    forest1->registerFunc(states_Sat);
    // /* Update time limit */
    // if (!isTimeLimitGlobal) newTimeLimit = time_Sat;
}

void compute_Frontier(const Func& target, const std::vector<Func>& relations)
{
    watch.reset();
    watch.note_time();
    getRes = (isUptoDistance) ? SSG_Frontier_upto(target, relations) : SSG_Frontier(target, relations);
    watch.note_time();
    // record time
    timeExplore = watch.get_last_seconds();
    // record #nodes
    num_nodes_final = forest1->getNodeManUsed(distance);
    num_nodes_peak = forest1->getNodeManPeak();
    // compute distribution
    long num_state = 0;
    for (size_t i=0; i<distance.size(); i++) {
        apply(CARDINALITY, distance[i], num_state);
        distribution_state.push_back(num_state);
        distribution_node.push_back(forest1->getNodeManUsed(distance[i]));
        if (isUptoDistance) {
            if (i == distance.size()-1) num_states = num_state;
        } else {
            num_states += num_state;
        }
    }
    // push back don't care set
    Func all(forest1);
    all.constant(1);
    for (size_t i=0; i<distance.size(); i++) {
        all = all & !distance[i];
    }
    distance.push_back(all);
    // push back #state and #node for don't-care set
    apply(CARDINALITY, distance.back(), num_state);
    distribution_state.push_back(num_state);
    distribution_node.push_back(forest1->getNodeManUsed(distance.back()));
}

void compute_BFS(const Func& target, const std::vector<Func>& relations)
{
    Func states_Sat(forest1);
    // Timer start
    watch.reset();
    watch.note_time();
    states_Sat = BFS_distance(target, relations);
    // apply(SATURATE, target, relations, states_Sat);
    watch.note_time();
    getRes = 1;
    // save for concretization
    distance_one_root = states_Sat;
    // record time
    timeExplore = watch.get_last_seconds();
    // record #states
    apply(CARDINALITY, states_Sat, num_states);
    // record #nodes
    num_nodes_final = forest1->getNodeManUsed(states_Sat);
    num_nodes_peak = forest1->getNodeManPeak();

    forest1->registerFunc(states_Sat);
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
            if ((strcmp("-encode", argv[i])==0) || (strcmp("-e", argv[i])==0)) {
                encodingMethod = atoi(argv[i+1]);
                i++;
                continue;
            }
            if ((strcmp("-pattern", argv[i])==0) || (strcmp("-pat", argv[i])==0)) {
                scanningPattern = atoi(argv[i+1]);
                if (scanningPattern == 2) scanningDirection = 2;
                i++;
                continue;
            }
            if ((strcmp("-initial", argv[i])==0) || (strcmp("-i", argv[i])==0)) {
                initialPosition = atoi(argv[i+1]);
                i++;
                continue;
            }
            if ((strcmp("-direction", argv[i])==0) || (strcmp("-dir", argv[i])==0)) {
                scanningDirection = atoi(argv[i+1]);
                i++;
                continue;
            }
            if ((strcmp("-algorithm", argv[i])==0) || (strcmp("-alg", argv[i])==0)) {
                algorithm = atoi(argv[i+1]);
                i++;
                continue;
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
            if ((strcmp("-relUnion", argv[i])==0) || (strcmp("-ru", argv[i])==0)) {
                isRelationUnion = 1;
                continue;
            }
            if ((strcmp("-relGroup", argv[i])==0) || (strcmp("-rg", argv[i])==0)) {
                isRelationGroup = 1;
                continue;
            }
            if ((strcmp("-concretize", argv[i])==0) || (strcmp("-cz", argv[i])==0)) {
                concretization = atoi(argv[i+1]);
                i++;
                continue;
            }
            /* Hide to users */
            if ((strcmp("-upto_distance", argv[i])==0) || (strcmp("-ud", argv[i])==0)) {
                isUptoDistance = 1;
                continue;
            }
            if ((strcmp("-show", argv[i])==0) || (strcmp("-s", argv[i])==0)) {
                isShowSize = 1;
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
            // option for encoding
            if (strcmp("-et", argv[i])==0) {
                isEmptyTop = 1;
                continue;
            }
            if (strcmp("-eb", argv[i])==0) {
                isEmptyBot = 1;
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
        explore_alg = "BFS";
    } else if (algorithm == 2) {
        explore_alg = "Frontier";
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
    out << std::left << std::setw(align) << explore_alg << timeExplore << std::endl;
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
    // std::vector<uint16_t> map = mapping(conf);
    // return 0;
    // initialize forests
    BITS = static_cast<uint16_t>(std::ceil(log2(N*M)));
    std::cout << "bits: " << BITS << std::endl;
    uint16_t levels = BITS * N * M;
    ForestSetting setting1(setType, levels);
    ForestSetting setting2(relType, levels);
    if (setting1.getRangeType() == BOOLEAN) {
        // compute distance?
        if (isComputeDistance) {
            isComputeDistance = 0;
            isComputeDistanceMR = 1;
            // isRelationUnion = 1;
            algorithm = 2;
        }
    } else {
        // compute distance?
        if (isComputeDistance) {
            setting1.setPosInf(1);  // trun on to support special value positive infinity
            if (N*M > 20) {
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
    // long numStateRel = 0;
    // for (int position=1; position<=N*M; position++) {
    //     for (int direction = 1; direction<=4; direction++) {
    //         if (((direction == 1) && (position > (N-1)*M)) 
    //             || ((direction == 2) && (position <= M))
    //             || ((direction == 3) && ((position%M == 1) || (M==1)))
    //             || ((direction == 4) && ((position%M == 0) || (M==1)))) {
    //             //invalid relation
    //             continue;
    //         }
    //         forward = trans(position, direction);
    //         // forward.getEdge().print(std::cerr);
    //         // std::cerr << std::endl;
    //         // if (position == 1 && direction == 1) {
    //         //     DotMaker dot(forest2);
    //         //     dot.buildGraph(forward, "relation_test0");
    //         //     dot.runDot("relation_test0", "pdf");
    //         // }
    //         relations.push_back(forward);
    //         forest2->registerFunc(forward);
    //         distribution_rel_node.push_back(forest2->getNodeManUsed(forward));
    //         apply(CARDINALITY, forward, numStateRel);
    //         distribution_rel_state.push_back(numStateRel);
    //         // try to union relations
    //         if (isRelationUnion) {
    //             relations[0] |= forward;
    //         }
    //     }
    // }
    relations = trans();
    std::cout << "Number of valid relations: " << relations.size() << std::endl;
    // count nodes
    num_rel_nodes_final = (isRelationUnion) ? forest2->getNodeManUsed(relations[0]) : forest2->getNodeManUsed(relations);
    num_rel_nodes_peak = forest2->getNodeManPeak();

    /* Explore reachable states */
    if (algorithm == 0) {
        compute_saturation(target, relations);
    } else if (algorithm == 1) {
        // compute_chain(target, relations);
        compute_BFS(target, relations);
    } else if (algorithm == 2) {
        compute_Frontier(target, relations);
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
        std::cout << std::left << std::setw(align) << "Bits per position: " << BITS << std::endl;
        std::cout << std::left << std::setw(align) << "Level:" << forest1->getSetting().getNumVars() << std::endl;
        std::cout << std::left << std::setw(align) << "The [Set] type:" << ((isConvert)? forest3 : forest1)->getSetting().getName() << std::endl;
        std::cout << std::left << std::setw(align) << "The [Relation] type:" << forest2->getSetting().getName() << std::endl;
        std::string encodingType = "Position as Index (Default)";
        if (encodingMethod == 1) encodingType = "Tile as Index";
        else if (encodingMethod == 2) encodingType = "Position as Index, Empty at Top";
        else if (encodingMethod == 3) encodingType = "Position as Index, Empty at Bottom";
        std::cout << std::left << std::setw(align) << "Encoding type:" << encodingType << std::endl;

        std::string pattern = "Standard (Default)";
        if (scanningPattern == 1) pattern = "Wave";
        else if (scanningPattern == 2) pattern = "Spiral";
        std::cout << std::left << std::setw(align) << "Scanning Pattern:" << pattern << std::endl;

        std::string initial = "Top Left (Default)";
        if (initialPosition == M) initial = "Top Right";
        else if (initialPosition == (N-1)*M+1) initial = "Bottom Left";
        else if (initialPosition == N*M) initial = "Bottom Right";
        std::cout << std::left << std::setw(align) << "Initial Position:" << initial << std::endl;

        std::string direction = "Horizontal (Default)";
        if (scanningDirection == 1) direction = "Vertical";
        else if (scanningDirection == 2) direction = "Clockwise";
        else if (scanningDirection == 3) direction = "Counter-Clockwise";
        std::cout << std::left << std::setw(align) << "Direction:" << direction << std::endl;

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
            fileName += (isUptoDistance) ? "_LE" : "_EX";
        }
        // if (isComputeDistanceMR) {
        //     fileName += "_MR";
        // }
        // if (isRelationUnion) {
        //     fileName += "_UR";
        // }
        std::string em = "PosIdx";
        if (encodingMethod == 1) em = "TileIdx";
        else if (encodingMethod == 2) em = "EpTop";
        else if (encodingMethod == 3) em = "EpBot";
        fileName += "_";
        fileName += em;
        fileName += "_";
        em = "Standard";
        if (scanningPattern == 1) em = "Wave";
        else if (scanningPattern == 2) em = "Spiral";
        fileName += em;
        fileName += "_";
        fileName += std::to_string(initialPosition);
        fileName += "_";
        em = "H";
        if (scanningDirection == 1) em = "V";
        else if (scanningDirection == 2) em = "Clk";
        else if (scanningDirection == 3) em = "Cclk";
        fileName += em;
        fileName += "_";
        std::string alg = "Saturation";
        if (algorithm == 1) alg = "BFS";
        fileName += alg;
        if (isScanningReversed) fileName += "_R";
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