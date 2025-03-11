/* 
 * This is an example of building the set of solutinos to the N-Queens problem for user-specified N.
 *
 * It will find all possible ways to put N queens onto an NxN chessboard so that no queen is attacking 
 * any other queen. We can use Boolean variable x_ij to represent whether a queen is placed at row 'i' 
 * and column 'j'. This is a Boolean variable that is true if there is a queen at that position.
 * 
 * The constraints for the N-Queens problem are as follows:
 *      1. Row constraints: Each row must have exactly one queen
 *      2. Column constraints: Each column must have exactly one queen
 *      3. Diagonal constraints: No two queens can be placed on the same diagonal
 */

#include <cstdio>
#include <cmath>
#include <vector>
#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;

// The size of board
int N;
// BDD type
std::string bdd_type;

bool processArgs(int argc, const char** argv)
{
    bool setN = 0;
    bdd_type = "RexBDD";
    for (int i=1; i<argc; i++) {
        if ('-' == argv[i][0]) {
            // options other than size
            if (strcmp("-t", argv[i])==0) {
                bdd_type = argv[i+1];
                i++;
                continue;
            }
        }
        if (setN) continue;
        N = atoi(argv[i]);
        setN = true;
    }

    if (!setN || N<1) return 0;
    return 1;
}

int usage(const char* who)
{
    /* Strip leading directory, if any: */
    const char* name = who;
    for (const char* ptr=who; *ptr; ptr++) {
        if ('/' == *ptr) name = ptr+1;
    }
    std::cout << "Usage: " << name << " [options] N\n" << std::endl;
    return 1;
}

Func rowConstraint(const std::vector<std::vector<Func> >& board, int row)
{
    /* Start in the same forest */
    Func constraint1(board[0][0].getForest());  // at most one 
    Func constraint2(board[0][0].getForest());  // at least one
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
    Func constraint1(board[0][0].getForest());  // at most one 
    Func constraint2(board[0][0].getForest());  // at least one
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
    Func constraint(board[0][0].getForest());
    constraint.trueFunc();

    /* No two queens in the same diagonal and anti-diagonal */
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            // check diagonals in both directions
            // diagonal
            for (int r = i + 1, c = j + 1; r < N && c < N; ++r, ++c) {
                constraint &= !(board[i][j] & board[r][c]);
            }
            // anti-diagonal
            for (int r = i + 1, c = j - 1; r < N && c >= 0; ++r, --c) {
                constraint &= !(board[i][j] & board[r][c]);
            }
        }
    }
    return constraint;
}

int main(int argc, const char** argv)
{
    /* Process Args */
    if (!processArgs(argc, argv)) return usage(argv[0]);
    std::cout << "Solving " << N << "-Queens in BDD type: " << bdd_type << std::endl;
    std::cout << "Using " << getLibInfo(0) << std::endl;

    /* Timer start */
    timer watch;
    watch.note_time();

    /* Initialize BraveDD forest */
    ForestSetting setting(bdd_type, N*N);   // Set the BDD type and the number of variables
    Forest* forest = new Forest(setting);
    setting.output(std::cout);

    /* Create BDD variables for each cell on the board */
    std::vector<std::vector<Func> > board(N, std::vector<Func>(N, Func(forest)));
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            board[i][j].variable(i * N + j + 1);
        }
    }

    /* Row, Column, and Diagonal constraint */
    Func solution(board[0][0].getForest());
    solution.trueFunc();
    for (int i=0; i<N; i++) {
        solution &= rowConstraint(board, i);
        solution &= colConstraint(board, i);
    }
    solution &= diagConstraint(board);

    /* Timer end */
    watch.note_time();
    std::cout << "Elapsed time: " << watch.get_last_seconds() << " seconds" << std::endl;

    /* Output */
    long num;
    apply(CARDINALITY, solution, num);
    std::cout << "Number of solutions: " << num << std::endl;

    /* delete Forest */
    delete forest;
    return 0;
}