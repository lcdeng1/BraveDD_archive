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
        // leave this if for future partial functions
        if (fun[start]) child[0].setValue(Value(fun[start]));
        child[1].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA)); 
        // leave this if for future partial functions
        if (fun[end]) child[1].setValue(Value(fun[end]));
        child[0].setRule(RULE_X); 
        child[1].setRule(RULE_X);
        return forest->reduceEdge(lvl, label, lvl, child);
    }
    child[0] = buildEvSetEdge(forest, lvl-1, fun, start, start+(1<<(lvl-1))-1);
    child[1] = buildEvSetEdge(forest, lvl-1, fun, start+(1<<(lvl-1)), end);
    return forest->reduceEdge(lvl, label, lvl, child);
}

bool buildEvSetForest(uint16_t num, PredefForest bdd, unsigned long maxRange=0)
{
    // test up to 5 variables
    if (num > 5) return 0;
    ForestSetting evSetting(bdd, num);
    ForestSetting evmodSetting(PredefForest::EVMODQBDD, num);
    if (bdd == PredefForest::EVFBDD) evmodSetting = ForestSetting(PredefForest::EVMODFBDD, num);
    evSetting.setValType(INT);
    evmodSetting.setValType(INT);
    evmodSetting.setMaxRange(maxRange);

    Forest* evforest = new Forest(evSetting);
    Forest* evmodforest = new Forest(evmodSetting);
    evforest->getSetting().output(std::cout);
    evmodforest->getSetting().output(std::cout);
    bool ans = 0;

    std::vector<std::vector<bool> > vars(1<<num, std::vector<bool>(num+1, false));
    for (int i=0; i<(1<<num); i++) {
        for (uint16_t k=1; k<=num; k++){
            vars[i][k] = i & (1<<(k-1));
        }
    }
    // function values
    Func evFunction(evforest);
    Func evmodFunction(evmodforest);
    std::vector<std::vector<int> > funs(1<<(1<<num), std::vector<int>(1<<num, false));
    for (int i=0; i<(1<<(1<<num)); i++) { 
        for (int k=0; k<(1<<num); k++) {
            funs[i][k] = distr32_int(gen); 
        }
        // build function
        Edge edge = buildEvSetEdge(evforest, num, funs[i], 0, (1<<num)-1);
        evFunction.setEdge(edge);
        Edge evmodEdge = evFunction.convert(evmodforest, edge);
        evmodFunction.setEdge(evmodEdge);
        // evaluate function
        for (int j=0; j<(1<<num); j++) {
            int evValue;
            int evmodValue;
            evFunction.evaluate(vars[j]).getValueTo(&evValue, INT);
            evFunction.evaluate(vars[j]).getValueTo(&evmodValue, INT);
            if (evValue != evmodValue) {
                std::cout<<"Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
                std::cout<<"\t assignment: ";
                for (uint16_t k=1; k<=num; k++){
                    std::cout << vars[j][k] << " ";
                }
                std::cout << std::endl;

                DotMaker evDot(evforest, "ev_function");
                evDot.buildGraph(evFunction);
                evDot.runDot("pdf");

                DotMaker evModDot(evmodforest, "evmod_function");
                evModDot.buildGraph(evmodFunction);
                evModDot.runDot("pdf");
                return 0;
            }            
        }
    }
    ans = 1;
    return ans;
}

int main(int argc, char** argv)
{
    uint16_t num;
    unsigned long mod;
    if (argc == 2) {
        mod = static_cast<unsigned long>(atoi(argv[2]));
        num = atoi(argv[1]);
    } else {
        mod = 3;
        num = 4;
    }

    std::cout<< "ReduceEdge test." << std::endl;

    ForestSetting setting(PredefForest::EVQBDD, num);
    Forest* forest = new Forest(setting);
    forest->getSetting().output(std::cout);
    delete forest;

    bool pass = 0;
 
    std::cout << "Testing EV+QBDD to EVMODQBDD" << std::endl;
    pass = buildEvSetForest(num, PredefForest::EVQBDD, mod);
    if (!pass) {
        std::cout << "Test Failed!" << std::endl;
    } else {
        std::cout << "Test Pass!" << std::endl;
    }
    std::cout << "Testing EV+FBDD to EVMODFBDD" << std::endl;
    pass = buildEvSetForest(num, PredefForest::EVFBDD, mod);
    if (!pass) {
        std::cout << "Test Failed!" << std::endl;
    } else {
        std::cout << "Test Pass!" << std::endl;
    }
    return !pass;
}