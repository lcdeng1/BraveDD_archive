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
    // out << std::left << std::setw(align) << "OVERVIEW: " << "N-Queens Problem" << std::endl;
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

Func attackConstraint(const std::vector<std::vector<Func> >& board, int row, int col)
{
    /* Start the constraint */
    Func constraint = board[row][col];
    Func rowAttack(forest);
    rowAttack.falseFunc();
    Func colAttack(forest);
    colAttack.falseFunc();
    Func diaAttack(forest);
    diaAttack.falseFunc();
    Func atdiaAttack(forest);
    atdiaAttack.falseFunc();

    /* Be attacked by the same row */
    for (int j=0; j<col; j++) {
        rowAttack |= board[row][j];
    }
    for (int j=col+1; j<N; j++) {
        rowAttack |= board[row][j];
    }
    /* Be attacked by the same column */
    for (int i=0; i<row; i++) {
        colAttack |= board[i][col];
    }
    for (int i=row+1; i<N; i++) {
        colAttack |= board[i][col];
    }
    /* Be attacked by the same diagonal */
    for (int i=row-1, j=col-1; i>=0 && j>=0; i--, j--) {
        diaAttack |= board[i][j];
    }
    for (int i=row+1, j=col+1; i<N && j<N; i++, j++) {
        diaAttack |= board[i][j];
    }
    /* Be attacked by the same anti-diagonal */
    for (int i=row+1, j=col-1; i<N && j>=0; i++, j--) {
        atdiaAttack |= board[i][j];
    }
    for (int i=row-1, j=col+1; i>=0 && j<N; i--, j++) {
        atdiaAttack |= board[i][j];
    }
    constraint |= rowAttack;
    constraint |= colAttack;
    constraint |= diaAttack;
    constraint |= atdiaAttack;
    return constraint;
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
    Func var(forest);
    var.variable(L);
    ans = (var & exactNQueens(L-1, n-1)) | ((!var) & exactNQueens(L-1, n));
    return ans;
}

/* Compute the number of choose n from L, used for testing the result of "exactNQueens" */
long choose(unsigned int L, unsigned int n) {
    if (n > L) return 0;
    if (n > L - n) n = L - n; // symmetry
    long res = 1;
    for (unsigned int i = 1; i <= n; ++i) {
        res = res * (L - n + i) / i;
    }
    return res;
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
    /* Timer start */
    timer watch;
    watch.reset();
    watch.note_time();
    /* Row, Column, and Diagonal constraint */
    Func solution(forest);
    solution.trueFunc();
    for (int i=0; i<N; i++) {
        Func rowSolution(forest);
        rowSolution.trueFunc();
        for (int j=0; j<N; j++) {
            rowSolution &= attackConstraint(board, i, j);
        }
        solution &= rowSolution;
        // mark nodes and GC
        forest->registerFunc(solution);
        forest->markAllFuncs();
        forest->markSweep();
        forest->deregisterFunc(solution);
    }
    /* Timer end */
    watch.note_time();
    forest->registerFunc(solution);

    int align = 36;
    if (isLog) {
        std::cerr << "**************** Process Report ****************" << std::endl;
        std::cerr << std::left << std::setw(align) << "Built cover constraints, took: " << watch.get_last_seconds() << " seconds" << std::endl;
        // long num;
        // apply(CARDINALITY, solution, num);
        // std::cerr << std::left << std::setw(align) << "Number of valid solutions: " << num << std::endl;
        std::cerr << std::left << std::setw(align) << "Number of nodes (final): " << forest->getNodeManUsed(solution) << std::endl;
        std::cerr << std::left << std::setw(align) << "Number of nodes (peak): " << forest->getNodeManPeak() << std::endl;
    }

    // check with exact n (from 1 to N) queens constraint
    for (int n=1; n<=N; n++) {
        // GC
        // forest->markAllFuncs();
        // forest->markSweep();
        timer watch1;
        watch1.reset();
        watch1.note_time();
        Func nqueens = exactNQueens(N*N, n);
        watch1.note_time();
        long num = 0;
        // apply(CARDINALITY, nqueens, num);
        // std::cerr << "exact queens: " << n << " size: " << num << std::endl;
        // if (num != choose(N*N, n)) {
        //     std::cerr << "exact N queens error\n" << std::endl;
        //     exit(1);
        // }
        timer watch2;
        watch2.reset();
        watch2.note_time();
        Func min_solution = nqueens & solution;
        watch2.note_time();
        if (isLog) {
            std::cerr << "**************** Process Report ****************" << std::endl;
            std::cerr << "Queens: " << n << std::endl;
            std::cerr << std::left << std::setw(align) << "Built n queens constraints, took: " << watch1.get_last_seconds() << " seconds" << std::endl;
            std::cerr << std::left << std::setw(align) << "Check solutions, took: " << watch2.get_last_seconds() << " seconds" << std::endl;
        }
        if (!((min_solution).getEdge().isConstantZero())) {
            std::cerr << "******************** Report ********************" << std::endl;
            apply(CARDINALITY, min_solution, num);
            std::cerr << "There are " << num << " covers with minimum required queens " << num << std::endl;
            return 0;
        }
    }

    /* delete Forest */
    delete forest;
}