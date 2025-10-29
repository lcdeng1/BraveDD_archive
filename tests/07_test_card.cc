#include "brave_dd.h"

#include "cstdlib"
#include "cstdio"

long seed =123456789;

using namespace BRAVE_DD;

/* Random function generating value between 0 and 1 */
double random01()
{
  const long MODULUS = 2147483647L;
  const long MULTIPLIER = 48271L;
  const long Q = MODULUS / MULTIPLIER;
  const long R = MODULUS % MULTIPLIER;

  long t = MULTIPLIER * (seed % Q) - R * (seed / Q);
  if (t > 0) {
    seed = t;
  } else {
    seed = t + MODULUS;
  }
  return ((double) seed / MODULUS);
}

void decimalToAssignment(long long decimal, std::vector<bool>& assignment)
{
    for (size_t k=1; k<=assignment.size()-1; k++) {
        assignment[k] = decimal & (1<<(k-1));
    }
}

Edge buildSetEdge(Forest* forest,
                uint16_t lvl,
                std::vector<bool>& fun,
                int start, int end)
{
    std::vector<Edge> child(2);
    EdgeLabel label = 0;
    packRule(label, RULE_X);
    if (lvl == 1) {
        child[0].setEdgeHandle(makeTerminal(INT, fun[start]?1:0));
        if (forest->getSetting().getValType() == FLOAT) {
            child[0].setEdgeHandle(makeTerminal(FLOAT, fun[start]?1.0f:0.0f));
        }
        child[1].setEdgeHandle(makeTerminal(INT, fun[end]?1:0));
        if (forest->getSetting().getValType() == FLOAT) {
            child[1].setEdgeHandle(makeTerminal(FLOAT, fun[end]?1.0f:0.0f));
        }
        child[0].setRule(RULE_X);
        child[1].setRule(RULE_X);
        return forest->reduceEdge(lvl, label, lvl, child);
    }
    child[0] = buildSetEdge(forest, lvl-1, fun, start, start+(1<<(lvl-1))-1);
    child[1] = buildSetEdge(forest, lvl-1, fun, start+(1<<(lvl-1)), end);
    return forest->reduceEdge(lvl, label, lvl, child);
}

Edge buildRelEdge(Forest* forest,
    uint16_t lvl,
    std::vector<bool>& fun,
    int start, int end)
{
std::vector<Edge> child(4);
EdgeLabel label = 0;
packRule(label, RULE_X);
if (lvl == 1) {
child[0].setEdgeHandle(makeTerminal(INT, fun[start]?1:0));
if (forest->getSetting().getValType() == FLOAT) {
child[0].setEdgeHandle(makeTerminal(FLOAT, fun[start]?1.0f:0.0f));
}
child[1].setEdgeHandle(makeTerminal(INT, fun[start+1]?1:0));
if (forest->getSetting().getValType() == FLOAT) {
child[1].setEdgeHandle(makeTerminal(FLOAT, fun[start+1]?1.0f:0.0f));
}
child[2].setEdgeHandle(makeTerminal(INT, fun[start+2]?1:0));
if (forest->getSetting().getValType() == FLOAT) {
child[2].setEdgeHandle(makeTerminal(FLOAT, fun[start+2]?1.0f:0.0f));
}
child[3].setEdgeHandle(makeTerminal(INT, fun[end]?1:0));
if (forest->getSetting().getValType() == FLOAT) {
child[3].setEdgeHandle(makeTerminal(FLOAT, fun[end]?1.0f:0.0f));
}
child[0].setRule(RULE_X);
child[1].setRule(RULE_X);
child[2].setRule(RULE_X);
child[3].setRule(RULE_X);
return forest->reduceEdge(lvl, label, lvl, child);
}
int end0 = start + (1<<(2*lvl-2)) - 1;
int end1 = end0 + (1<<(2*lvl-2));
int end2 = end1 + (1<<(2*lvl-2));
child[0] = buildRelEdge(forest, lvl-1, fun, start, end0);
child[1] = buildRelEdge(forest, lvl-1, fun, end0+1, end1);
child[2] = buildRelEdge(forest, lvl-1, fun, end1+1, end2);
child[3] = buildRelEdge(forest, lvl-1, fun, end2+1, end);
return forest->reduceEdge(lvl, label, lvl, child);
}

bool testOperation(uint16_t num, PredefForest bdd, BinaryOperationType opt)
{
    // forest setting
    ForestSetting setting(bdd, num);
    Forest* forest = new Forest(setting);
    bool isRel = setting.isRelation();
    // functions
    Func f1(forest);
    // truth table
    long long size = (isRel) ? 0x01LL<<(2*num) : 0x01LL<<(num);
    std::vector<bool> fun1(size);
    // countings
    long long countOne1 = 0;
    for (long long i=0; i<size; i++) {
        fun1[i] = (random01() > 0.5f)? 1 : 0;
        if (fun1[i]) countOne1++;
    }
    std::cout << "Ones counting:" << std::endl;
    std::cout << "Func: " << countOne1 << ";" << std::endl;

    // build edges
    Edge e1;
    if (isRel) {
        e1 = buildRelEdge(forest, num, fun1, 0, size-1);
    } else {
        e1 = buildSetEdge(forest, num, fun1, 0, size-1);
    }
    
    std::cout << "Func 1: " << std::endl;
    e1.print(std::cout);
    std::cout << std::endl;

    // call apply, and check results
    f1.setEdge(e1);
    
    long card = 0;
    apply(CARDINALITY, f1, card);

    std::cout << "result (card): " << card << std::endl;

    // evaluation
    bool isPass = 1;
    if (card != countOne1) {
        std::cout << "result evaluation failed!" << std::endl;
        isPass = 0;
    }
    if (!isPass && (num <=6)) {
        std::cout << std::endl;
        std::cout << "Correct: " << countOne1 << "; result: " << card << std::endl;
    }
    if (!isPass) {
        if (num < 6) {
            DotMaker dot1(forest, "func1");
            dot1.buildGraph(f1);
            dot1.runDot("pdf");
        }
    }
    delete forest;
    return isPass;
}

int main(int argc, char** argv){
    if (argc == 1) {
      printf("Usage: ./test_card [PredefForest::bdd] [num_val]\n");
      printf("\tThis will randomly generate a boolean functions to test card\n");
      exit(0);
    }
    int TESTS = 1000;
    uint16_t numVals = 10;
    PredefForest bdd = PredefForest::REXBDD;
    BinaryOperationType opt = BinaryOperationType::BOP_UNION;
    if ((argc == 3) || (argc == 4)) {
        bdd = (PredefForest)atoi(argv[1]);
        numVals = atoi(argv[2]);
        if (argc == 4) TESTS = atoi(argv[3]);
    }
    
    // forest setting
    ForestSetting setting(bdd, numVals);
    setting.output(std::cout);

    bool isPass = 0;
    // Randomly generate assignments and build two BDDs
    for (int test=0; test<TESTS; test++) {
        isPass = testOperation(numVals, bdd, opt);
        if (!isPass) break;
    }

    if (!isPass) {
        std::cout << "Test Failed!" << std::endl;
    } else {
        std::cout << "Test Pass!" << std::endl;
    }
    return 0;
}