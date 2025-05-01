/* This is an example of building the set of configurations (states) reachable to target configure for
 * user-specified sliding puzzle.
 * 
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;

// The size of the puzzle
uint16_t N;     // row
uint16_t M;     // column
uint16_t bits;  // bits per position

using PuzzleState = std::vector<std::vector<uint16_t> >; // 2D representation of the puzzle

// Timers
timer watchChain, watchBFS, watchBFS0;
double timeLimit = 1800.0;
bool isTimeLimitGlobal = 0;


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
        result &= ((varTo & zero) | (!varTo & !zero));    // equivalence interface for performance TBD
    }

    /* Firing */
    // sliding tile at position FROM to position TO: X'_to = X_from, X'_from = 0
    for (uint16_t i=0; i<bits; i++) {
        varTo.variable(startTo+i, 1);
        varFrom.variable(startFrom+i, 0);
        result &= ((varTo & varFrom) | (!varTo & !varFrom));  // equivalence interface for performance TBD
        varFrom.variable(startFrom+i, 1);
        result &= ((varFrom & zero) | (!varFrom & !zero));    // equivalence interface for performance TBD
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
    Func nextStep(forest1);
    Func FF(forest1);
    FF.falseFunc();
    Func SS = FF;
    unsigned long n = 0;
    long card_SS = 0, card_curr = 0;
    while (true) {
        for (size_t i=0; i<relations.size(); i++) {
            std::cout << "BFS image process: " << n << " : (" << i << "/" << relations.size() << ")" << std::endl;
            apply(POST_IMAGE, curr, relations[i], nextStep);
            FF |= nextStep;
        }
        // if (!testImage(curr, relations, FF)) {
        //     std::cout << "image test failed\n";
        //     exit(0);
        // }
        n++;
        SS = curr | FF;

        apply(CARDINALITY, SS, card_SS);
        apply(CARDINALITY, curr, card_curr);
        std::cout << "BFS SS ";
        SS.getEdge().print(std::cout);
        std::cout << " card: " << card_SS << std::endl;
        std::cout << "BFS curr ";
        curr.getEdge().print(std::cout);
        std::cout << " card: " << card_curr << std::endl;
        std::cout << "BFS SS num: " << forest1->getNodeManUsed(SS) << std::endl;
        std::cout << "BFS curr num: " << forest1->getNodeManUsed(curr) << std::endl;
        // check fixpoint
        if (SS == curr) break;
        curr = SS;
        // GC
        forest1->registerFunc(curr);
        std::cout << "forest1 mark nodes; num funcs: " << forest1->numFuncs() << std::endl;
        forest1->markAllFuncs();
        std::cout << "forest1 mark and sweep: " << std::endl;
        forest1->markSweep();
        forest1->deregisterFunc(curr);
        std::cout << "forest2 mark nodes; num funcs: " << forest2->numFuncs() << std::endl;
        forest2->markAllFuncs();
        std::cout << "forest2 mark and sweep: " << std::endl;
        forest2->markSweep();

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
            apply(POST_IMAGE, curr, relations[i], nextStep);
            curr |= nextStep;
        }
        n++;
        apply(CARDINALITY, SS, card_SS);
        apply(CARDINALITY, curr, card_curr);
        std::cout << "SS card: " << card_SS << std::endl;
        std::cout << "curr card: " << card_curr << std::endl;
        std::cout << "SS num: " << forest1->getNodeManUsed(SS) << std::endl;
        std::cout << "curr num: " << forest1->getNodeManUsed(curr) << std::endl;

        if (SS == curr) break;
        SS |= curr;
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
        row = (i%M==0) ? i/M-1 : i/M;
        col = (i%M==0) ? M-1 : i%M-1;
        for (uint16_t b=0; b<bits; b++) {
            var.variable((i-1)*bits+b+1);
            if (puzzle[row][col] & (0x01<<b)) {
                result &= var;
            } else {
                result &= !var;
            }
        }
    }
    return result;
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
    long card = 0;
    apply(CARDINALITY, states, card);
    std::cout << "correct answer: ";
    states.getEdge().print(std::cout);
    std::cout << std::endl;
    std::cout << "\tnumber of nodes: " << forest1->getNodeManUsed(states) << std::endl;
    std::cout << "\tnumber of states: " << num << std::endl;
    std::cout << "\tnumber of states(card): " << card << std::endl;
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
            // option for time out
            if (strcmp("-t", argv[i])==0) {
                timeLimit = std::stod(argv[i+1]);
                i++;
                continue;
            }
            if (strcmp("-tg", argv[i])==0) {
                isTimeLimitGlobal = 1;
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
    bits = (const uint16_t)std::ceil(log2(N*M));
    std::cout << "bits: " << bits << std::endl;
    uint16_t levels = bits * N * M;
    ForestSetting setting1(setType, levels);
    ForestSetting setting2(relType, levels);
    forest1 = new Forest(setting1);
    forest2 = new Forest(setting2);
    /* Encode final target configure to BDD */
    Func target(forest1);
    target = encodePuzzle2BDD(conf);
    forest1->registerFunc(target);
    // std::cout << "encode puzzle done" << std::endl;
    // std::cout << "\tnumber of nodes: " << forest1->getNodeManUsed(target) << std::endl;
    long num = 0;
    // apply(CARDINALITY, target, num);
    // std::cout << "\tnumber of states: " << num << std::endl;
    /* Build forward functions */
    // each position takes 4 slots for down, up, left, right
    // some may be constant 0 because of invalid forward direction
    Func forward(forest2);
    std::vector<Func> relations(4*M*N, Func(forest2));
    for (int position=1; position<=N*M; position++) {
        for (int direction = 1; direction<=4; direction++) {
            std::cout << "trans: position = " << position  << " direction: " << direction << std::endl;
            forward = trans(position, direction);
            relations[4*(position-1)+direction-1] = forward;
            forest2->registerFunc(forward);
            std::cout << "trans done, number of nodes: " << forest2->getNodeManUsed(relations[4*(position-1)+direction-1]) << std::endl;
            forward.getEdge().print(std::cout);
            std::cout << std::endl;
        }
    }
    // evaluate forward relation
    if (!evalTrans(relations)) {
        exit(0);
    }

    /* flags if results are computed */
    bool getRes_BFS = 0, getRes_chain = 0, getRes_correct = 0;
    /* time (sec) of each method */
    double time_BFS = 0.0, time_chain = 0.0, time_correct = 0.0, newTimeLimit = timeLimit;
    /* #state of each methods */
    long state_BFS = 0, state_chain = 0, state_correct = 0;
    /* #nodes of each methods */
    uint64_t nodes_BFS = 0, nodes_chain = 0, nodes_correct = 0;

    /* Compute state space using saturation */

    /* Compute state space using chain search */
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

    // forest1->registerFunc(states_chain);
    // reset the forest
    forest1->markAllFuncs();
    forest1->markSweep();

    /* Update time limit */
    if (!isTimeLimitGlobal) newTimeLimit = time_chain;

    /* Compute state space using BFS */
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

    // forest1->registerFunc(states_BFS);
    // reset the forest
    forest1->markAllFuncs();
    forest1->markSweep();

    /* Compute state space using BFS without image */
    Func states(forest1);
    watchBFS0.reset();
    watchBFS0.note_time();
    getRes_correct = (N>2 || M>2) ? SSG(0, states, newTimeLimit) : SSG(1, states, (isTimeLimitGlobal) ? timeLimit : newTimeLimit);
    watchBFS0.note_time();
    // record time
    time_correct = watchBFS0.get_last_seconds();
    // record #states
    apply(CARDINALITY, states, state_correct);
    // record #nodes
    nodes_correct = forest1->getNodeManUsed(states);


    // check with the correct answer
    if ((getRes_BFS && getRes_correct) && (states.getEdge() != states_BFS.getEdge())) {
        std::cout << "[SSG Failed]: BFS is different from the correct!" << std::endl;
        std::cout << "\tcorrect answer: ";
        states.getEdge().print(std::cout);
        std::cout << std::endl;
        std::cout << "\tBFS answer: ";
        states_BFS.getEdge().print(std::cout);
        std::cout << std::endl;
        // check if encode the same function
        std::cout << "Checking if they're encoding different functions!" << std::endl;
        num = N*M*bits;
        std::vector<bool> assign(num+1, 0);
        bool isDiff = 0;
        for (long i=0; i<(1<<num); i++) {
            for (uint16_t k=1; k<=num; k++) {
                assign[k] = i&(1<<(k-1));
            }
            int valIntS, valIntBFS;
            Value evalS = states.evaluate(assign);
            Value evalBFS = states_BFS.evaluate(assign);
            evalS.getValueTo(&valIntS, INT);
            evalBFS.getValueTo(&valIntBFS, INT);
            if (valIntS != valIntBFS) {
                isDiff = 1;
                break;
            }
        }
        if (isDiff) {
            std::cout << "\tDifferent functions!" << std::endl;
        } else {
            std::cout << "\tSame functions!" << std::endl;
        }
        // graph
        DotMaker dot0(forest1, "states");
        dot0.buildGraph(states);
        dot0.runDot("pdf");

        DotMaker dot1(forest1, "states_BFS");
        dot1.buildGraph(states_BFS);
        dot1.runDot("pdf");
    }

    if ((getRes_chain && getRes_correct) && (states.getEdge() != states_chain.getEdge())) {
        std::cout << "[SSG Failed]: chain is different from the correct!" << std::endl;
        std::cout << "\tcorrect answer: ";
        states.getEdge().print(std::cout);
        std::cout << std::endl;
        std::cout << "\tchain answer: ";
        states_chain.getEdge().print(std::cout);
        std::cout << std::endl;
        // check if encode the same function
        std::cout << "Checking if they're encoding different functions!" << std::endl;
        num = N*M*bits;
        std::vector<bool> assign(num+1, 0);
        bool isDiff = 0;
        for (long i=0; i<(1<<num); i++) {
            for (uint16_t k=1; k<=num; k++) {
                assign[k] = i&(1<<(k-1));
            }
            int valIntS, valIntChain;
            Value evalS = states.evaluate(assign);
            Value evalChain = states_chain.evaluate(assign);
            evalS.getValueTo(&valIntS, INT);
            evalChain.getValueTo(&valIntChain, INT);
            if (valIntS != valIntChain) {
                isDiff = 1;
                break;
            }
        }
        if (isDiff) {
            std::cout << "\tDifferent functions!" << std::endl;
        } else {
            std::cout << "\tSame functions!" << std::endl;
        }
        // graph
        DotMaker dot0(forest1, "states");
        dot0.buildGraph(states);
        dot0.runDot("pdf");

        DotMaker dot1(forest1, "states_chain");
        dot1.buildGraph(states_chain);
        dot1.runDot("pdf");
    }

    /* evaluate these results */
    if ((getRes_BFS && getRes_chain) && states_BFS.getEdge() != states_chain.getEdge()) {
        std::cout << "[SSG Failed]: BFS and chain are different!" << std::endl;
        exit(0);
    }
    std::cout << "Done!" << std::endl;

    /* report time */
    std::cout << "=========================| Time |=========================" << std::endl;
    if (getRes_chain) {
        std::cout << "chain: \t\t\t" << time_chain << " seconds" << std::endl;
    } else {
        std::cout << "chain: \t\t\tTIME OUT" << std::endl;
    }
    if (getRes_BFS) {
        std::cout << "BFS: \t\t\t" << time_BFS << " seconds" << std::endl;
    } else {
        std::cout << "BFS: \t\t\tTIME OUT" << std::endl;
    }
    if (getRes_correct) {
        std::cout << "BFS (w/o image): \t" << time_correct << " seconds" << std::endl;
    } else {
        std::cout << "BFS (w/o image): \tTIME OUT" << std::endl;
    }
    std::cout << "\t\t\t\t[TIME OUT: " << newTimeLimit << " seconds]" << std::endl;

    /* report expored #states */
    std::cout << "=======================| #States |========================" << std::endl;
    std::cout << "chain: \t\t\t" << state_chain << std::endl;
    std::cout << "BFS: \t\t\t" << state_BFS << std::endl;
    std::cout << "BFS (w/o image): \t" << state_correct << std::endl;
    
    /* report nodes */
    std::cout << "=========================| Node |=========================" << std::endl;
    std::cout << "chain: \t\t\t" << nodes_chain << std::endl;
    std::cout << "BFS: \t\t\t" << nodes_BFS << std::endl;
    std::cout << "BFS (w/o image): \t" << nodes_correct << std::endl;

    /* Compute state space using saturation */
    // Func states(forest1);
    // states = saturate(target, forwards);
    /* Compute distance */
    // TBD
    delete forest1;
    delete forest2;
    return 0;
}