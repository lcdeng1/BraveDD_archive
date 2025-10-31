/* 
 * -----------------------------------------------------------------------------
 *  N-Queens Solver using Binary Decision Diagrams (BDDs)
 * -----------------------------------------------------------------------------
 *  Overview:
 *  This program provides a symbolic solution to the classic N-Queens problem
 *  using Binary Decision Diagrams (BDDs) as the core data structure for Boolean
 *  constraint representation and manipulation.
 *
 *  The N-Queens problem asks for all possible placements of N queens on an
 *  N*N chessboard such that no two queens attack each other horizontally,
 *  vertically, or diagonally. Instead of exploring each board configuration
 *  explicitly, this solver encodes the problem as a Boolean formula and builds
 *  a BDD to represent all satisfying assignments symbolically.
 *
 *  Users can specify:
 *      - The board size (N)
 *      - The predefined BDD type
 *        (e.g., FBDD, CFBDD, SFBDD, CSFBDD, ZBDD, ESRBDD, CESRBDD, or REXBDD)
 * 
 *  The solver constructs the constraints:
 *      1. Row constraints: Each row must have exactly one queen
 *      2. Column constraints: Each column must have exactly one queen
 *      3. Diagonal constraints: No two queens can be placed on the same diagonal
 * 
 *  NOTE:
 *  The encoding used in this solver is 'one-hot', meaning each cell (i, j) 
 *  on the board is represented by a unique Boolean variable X_ij: 
 *  X_ij = 1 <=> a queen is placed on cell (i, j).
 *  As a result, the BDD will contain N*N variable levels. 
 *  While this representation is intuitive and works well for small to
 *  moderate N, it may potentially lead to more memory- and time-intensive than 
 *  Multi-valued Decision Diagrams (MDDs) encodings.
 * 
 *  Example usage:
 *      ./02_queens -n 8
 *      ./02_queens -n 8 -t fbdd
 * 
 * 
 * Author: Lichuan Deng
 * Version: 1.0
 * Last Update Date: Oct 27, 2025
 */

#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;

// Flag to output help info
bool isHelp = 0;
// The size of board
int N = 6;
// BDD type
std::string bdd_type = "RexBDD";
// BDD forest
Forest* forest;

/* Usage message */
int usage(const char* who)
{
    std::ostream& out = std::cerr;
    int align = 10;
    /* Strip leading directory, if any: */
    const char* name = who;
    for (const char* ptr=who; *ptr; ptr++) {
        if ('/' == *ptr) name = ptr+1;
    }
    out << std::left << std::setw(align) << "USAGE: " << name << " -n <N>" << std::endl;
    out << std::endl;
    out << std::left << std::setw(align) << "OPTIONS: "<< std::endl;
    out << std::left << std::setw(2*align) << "  -help, -h " << "Display help message and usage" << std::endl;
    out << std::left << std::setw(2*align) << "  -n <number>" << "Set the board size (number of queens). Default: 6" << std::endl;
    out << std::left << std::setw(2*align) << "  -type <string>" << "Select the predefined BDD type. Default: REXBDD" << std::endl;
    out << std::left << std::setw(2*align) << "" << "Supported: FBDD, CFBDD, SFBDD, CSFBDD, ZBDD, ESRBDD, CESRBDD, REXBDD" << std::endl;
    out << std::endl;
    out << std::left << std::setw(align) << "EXAMPLES: "<< std::endl;
    out << std::left << std::setw(align) << "" << name << " -n 8" << std::endl;
    out << std::left << std::setw(align) << "" << name << " -n 8 -type fbdd" << std::endl;
    return 1;
}

/* Help message */
int helpInfo(const char* who)
{
    std::ostream& out = std::cerr;
    int align = 10;
    out << std::left << std::setw(align) << "OVERVIEW: "
    << R"(
    This program symbolically encodes the N-Queens problem using Binary Decision
    Diagrams (BDDs). By selecting different BDD types, users can explore the
    trade-offs between memory usage, performance, and node sharing efficiency.
    )"
    << std::endl;
    out << std::left << std::setw(align) << "NOTE: "
    << R"(
    Each board cell is modeled as a distinct Boolean variable
    (one-hot encoding), resulting in N*N variable levels in the BDD. While this
    encoding is intuitive, it may lead to larger diagrams and longer solving
    times compared to multi-valued (MDD) approaches.
    )"
    << std::endl;
    out << "----------------------------------------------------------" << std::endl;
    return usage(who);
}

bool processArgs(int argc, const char** argv)
{
    for (int i=1; i<argc; i++) {
        if ('-' == argv[i][0]) {
            if ((strcmp("-help", argv[i])==0) || (strcmp("-h", argv[i])==0)) {
                isHelp = 1;
                return 1;
            }
            if (strcmp("-n", argv[i])==0) {
                N = atoi(argv[i+1]);
                i++;
                continue;
            }
            // options other than size
            if (strcmp("-type", argv[i])==0) {
                bdd_type = argv[i+1];
                i++;
                continue;
            }
        } else {
            return 1;
        }
    }
    return 0;
}

Func rowConstraint(const std::vector<std::vector<Func> >& board, int row)
{
    /* Start in the same forest */
    Func constraint1(forest);  // at most one 
    Func constraint2(forest);  // at least one
    constraint1.trueFunc();
    constraint2.falseFunc();

    /* No two queens in the same row */
    // At most one queen in the same row
    for (int i=0; i<N; i++) {
        for (int j=i+1; j<N; j++) {
            constraint1 &= !(board[row][i] & board[row][j]);
        }
    }
    // At least one queen in the same row
    for (int j=0; j<N; j++) {
        constraint2 |= board[row][j];
    }

    return constraint1 & constraint2;
}

Func colConstraint(const std::vector<std::vector<Func> >& board, int col)
{
    /* Start in the same forest */
    Func constraint1(forest);  // at most one 
    Func constraint2(forest);  // at least one
    constraint1.trueFunc();
    constraint2.falseFunc();

    /* No two queens in the same column */
    // At most one queen in the same column
    for (int i=0; i<N; i++) {
        for (int j=i+1; j<N; j++) {
            constraint1 &= !(board[i][col] & board[j][col]);
        }
    }
    // At least one queen in the same row
    for (int i=0; i<N; i++) {
        constraint2 |= board[i][col];
    }

    return constraint1 & constraint2;
}

Func diagConstraint(const std::vector<std::vector<Func> >& board)
{
    /* Start with true in the same forest */
    Func constraint(forest);
    constraint.trueFunc();

    /* No two queens in the same diagonal and anti-diagonal */
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            // check diagonals in both directions
            // diagonal
            for (int r = i + 1, c = j + 1; r < N && c < N; ++r, ++c) {
                constraint &= !(board[i][j] & board[r][c]);
                // GC
                if (N>6) {
                    forest->registerFunc(constraint);
                    forest->markAllFuncs();
                    forest->markSweep();
                    forest->deregisterFunc(constraint);
                }
            }
            // anti-diagonal
            for (int r = i + 1, c = j - 1; r < N && c >= 0; ++r, --c) {
                constraint &= !(board[i][j] & board[r][c]);
                // GC
                if (N>6) {
                    forest->registerFunc(constraint);
                    forest->markAllFuncs();
                    forest->markSweep();
                    forest->deregisterFunc(constraint);
                }
            }
        }
    }
    return constraint;
}

int main(int argc, const char** argv)
{
    /* Process Args */
    if (processArgs(argc, argv)) {
        if (isHelp) return helpInfo(argv[0]);
        return usage(argv[0]);
    }

    /* Initialize BraveDD forest */
    ForestSetting setting(bdd_type, N*N);   // Set the BDD type and the number of variables
    forest = new Forest(setting);
    // setting.output(std::cerr);

    std::cerr << "Using " << getLibInfo(0) << std::endl;
    std::cerr << "Solving " << N << std::left << std::setw(27) << "-Queens in BDD type: " << setting.getName() << std::endl;

    /* Create BDD variables for each cell on the board */
    std::vector<std::vector<Func> > board(N, std::vector<Func>(N, Func(forest)));
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            board[i][j].variable(i * N + j + 1);
            forest->registerFunc(board[i][j]); // register function to protect for GC
        }
    }
    /* Timer start */
    timer watch;
    watch.note_time();
    /* Row, Column, and Diagonal constraint */
    Func solution(board[0][0].getForest());
    solution.trueFunc();
    for (int i=0; i<N; i++) {
        solution &= rowConstraint(board, i);
        solution &= colConstraint(board, i);
        // mark nodes and GC
        forest->registerFunc(solution);
        forest->markAllFuncs();
        forest->markSweep();
        forest->deregisterFunc(solution);
    }
    forest->registerFunc(solution);
    solution &= diagConstraint(board);

    /* Timer end */
    watch.note_time();

    /* Output */
    int align = 36;
    long num;
    apply(CARDINALITY, solution, num);
    std::cerr << std::left << std::setw(align) << "Number of solutions: " << num << std::endl;
    std::cerr << std::left << std::setw(align) << "Elapsed time (seconds): " << watch.get_last_seconds() << std::endl;
    std::cerr << std::left << std::setw(align) << "Number of nodes (final): " << forest->getNodeManUsed(solution) << std::endl;
    std::cerr << std::left << std::setw(align) << "Number of nodes (peak): " << forest->getNodeManPeak() << std::endl;

    /* delete Forest */
    delete forest;
}