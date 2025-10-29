/* 
 * -----------------------------------------------------------------------------
 *  N-Dining Philosophers Reachability using Binary Decision Diagrams (BDDs)
 * -----------------------------------------------------------------------------
 *  Overview:
 *  This program explores the reachable states of the classic N-Dining Philosophers
 *  problem modeled as a *safe Petri net* using Binary Decision Diagrams (BDDs).
 *
 *  In the N-Dining Philosophers problem, N philosophers sit at a round table,
 *  each alternating between idling and eating, and each requiring two forks
 *  (resources) to eat. In the safe Petri net model:
 *      - Each *place* represents a local state (e.g., Idle, WaitLeft, HasLeft,
 *        WaitRight, HasRight, or fork availability).
 *      - Each place is *1-bounded*, i.e., it holds at most one token.
 *  Transitions capture the acquisition and release of forks and the progression
 *  of philosophers’ states.
 *
 *  This program encodes the Petri net marking as a Boolean vector:
 *      - Each place is represented by one Boolean variable.
 *      - A value of 1 indicates a token is present, 0 indicates empty.
 *  The BDD represents the set of all reachable markings symbolically.
 *  The variable ordering used in this program is (bottom < top):
 *      - For each philosopher i: 
 *          Fork_i < Idle_i < WaitLeft_i < HasLeft_i < WaitRight_i < HasRight_i
 *      - Overall:
 *          Philosopher_1 < Philosopher_2 < ... < Philosopher_N
 *
 *  Users can specify:
 *    - The number of philosophers (N)
 *    - The predefined BDD type to encode states
 *        (e.g., FBDD, CFBDD, SFBDD, CSFBDD, ZBDD, ESRBDD, CESRBDD, or REXBDD)
 * 
 * Author: Lichuan Deng
 * Version: 1.0
 * Last Update Date: Oct 27, 2025
 */

#include "brave_dd.h"
#include "Fibonacci_fast.h"
#include "timer.h"

using namespace BRAVE_DD;

// Flag to output help info
bool isHelp = 0;
// Flag to output log
bool isLog = 0;
// The size of board
int N = 3;
// BDD type
std::string bdd_type = "FBDD";
std::string bmxd_type = "ESRBMxD";
// BDD forest
Forest* bdd_forest;
Forest* bmxd_forest;
// time
double time_initial = 0.0, time_transition = 0.0, time_explore = 0.0;

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
    out << std::left << std::setw(2*align) << "  -n <number>" << "Set the number of philosophers. Default: 3" << std::endl;
    out << std::left << std::setw(2*align) << "  -type <string>" << "Select the predefined BDD type. Default: FBDD" << std::endl;
    out << std::left << std::setw(2*align) << "" << "Supported: FBDD, CFBDD, SFBDD, CSFBDD, ZBDD, ESRBDD, CESRBDD, REXBDD" << std::endl;
    out << std::left << std::setw(2*align) << "  -log, -l " << "Display help message and usage" << std::endl;
    out << std::endl;
    out << std::left << std::setw(align) << "EXAMPLES: "<< std::endl;
    out << std::left << std::setw(align) << "" << name << " -n 10" << std::endl;
    out << std::left << std::setw(align) << "" << name << " -n 10 -type zbdd" << std::endl;
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
    This program symbolically explores the reachable state space of the
    N-Dining Philosophers problem using Binary Decision Diagrams (BDDs).
    )"
    << std::endl;
    out << "----------------------------------------------------------" << std::endl;
    return usage(who);
}

/* Parse the arguments */
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

/* Build the transition relations 
 * Each philosopher has 4 transitions:
 *  0: "Release", 1: "GoEat", 2: "GetLeft", 3: "GetRight"
 */
Func transition(const int phil, const int trans)
{
    // helpers
    Level numVars = 6 * N;
    Func const_zero(bmxd_forest), const_one(bmxd_forest);
    // const_zero.constant(0);
    // const_one.constant(1);
    const_zero.falseFunc();
    const_one.trueFunc();
    std::vector<bool> dependance(numVars+1, 0);
    Func var_from(bmxd_forest);
    Func var_to(bmxd_forest);
    // the result
    Func ans(bmxd_forest);
    // ans.constant(1);
    ans.trueFunc();
    // build
    if (trans == 0) {
        /* "Release" */
        // Enabling: X_hasL == 1, X_hasR == 1
        var_from.variable(6*phil + 4, 0);
        ans &= ((var_from & const_one) | ((!var_from) & (!const_one)));
        var_from.variable(6*phil + 6, 0);
        ans &= ((var_from & const_one) | ((!var_from) & (!const_one)));
        // Firing: Fork'_n == 1, Fork'_{n+1} == 1, X'_idle == 1 X'_hasL == 0, X'_hasR == 0
        var_to.variable(6*(phil%N)+1, 1);
        ans &= ((var_to & const_one) | ((!var_to) & (!const_one)));
        var_to.variable(6*((phil+1)%N)+1, 1);
        ans &= ((var_to & const_one) | ((!var_to) & (!const_one)));
        var_to.variable(6*phil + 2, 1);
        ans &= ((var_to & const_one) | ((!var_to) & (!const_one)));
        var_to.variable(6*phil + 4, 1);
        ans &= ((var_to & const_zero) | ((!var_to) & (!const_zero)));
        var_to.variable(6*phil + 6, 1);
        ans &= ((var_to & const_zero) | ((!var_to) & (!const_zero)));
        // Dependance
        dependance[6*phil + 2] = 1;
        dependance[6*phil + 4] = 1;
        dependance[6*phil + 6] = 1;
        dependance[6*(phil%N)+1] = 1;
        dependance[6*((phil+1)%N)+1] = 1;
    } else if (trans == 1) {
        /* "GoEat" */
        // Enabling: X_idle == 1
        var_from.variable(6*phil + 2, 0);
        ans &= ((var_from & const_one) | ((!var_from) & (!const_one)));
        // Firing: X'_waitL == 1, X'_waitR == 1, X'_idle == 0
        var_to.variable(6*phil + 3, 1);
        ans &= ((var_to & const_one) | ((!var_to) & (!const_one)));
        var_to.variable(6*phil + 5, 1);
        ans &= ((var_to & const_one) | ((!var_to) & (!const_one)));
        var_to.variable(6*phil + 2, 1);
        ans &= ((var_to & const_zero) | ((!var_to) & (!const_zero)));
        // Dependance
        dependance[6*phil + 2] = 1;
        dependance[6*phil + 3] = 1;
        dependance[6*phil + 5] = 1;
    } else if (trans == 2) {
        /* "GetLeft" */
        // Enabling: Fork_n == 1, X_waitL == 1
        var_from.variable(6*(phil%N)+1, 0);
        ans &= ((var_from & const_one) | ((!var_from) & (!const_one)));
        var_from.variable(6*phil + 3, 0);
        ans &= ((var_from & const_one) | ((!var_from) & (!const_one)));
        // Firing: X'+hasL == 1, Fork'_n == 0, X'_waitL == 0
        var_to.variable(6*phil + 4, 1);
        ans &= ((var_to & const_one) | ((!var_to) & (!const_one)));
        var_to.variable(6*(phil%N)+1, 1);
        ans &= ((var_to & const_zero) | ((!var_to) & (!const_zero)));
        var_to.variable(6*phil + 3, 1);
        ans &= ((var_to & const_zero) | ((!var_to) & (!const_zero)));
        // Dependance
        dependance[6*phil + 3] = 1;
        dependance[6*phil + 4] = 1;
        dependance[6*(phil%N)+1] = 1;
    } else if (trans == 3) {
        /* "GetRight" */
        // Enabling: Fork_{n+1} == 1, X_waitR == 1
        var_from.variable(6*((phil+1)%N)+1, 0);
        ans &= ((var_from & const_one) | ((!var_from) & (!const_one)));
        var_from.variable(6*phil + 5, 0);
        ans &= ((var_from & const_one) | ((!var_from) & (!const_one)));
        // Firing: X'_hasR == 1, Fork'_{n+1} == 0, X'_waitR == 0
        var_to.variable(6*phil + 6, 1);
        ans &= ((var_to & const_one) | ((!var_to) & (!const_one)));
        var_to.variable(6*((phil+1)%N)+1, 1);
        ans &= ((var_to & const_zero) | ((!var_to) & (!const_zero)));
        var_to.variable(6*phil + 5, 1);
        ans &= ((var_to & const_zero) | ((!var_to) & (!const_zero)));
        // Dependance
        dependance[6*phil + 5] = 1;
        dependance[6*phil + 6] = 1;
        dependance[6*((phil+1)%N)+1] = 1;
    } else {
        std::cerr << "unknown transition!" << std::endl;
        ans.constant(0);
        return ans;
    }
    /* Identities */
    const_one.identity(dependance);

    return ans & const_one;
}

int main(int argc, const char** argv)
{
    int align = 36;
    /* Process Args */
    if (processArgs(argc, argv)) {
        if (isHelp) return helpInfo(argv[0]);
        return usage(argv[0]);
    }

    /* Initialize BraveDD forest */
    Level numVars = 6 * N;
    ForestSetting setting1(bdd_type, numVars);   // Set the BDD type and the number of variables
    ForestSetting setting2(bmxd_type, numVars);   // Set the BMXD type and the number of variables
    bdd_forest = new Forest(setting1);
    bmxd_forest = new Forest(setting2);
    std::cerr << "Using " << getLibInfo(0) << std::endl;
    std::cerr << "Exploring Reachability for " << N << std::left << std::setw(27) << "-Dining Philosophers in" << std::endl;
    std::cerr << std::left << std::setw(align) << "BDD type: " << bdd_forest->getSetting().getName() << std::endl;
    std::cerr << std::left << std::setw(align) << "BMXD type: " << bmxd_forest->getSetting().getName() << std::endl;
    /* Expexted number of reachable states */
    std::cerr << std::left << std::setw(align) << "Expecting number of states: " << (fib(3*N+1) + fib(3*N-1)) << std::endl;
    if (N > 30) {
        std::cerr << std::left << std::setw(align) << "##[Warning]: Exceeding the range of display data, but still exploring! GMP support coming soon...##" << std::endl;
    }

    /* Timer */
    timer watch;
    watch.reset();

    /* Create the initial state */
    Func initial(bdd_forest);
    // initial.constant(1);
    initial.trueFunc();
    watch.note_time();
    for (int i=0; i<N; i++) {
        for (int p=1; p<=6; p++) {
            Func place(bdd_forest);
            place.variable(6*i + p);
            if ((p != 1) && (p != 2)) { // fork and idle
                place = !place;
            }
            initial &= place;
        }
    }
    watch.note_time();
    time_initial = watch.get_last_seconds();
    if (isLog) {
        std::cerr << "**************** Process Report ****************" << std::endl;
        std::cerr << std::left << std::setw(align) << "Built the initial state, took " << time_initial << " seconds" << std::endl;
    }


    /* Create the transition relations 
     *  Each philosopher has 4 transitions:
     *  "Release", "GoEat", "GetLeft", "GetRight"
     */
    watch.reset();
    watch.note_time();
    std::vector<Func> transitions(4*N, Func(bmxd_forest));
    for (int i=0; i<N; i++) {
        for (int t=0; t<4; t++) {
            // std::cerr << "build transition " << t << " for philosopher " << i << std::endl;
            transitions[4*i + t] = transition(i, t);
        }
    }
    watch.note_time();
    time_transition = watch.get_last_seconds();
    if (isLog) {
        std::cerr << "**************** Process Report ****************" << std::endl;
        std::cerr << std::left << std::setw(align) << "Built transition relations, took " << time_transition << " seconds" << std::endl;
    }

    /* Explore reachable states */
    watch.reset();
    watch.note_time();
    Func reachables(bdd_forest);
    apply(SATURATE, initial, transitions, reachables);
    // SSG_chain(initial, transitions, reachables);
    /* Timer end */
    watch.note_time();
    time_explore = watch.get_last_seconds();
    if (isLog) {
        std::cerr << "**************** Process Report ****************" << std::endl;
        std::cerr << std::left << std::setw(align) << "Built reachability set, took " << time_explore << " seconds" << std::endl;
    }

    /* Report */
    long num;
    apply(CARDINALITY, reachables, num);
    std::cerr << "******************** Report ********************" << std::endl;
    std::cerr << std::left << std::setw(align) << "Number of reachable states: " << num << std::endl;
    std::cerr << std::left << std::setw(align) << "Elapsed time (seconds): " << (time_initial + time_transition + time_explore) << std::endl;
    std::cerr << std::left << std::setw(align) << "Number of nodes (final): " << bdd_forest->getNodeManUsed(reachables) << std::endl;
    std::cerr << std::left << std::setw(align) << "Number of nodes (peak): " << bdd_forest->getNodeManPeak() << std::endl;

    /* delete Forest */
    delete bdd_forest;
    delete bmxd_forest;
}