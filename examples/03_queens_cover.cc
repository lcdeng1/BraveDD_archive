/* 
 * -----------------------------------------------------------------------------
 *  Queens Cover Solver using Binary Decision Diagrams (BDDs)
 * -----------------------------------------------------------------------------
 *  Overview:
 *  This program provides a symbolic solution to the Queens Cover problem
 *  using Binary Decision Diagrams (BDDs) as the core data structure for Boolean
 *  constraint representation and manipulation.
 *
 *  The Queens Cover problem asks for a placement of the fewest number of queens
 *  on an N*N chessboard such that every cell is either occupied by a queen or
 *  attacked by at least one queen. Instead of searching over all placements
 *  explicitly, this solver encodes the covering conditions as a Boolean formula
 *  and constructs a BDD to represent all valid configurations symbolically.
 *
 *  Users can specify:
 *      - The board size (N)
 *      - The predefined BDD type
 *        (e.g., FBDD, CFBDD, SFBDD, CSFBDD, ZBDD, ESRBDD, CESRBDD, or REXBDD)
 * 
 *  The solver constructs the constraints:
 *      1. Each non-queen cell must be covered (attacked) by at least one queen.
 *      2. The number of queens placed can optionally be minimized.
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
 *      ./03_queens -n 8
 *      ./03_queens -n 8 -t fbdd
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
// Flag to output log info
bool isLog = 0;
// The size of board
int N = 6;
// BDD type
std::string bdd_type = "RexBDD";
// BDD forest
Forest* forest;

std::vector<std::vector<bool>> isComputed;
std::vector<std::vector<Func>> cache;

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
    out << std::left << std::setw(2*align) << "  -log, -l " << "Display log message in the process" << std::endl;

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
            if ((strcmp("-log", argv[i])==0) || (strcmp("-l", argv[i])==0)) {
                isLog = 1;
                continue;
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

Func rowConstraint(const std::vector<std::vector<Func> >& board, const int row)
{
    Func rowAttack(forest);
    rowAttack.falseFunc();
    /* Be attacked by the same row */
    for (int j=0; j<N; j++) {
        rowAttack |= board[row][j];
    }
    return rowAttack;
}

Func colConstraint(const std::vector<std::vector<Func> >& board, const int col)
{
    Func colAttack(forest);
    colAttack.falseFunc();
    /* Be attacked by the same column */
    for (int i=0; i<N; i++) {
        colAttack |= board[i][col];
    }
    return colAttack;
}

Func diagConstraint(const std::vector<std::vector<Func> >& board, int row, int col)
{
    Func diagAttack0(forest);
    diagAttack0.falseFunc();
    Func diagAttack1(forest);
    diagAttack1.falseFunc();
    /* Be attacked by the same diagonal */
    for (int i=row-1, j=col-1; i>=0 && j>=0; i--, j--) {
        diagAttack0 |= board[i][j];
    }
    for (int i=row+1, j=col+1; i<N && j<N; i++, j++) {
        diagAttack0 |= board[i][j];
    }
    /* Be attacked by the same anti-diagonal */
    for (int i=row+1, j=col-1; i<N && j>=0; i++, j--) {
        diagAttack1 |= board[i][j];
    }
    for (int i=row-1, j=col+1; i>=0 && j<N; i--, j++) {
        diagAttack1 |= board[i][j];
    }
    return diagAttack0 | diagAttack1;
}

/* This will construct a BDD represents that the constraint of exact n queens placed */
Func exactNQueens(const Level L, const Level n)
{
    Func ans(forest);
    // terminal case
    if ((L == 0) && (n == 0)) {
        ans.constant(1);
        return ans;
    }
    if (n > L) {
        ans.constant(0);
        return ans;
    }
    // check if it is computed
    if (isComputed[L][n]) {
        return cache[L][n];
    }
    Func var(forest);
    var.variable(L);
    ans = (var & exactNQueens(L-1, n-1)) | ((!var) & exactNQueens(L-1, n));
    cache[L][n] = ans;
    isComputed[L][n] = 1;
    return ans;
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
    std::cerr << "Solving " << N << std::left << std::setw(27) << "-Queens Cover in BDD type: " << setting.getName() << std::endl;

    /* Create BDD variables for each cell on the board */
    std::vector<std::vector<Func> > board(N, std::vector<Func>(N, Func(forest)));
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            board[i][j].variable(i * N + j + 1);
            forest->registerFunc(board[i][j]); // register function to protect for GC
        }
    }

    // /* Row, Column, and Diagonal constraint */
    std::vector<Func> rowConstraints;
    std::vector<Func> colConstraints;
    // pre compute row and column constraints
    for (int i=0; i<N; i++) {
        rowConstraints.push_back(rowConstraint(board, i));
        colConstraints.push_back(colConstraint(board, i));
        forest->registerFunc(rowConstraints[i]);
        forest->registerFunc(colConstraints[i]);
    }

    /* Initialize computing table for n queens constraints */
    isComputed = std::vector<std::vector<bool>>(N*N+1, std::vector<bool>(N+1, 0));
    cache = std::vector<std::vector<Func>>(N*N+1, std::vector<Func>(N+1, Func(forest)));
    /* check with exact n (from 1 to N) queens constraint */
    for (int n=1; n<=N; n++) {
        // clear cache
        for (size_t i=0; i<isComputed.size(); i++) {
            for (int j=0; j<=n; j++) {
                isComputed[i][j] = 0;
            }
        }
        // stop watch
        timer watch1;
        watch1.reset();
        watch1.note_time();
        Func nqueens = exactNQueens(N*N, n);
        watch1.note_time();
        forest->registerFunc(nqueens);
        // solutions
        Func solutions = nqueens;
        std::cerr << "Trying to cover with Queens: " << n << std::endl;
        std::cerr << "Adding constraints by row" << std::endl;
        // stop watch
        timer watch2;
        watch2.reset();
        watch2.note_time();
        // Func min_solution = nqueens & solution;
        for (int i=0; i<N; i++) {
            std::cerr << N-i;
            Func rowcov = nqueens;
            Func qir = rowConstraints[i];
            for (int j=0; j<N; j++) {
                Func col = colConstraints[j];
                col |= diagConstraint(board, i, j);
                col |= qir;
                rowcov &= col;
            }
            std::cerr << " ";
            solutions &= rowcov;
            // GC
            forest->registerFunc(solutions);
            forest->markAllFuncs();
            forest->markSweep();
            forest->deregisterFunc(solutions);
        }
        std::cerr << std::endl;
        watch2.note_time();
        int align = 36;
        if (isLog) {
            std::cerr << "**************** Process Report ****************" << std::endl;
            std::cerr << std::left << std::setw(align) << "Built n queens constraint, took: " << watch1.get_last_seconds() << " seconds" << std::endl;
            std::cerr << std::left << std::setw(align) << "Added constraints, took: " << watch2.get_last_seconds() << " seconds" << std::endl;
            std::cerr << "------------------------------------------------" << std::endl;
        }
        if (!((solutions).getEdge().isConstantZero())) {
            std::cerr << "******************** Report ********************" << std::endl;
            long num = 0;
            apply(CARDINALITY, solutions, num);
            std::cerr << "There are " << num << " covers (including all symmetries) " << std::endl;
            std::cerr << "with minimum required queens " << n << std::endl;
            return 0;
        } else {
            std::cerr << "No solutions\n" << std::endl;
        }
    }
    /* delete Forest */
    delete forest;
}