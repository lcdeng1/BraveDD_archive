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

Edge buildEvSetEdge(Forest* forest,
                    uint16_t lvl,
                    const std::vector<int>& fun,
                    int start, int end)
{
    std::vector<Edge> child(2);
    EdgeLabel label = 0;
    packRule(label, RULE_X);
    if (lvl == 1) {
        // TODO: Determine how to assign partial function
        child[0].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA)); 
        if (fun[start]) child[0].setValue(Value(fun[start]));
        child[1].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA)); 
        if (fun[end]) child[1].setValue(Value(fun[end]));
        child[0].setRule(RULE_X); 
        child[1].setRule(RULE_X);
        return forest->reduceEdge(lvl, label, lvl, child);
    }
    child[0] = buildEvSetEdge(forest, lvl-1, fun, start, start+(1<<(lvl-1))-1);
    child[1] = buildEvSetEdge(forest, lvl-1, fun, start+(1<<(lvl-1)), end);
    return forest->reduceEdge(lvl, label, lvl, child);
}


bool buildSetForest(uint16_t num, PredefForest bdd)
{
    // test up to 5 variables
    if (num > 5) return 0;
    ForestSetting setting(bdd, num);
    Forest* forest = new Forest(setting);
    forest->getSetting().output(std::cout);
    bool ans = 0;
    // variables
    std::vector<std::vector<bool> > vars(1<<num, std::vector<bool>(num+1, false));
    for (int i=0; i<(1<<num); i++) {
        for (uint16_t k=1; k<=num; k++){
            vars[i][k] = i & (1<<(k-1));
        }
    }
    // function values
    Func function(forest);
    std::vector<std::vector<bool> > funs(1<<(1<<num), std::vector<bool>(1<<num, false));
    for (int i=0; i<(1<<(1<<num)); i++) { 
        for (int k=0; k<(1<<num); k++) {
            funs[i][k] = i & (1<<k); 
        }
        // build function
        Edge edge = buildSetEdge(forest, num, funs[i], 0, (1<<num)-1);
        function.setEdge(edge);
        // evaluate function
        for (int j=0; j<(1<<num); j++) {
            Value eval = function.evaluate(vars[j]);
            union
            {
                int valInt;
                float valFloat;
            };
            eval.getValueTo(&valInt, INT);
            if (valInt != funs[i][j]) {
                std::cout<<"Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
                std::cout<<"\t assignment: ";
                for (uint16_t k=1; k<=num; k++){
                    std::cout << vars[j][k] << " ";
                }
                std::cout << "; value shoud be: " << funs[i][j] << "; was: " << valInt << std::endl;
                std::cout<<"\t full function: ";
                for (int k=0; k<(1<<num); k++){
                    std::cout << funs[i][k] << " ";
                }
                std::cout << std::endl;

                DotMaker dot(forest, "error_function");
                dot.buildGraph(function);
                dot.runDot("pdf");
                return 0;
            }
        }
    }
    ans = 1;
    for (uint16_t i=1; i<=num; i++) {
        std::cout << "Number of nodes at level " << i << ": " << forest->getNodeManUsed(i) << std::endl;
    }
    delete forest;
    return ans;
}

// this will randomly build a relational function
bool buildRelForest(uint16_t num, PredefForest bmxd)
{
    ForestSetting setting(bmxd, num);
    Forest* forest = new Forest(setting);
    // forest->getSetting().output(std::cout);
    bool ans = 0;
    // variables
    std::vector<bool> from(num+1, 0);
    std::vector<bool> to(num+1, 0);
    // function values
    Func function(forest);
    std::vector<bool> fun(1<<(2*num), 0);
    for (long long i=0; i<(1<<(2*num)); i++) {
        fun[i] = random01();
    }
    // build function
    // std::cout << "start build rel edge\n";
    Edge edge = buildRelEdge(forest, num, fun, 0, (1<<(2*num))-1);
    // std::cout << "done build rel edge\n";
    function.setEdge(edge);
    // evaluate function
    for (long long i=0; i<(1<<(2*num)); i++) {
        // update variables
        for (uint16_t l=1; l<=num; l++) {
            from[l] = i & (0x01 << (2*l-1));
            to[l] = i & (0x01 << (2*l-2));
        }
        // std::cout << "start evaluation\n";
        Value eval = function.evaluate(from, to);
        // std::cout << "done evaluation\n";
        union
        {
            int valInt;
            float valFloat;
        };
        eval.getValueTo(&valInt, INT);
        if (valInt != fun[i]) {
            std::cout<<"Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
            std::cout<<"\t assignment (from): ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << from[k] << " ";
            }
            std::cout<<"\n\t assignment (to): ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << to[k] << " ";
            }
            std::cout << "\nvalue shoud be: " << fun[i] << "; was: " << valInt << std::endl;
            std::cout<<"\t full function: ";
            for (int k=0; k<(1<<(2*num)); k++){
                std::cout << fun[k] << " ";
            }
            std::cout << std::endl;

            DotMaker dot(forest, "error_function");
            dot.buildGraph(function);
            dot.runDot("pdf");
            return 0;
        }

    }
    ans = 1;
    for (uint16_t i=1; i<=num; i++) {
        std::cout << "Number of nodes at level " << i << ": " << forest->getNodeManUsed(i) << std::endl;
    }
    delete forest;
    return ans;
}

bool buildEvSetForest(uint16_t num, PredefForest bdd, unsigned long maxRange=0)
{
    // test up to 5 variables
    if (num > 5) return 0;
    ForestSetting setting(bdd, num);
    setting.setValType(INT);
    if(bdd == PredefForest::EVMODQBDD || bdd == PredefForest::EVMODFBDD) setting.setMaxRange(maxRange);
    Forest* forest = new Forest(setting);
    forest->getSetting().output(std::cout);
    bool ans = 0;

    std::vector<std::vector<bool> > vars(1<<num, std::vector<bool>(num+1, false));
    for (int i=0; i<(1<<num); i++) {
        for (uint16_t k=1; k<=num; k++){
            vars[i][k] = i & (1<<(k-1));
        }
    }
    // function values
    Func function(forest);
    std::vector<std::vector<int> > funs(1<<(1<<num), std::vector<int>(1<<num, false));
    for (int i=0; i<(1<<(1<<num)); i++) { 
        for (int k=0; k<(1<<num); k++) {
            funs[i][k] = distr32_int(gen); 
        }
        funs[0] = std::vector<int>{1, 0, 4, 4, 0, 5, 0, 1, 0, 2, 2, 4, 0, 2, 4, 1};
        // build function
        Edge edge = buildEvSetEdge(forest, num, funs[i], 0, (1<<num)-1);
        function.setEdge(edge);
        // evaluate function
        for (int j=0; j<(1<<num); j++) {
            Value eval = function.evaluate(vars[j]);
            union
            {
                int valInt;
                float valFloat;
            };
            eval.getValueTo(&valInt, INT);
            if (valInt != (funs[i][j] % setting.getMaxRange())) {
                std::cout<<"Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
                std::cout<<"\t assignment: ";
                for (uint16_t k=1; k<=num; k++){
                    std::cout << vars[j][k] << " ";
                }
                std::cout << "; value shoud be: " << funs[i][j] << "; was: " << valInt << std::endl;
                std::cout<<"\t full function: ";
                for (int k=0; k<(1<<num); k++){
                    std::cout << funs[i][k] << " ";
                }
                std::cout << std::endl;

                DotMaker dot(forest, "error_function");
                dot.buildGraph(function);
                dot.runDot("pdf");
                return 0;
            }
        }
    }
    ans = 1;
    for (uint16_t i=1; i<=num; i++) {
        std::cout << "Number of nodes at level " << i << ": " << forest->getNodeManUsed(i) << std::endl;
    }
    delete forest;
    return ans;
}

int main(int argc, char** argv)
{
    uint16_t num;
    PredefForest bdd;
    unsigned long mod;
    int TESTS = 10000;
    if (argc == 3) {
        mod = static_cast<unsigned long>(atoi(argv[3]));
        num = atoi(argv[2]);
        bdd = (PredefForest)atoi(argv[1]);
    } else {
        mod = 2;
        num = 4;
        bdd = PredefForest::REXBDD;
    }

    std::cout<< "ReduceEdge test." << std::endl;

    ForestSetting setting(bdd, num);
    Forest* forest = new Forest(setting);
    forest->getSetting().output(std::cout);
    delete forest;

    bool pass = 0;
    if (bdd < PredefForest::FBMXD) {
        pass = buildSetForest(num, bdd);
    } else if (bdd < PredefForest::EVQBDD) {
        for (int i=0; i<TESTS; i++) {
            pass = buildRelForest(num, bdd);
            if (!pass) break;
        }
    } else {
        pass = buildEvSetForest(num, bdd, mod);
    }

    if (!pass) {
        std::cout << "Test Failed!" << std::endl;
    } else {
        std::cout << "Test Pass!" << std::endl;
    }
    return !pass;
}