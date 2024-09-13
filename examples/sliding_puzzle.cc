/* This is an example of building the set of configurations (states) reachable to target configure for
 * user-specified sliding puzzle.
 * 
 */

#include <iostream>
#include <vector>
#include <cmath>
#include "brave_dd.h"

using namespace BRAVE_DD;

// The size of the puzzle
uint16_t N;     // row
uint16_t M;     // column
uint16_t bits;  // bits per position

/**
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
 */

// BDD setting
std::string setType, relType;

// BDD forests
Forest* forest1;   // Set
Forest* forest2;   // Relation

// Helper function to print the puzzle state (configure)
void printPuzzleState(const std::vector<std::vector<uint16_t> >& puzzle)
{
    for (const auto& row : puzzle) {
        for (uint16_t val : row) {
            if (val == 0) std::cout << "\t  "; // empty space
            else std::cout << "\t" << val;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/** 
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
            if (from%M == 1) {  // invalid left
                result.constant(0);
                return result;
            }
            to = from - 1;  // position to
            break;
        case 4: // Right
            if (from%M == 0) {  // invalid right
                result.constant(0);
                return result;
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
    /* Set dependance vector based on position FROM and TO*/
    for (int i=0; i<bits; i++) {
        dependance[startFrom+i] = 1;
        dependance[startTo+i] = 1;
    }
    /* Intermediates and constant zero*/
    Func varTo(forest2), varFrom(forest2), zero(forest2);
    zero.constant(0);
    /* result starts from constant true */
    result.constant(1);
    /* Enabling */
    // position TO is empty: X_to == 0
    for (uint16_t i=0; i<bits; i++) {
        varTo.variable(startTo+i, 0);
        result &= (varTo & zero) | (!varTo & !zero);    // equivalence
    }
    /* Firing */
    // sliding tile at position FROM to position TO: X'_to = X_from, X'_from = 0
    for (uint16_t i=0; i<bits; i++) {
        varTo.variable(startTo+i, 1);
        varFrom.variable(startFrom+i, 0);
        result &= (varTo & varFrom) | (!varTo & !varFrom);  // equivalence
        varFrom.variable(startFrom+i, 1);
        result &= (varFrom & zero) | (!varFrom & !zero);    // equivalence
    }
    /* Identities (dependance) */
    zero.identity(dependance);
    return result & zero;
}

// Encode the puzzle state (configure) as a BDD
Func encodePuzzle2BDD(const std::vector<std::vector<uint16_t> >& puzzle)
{
    /* Assuming forest1 is initialized and result starts from constant 1 */
    Func result(forest1);
    result.trueFunc();
    /* Intermediates */
    Func var(forest1);
    uint16_t row, col;
    /* Encoding */
    for (uint16_t i=1; i<=N*M; i++) {
        row = (i%M==0)?i/M-1:i/M;
        col = (i%M==0)?M-1:i%M-1;
        for (uint16_t b=0; b<bits; b++) {
            var.variable(row*M+b+1);
            if (puzzle[row][col] & (0x01<<b)) {
                result &= var;
            } else {
                result &= !var;
            }
        }
    }
    return result;
}

bool processArgs(int argc, const char** argv)
{
    // default types
    setType = "RexBDD";
    relType = "MTBMxD";
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
    return 1;
}

int main(int argc, const char** argv)
{
    /* Process arguments and initialize BDD forests*/
    if (!processArgs(argc, argv)) return usage(argv[0]);
    std::cout << "Solving " << N << " x " << M << "-sliding puzzle." << std::endl;
    std::cout << "Set BDD type: " << setType << "\t Relation BDD type: " << relType << std::endl;
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
    bits = (const uint16_t)std::ceil(log2(N*M));
    uint16_t levels = bits * N * M;
    ForestSetting setting1(setType, levels);
    ForestSetting setting2(relType, levels);
    forest1 = new Forest(setting1);
    forest2 = new Forest(setting2);
    /* Encode final target configure to BDD */
    Func target(forest1);
    target = encodePuzzle2BDD(conf);
    /* Build forward functions */
    // each position takes 4 slots for down, up, left, right
    // some may be constant 0 because of invalid forward direction
    Func forward(forest2);
    FuncArray forwards(forest2, 4*M*N);
    for (int n=0; n<4*N*M; n++) {
        forwards.add(trans(n, n%4+1));
    }
    /* Compute state space using saturation */
    Func states(forest1);
    states = saturate(target, forwards);
    /* Compute distance */
    // TBD
    std::cout << "Done!" << std::endl;
    delete forest1;
    delete forest2;
    return 0;
}