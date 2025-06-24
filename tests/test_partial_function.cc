#include "brave_dd.h"

#include <random>
#include <iostream>
#include <cstdint>
long seed =123456789;

using namespace BRAVE_DD;

std::mt19937 gen(seed);   
std::uniform_int_distribution<uint32_t> distr32(-1, 5);

Edge buildEvSetEdge(Forest* forest,
                    uint16_t lvl,
                    std::vector<Value>& fun,
                    int start, int end)
{
    std::vector<Edge> child(2);
    EdgeLabel label = 0;
    packRule(label, RULE_X);
    if (lvl == 1) {
        if (fun[start].getType() != VOID) {
            child[0].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA));
            child[0].setValue(fun[start]);
            
        } else {
            child[0].setEdgeHandle(makeTerminal(VOID, SpecialValue::POS_INF));
        }
        if(fun[end].getType() != VOID) {
            child[1].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA)); 
            child[1].setValue(fun[end]);
        } else {
            child[1].setEdgeHandle(makeTerminal(VOID, SpecialValue::POS_INF));
        }
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
    ForestSetting setting(bdd, num);
    setting.setValType(INT);
    if(bdd == PredefForest::EVMODQBDD || bdd == PredefForest::EVMODFBDD) setting.setMaxRange(maxRange);
    Forest* forest = new Forest(setting);
    forest->getSetting().output(std::cout);
    bool ans = 0;

    std::vector<std::vector<bool> > vars(1 << num, std::vector<bool>(num+1, false));
    for (int i=0; i<(1<<num); i++) {
        for (uint16_t k=1; k<=num; k++){
            vars[i][k] = i & (1<<(k-1));
        }
    }
    // function values
    Func function(forest);
    std::vector<std::vector<Value> > funs(1<<(1<<num), std::vector<Value>(1<<num, Value(0)));
    for (int i=0;i<(1<<(1<<num));i++) {
        for (int k=0;k<(1<<num);k++) {
            int temp = distr32(gen);
            if (temp < 0) funs[i][k] = Value(SpecialValue::POS_INF);
            else funs[i][k] = Value(temp);
        }
        // build function
        Edge edge = buildEvSetEdge(forest, num, funs[i], 0, (1<<num)-1);
        function.setEdge(edge);
        // evaluate function
        for (int j=0; j<(1<<num); j++) {
            Value eval = function.evaluate(vars[j]);
            if (setting.getEncodeMechanism() == EDGE_PLUS) {
                if (eval != funs[i][j]) {
                    std::cout<<"Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
                    std::cout<<"\t assignment: ";
                    for (uint16_t k=1; k<=num; k++){
                        std::cout << vars[j][k] << " ";
                    }
                    std::cout << "; value shoud be: ";
                    funs[i][j].print(std::cout, 0) ;
                    std::cout << "; was: ";
                    eval.print(std::cout,0);
                    std::cout << std::endl;
                    std::cout<<"\t full function: ";
                    for (int k=0; k<(1<<num); k++){
                        funs[i][k].print(std::cout, 0);
                        std::cout << " ";
                    }
                    std::cout << std::endl;
    
                    DotMaker dot(forest, "error_function");
                    dot.buildGraph(function);
                    dot.runDot("pdf");
                    return 0;
                }
            } else {
                bool isCorrect = 0;

                if (eval.getType() == VOID && funs[i][j].getType() == VOID) {
                    SpecialValue evalVal, funVal;
                    eval.getValueTo(&evalVal, VOID);
                    funs[i][j].getValueTo(&funVal, VOID);
                    if (evalVal == SpecialValue::POS_INF && funVal == SpecialValue::POS_INF) isCorrect = 1;
                } else {
                    int evalInt, funInt;
                    eval.getValueTo(&evalInt, INT);
                    funs[i][j].getValueTo(&funInt, INT);
                    isCorrect = evalInt == (funInt % setting.getMaxRange()) ? 1 : 0;
                }

                if (!isCorrect) {

                    std::cout<<"Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
                    std::cout<<"\t assignment: ";
                    for (uint16_t k=1; k<=num; k++){
                        std::cout << vars[j][k] << " ";
                    }
                    std::cout << "; value shoud be: ";
                    funs[i][j].print(std::cout,0);
                    std::cout  << "; was: ";
                    eval.print(std::cout,0); 
                    std::cout << std::endl;
                    std::cout<<"\t full function: ";
                    for (int k=0; k<(1<<num); k++){
                        funs[i][k].print(std::cout, 0);
                        std::cout << " ";
                    }
                    std::cout << std::endl;
    
                    DotMaker dot(forest, "error_function");
                    dot.buildGraph(function);
                    dot.runDot("pdf");
                    return 0;
                }
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
    std::cout<< "Testing edge valued decision diagram with partial function." << std::endl;
    uint16_t num;
    unsigned long mod;

    if (argc == 2) {
        mod = static_cast<unsigned long>(atoi(argv[2]));
        num = atoi(argv[1]);
    } else {
        mod = 3;
        num = 4;
    }

    bool pass = 0;
    std::cout<< "Testing EVQBDD ... " << std::endl;
    pass = buildEvSetForest(num, PredefForest::EVQBDD);
    std::cout<< "Testing EVMODQBDD ... " << std::endl;
    pass = buildEvSetForest(num, PredefForest::EVMODQBDD, mod);
    std::cout<< "Testing EVFBDD ... " << std::endl;
    pass = buildEvSetForest(num, PredefForest::EVFBDD);
    std::cout<< "Testing EVMODFBDD ... " << std::endl;
    pass = buildEvSetForest(num, PredefForest::EVMODFBDD, mod);

    std::cout << "Finished testing edge valued decision diagram with partial function" << std::endl;
    if (!pass) {
        std::cout << "Test Failed!" << std::endl;
    } else {
        std::cout << "Test Pass!" << std::endl;
    }
    return !pass;
}