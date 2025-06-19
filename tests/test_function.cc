#include "brave_dd.h"

#include <random>
#include <iostream>
#include <cstdint>

using namespace BRAVE_DD;



Edge buildEvSetEdge(Forest* forest,
                    uint16_t lvl,
                    std::vector<int>& fun,
                    int start, int end)
{
    std::vector<Edge> child(2);
    EdgeLabel label = 0;
    packRule(label, RULE_X);
    if (lvl == 1) {
        // TODO: Determine how to assign partial function
        child[0].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA)); 
        if (fun[start]) child[0].setValue(fun[start]);
        child[1].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA)); 
        if (fun[end]) child[1].setValue(fun[end]);
        child[0].setRule(RULE_X); 
        child[1].setRule(RULE_X);
        return forest->reduceEdge(lvl, label, lvl, child);
    }
    child[0] = buildEvSetEdge(forest, lvl-1, fun, start, start+(1<<(lvl-1))-1);
    child[1] = buildEvSetEdge(forest, lvl-1, fun, start+(1<<(lvl-1)), end);
    return forest->reduceEdge(lvl, label, lvl, child);
}

bool buildEvSetForest(uint16_t num, PredefForest bdd)
{
    // test up to 5 variables
    if (num > 5) return 0;
    ForestSetting setting(bdd, num);
    setting.setValType(INT);
    Forest* forest = new Forest(setting);
    forest->getSetting().output(std::cout);
    bool ans = 0;

    std::vector<std::vector<bool> > vars(1, std::vector<bool>(num+1, false));
    for (int i=0; i<(1<<num); i++) {
      for (uint16_t k=1; k<=num; k++){
          vars[i][k] = i & (1<<(k-1));
      }
    }    // function values
    Func function(forest);
    std::vector<std::vector<int> > funs(1, std::vector<int>(1<<num, false));
    funs[0] = std::vector<int>{0, 5, 4, 3, 4, 5, 1, 1, 4, 3, 2, 4, 5, 2, 5, 2};
    for (int i=0; i < 1; i++) { 
        // build function
        Edge edge = buildEvSetEdge(forest, num, funs[i], 0, (1<<num)-1);
        function.setEdge(edge);
        // evaluate function
        for (int j=0; j<1; j++) {
            Value eval = function.evaluate(vars[0]);
            union
            {
                int valInt;
                float valFloat;
            };
            eval.getValueTo(&valInt, INT);
            std::cout<<"Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
            std::cout<<"\t assignment: ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << vars[0][k] << " ";
            }
            std::cout << "; value shoud be: " << funs[0][j] << "; was: " << valInt << std::endl;
            std::cout<<"\t full function: ";
            for (int k=0; k<(1<<num); k++){
                std::cout << funs[0][k] << " ";
            }
            std::cout << std::endl;
            
            std::string name = bdd == PredefForest::EVQBDD ? "EVQ" : "EVF";

            DotMaker dot(forest, name);
            dot.buildGraph(function);
            dot.runDot("pdf");
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
    std::cout<< "ReduceEdge test." << std::endl;
    
    buildEvSetForest(4, PredefForest::EVQBDD);
    buildEvSetForest(4, PredefForest::EVFBDD);
}