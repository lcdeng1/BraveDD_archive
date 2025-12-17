#include "brave_dd.h"

#include <random>
#include <iostream>
#include <cstdint>
long seed =123456789;

using namespace BRAVE_DD;

std::mt19937 gen(seed);   
std::uniform_int_distribution<uint32_t> distr32(0, 1);
std::uniform_int_distribution<uint32_t> distr32_int(0, 5);

/* Random function generating value between 0 and 1 */
bool random01()
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
  return ((double) seed / MODULUS) > 0.5f;
}

bool recursiveCheck(Forest* forest, Edge root) {
    bool check = false;
    if ((root.getRule() == RULE_OR) || (root.getRule() == RULE_AND)) {
        return true;
    }
    std::vector<Edge> child(2);
    if (root.getNodeLevel() != 0) {
        child[0] = forest->getChildEdge(root.getNodeLevel(), root.getNodeHandle(), 0);
        check = recursiveCheck(forest, child[0]);
        if (check) return check;
        child[1] = forest->getChildEdge(root.getNodeLevel(), root.getNodeHandle(), 1);
        check = recursiveCheck(forest, child[1]);
        if (check) return check;
    }
    return check;
}

Edge buildSetEdge(Forest* forest,
                uint64_t  lvl,
                std::vector<bool>& fun,
                uint64_t start, uint64_t end)
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
    // if (lvl == 3) exit(0);
    return forest->reduceEdge(lvl, label, lvl, child);
}

bool buildSetForest(uint64_t  num, PredefForest bdd)
{
    // test up to 5 variables
    ForestSetting setting(PredefForest::UBDD, num);
    // setting.setSwapType(NO_SWAP);
    // setting.setCompType(NO_COMP);
    Forest* forest = new Forest(setting);
    forest->getSetting().output(std::cout);

    ForestSetting compSetting(PredefForest::REXBDD, num);
    // compSetting.setCompType(COMP);
    // compSetting.setSwapType(ONE);
    Forest* compForest = new Forest(compSetting);
    compForest->getSetting().output(std::cout);

    bool ans = 0;
    // variables
    std::vector<std::vector<bool> > vars(1ULL<<num, std::vector<bool>(num+1, false));
    for (uint64_t i=0; i<(1ULL<<num); i++) {
        for (uint64_t  k=1; k<=num; k++){
            vars[i][k] = i & (1<<(k-1));
        }
    }
    // function values
    Func function(forest);
    Func compFunction(compForest);
    std::vector<std::vector<bool> > funs(1ULL<<(1<<num), std::vector<bool>(1<<num, false));

    uint64_t counta = 0;
    for (uint64_t i=0; i<(1ULL<<(1<<num)); i++) { 
            for (uint64_t k=0; k<(1<<num); k++) {
                funs[i][k] = i & (1<<k); 
            }
            // build function
            Edge edge = buildSetEdge(forest, num, funs[i], 0, (1<<num)-1);
            function.setEdge(edge);
    
            Edge compEdge = buildSetEdge(compForest, num, funs[i], 0, (1<<num)-1);
            compFunction.setEdge(compEdge);
            // evaluate function
            for (uint64_t j=0; j<(1<<num); j++) {
                Value eval = function.evaluate(vars[j]);
                Value compEval = compFunction.evaluate(vars[j]);
                union
                {
                    int valInt;
                    float valFloat;
                };
                union
                {
                    int valInt2;
                    float valFloat2;
                };
                compEval.getValueTo(&valInt2, INT);
                eval.getValueTo(&valInt, INT);
                if (valInt != funs[i][j] || valInt2 != valInt) {
                // if (i == 84) {
                // bool check = recursiveCheck(forest, function.getEdge());
                // if (check) {
                bool condition = forest->getNodeManUsed(function) > compForest->getNodeManUsed(compFunction);
                if (condition) counta++;
                // if (condition) {
                    std::cout << "function: " << i << std::endl;
                    std::cout<<"Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
                    std::cout<<"\t assignment: ";
                    for (uint64_t k=1; k<=num; k++){
                        std::cout << vars[j][k] << " ";
                    }
                    std::cout << "; value shoud be: " << funs[i][j] << "; was: " << valInt << std::endl;
                    std::cout<<"\t full function: ";
                    for (uint64_t k=0; k<(1<<num); k++){
                        std::cout << funs[i][k] << " ";
                    }
                    std::cout << std::endl;
    
                    DotMaker dot(forest);
                    dot.buildGraph(function, "error_function" + std::to_string(i));
                    dot.runDot("error_function" + std::to_string(i), "pdf");
        
                    DotMaker dot1(compForest);
                    dot1.buildGraph(compFunction, "comp_function" + std::to_string(i));
                    dot1.runDot("comp_function" + std::to_string(i), "pdf");
                    // break;
                    return 0;
                }
            }
        }
        
    ans = 1;
    std::cout << "Number of UBDD bigger than REX: " << counta << std::endl;
    for (uint64_t i=1; i<=num; i++) {
        std::cout << "UBDD at level " << i << ": " << forest->getNodeManUsed(i) << std::endl;
    }
    for (uint64_t i=1; i<=num; i++) {
        std::cout << "Rex at level " << i << ": " << compForest->getNodeManUsed(i) << std::endl;
    }

    delete forest;
    return ans;
}
       

int main(int argc, char** argv)
{
    uint64_t  num;
    PredefForest bdd;
    unsigned long mod;
    int TESTS = 10000;
    
    mod = 3;
    num = 4;
    bdd = PredefForest::UBDD;
    
    std::cout<< "ReduceEdge test." << std::endl;

    bool pass = 0;

    pass = buildSetForest(num, bdd);

    if (!pass) {
        std::cout << "Test Failed!" << std::endl;
    } else {
        std::cout << "Test Pass!" << std::endl;
    }
    return !pass;
}