#include "brave_dd.h"

long seed =123456789;

using namespace BRAVE_DD;

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

bool testImage(uint16_t num, PredefForest bdd, PredefForest bmxd)
{
    ForestSetting settingS(bdd, num);
    Forest* forestS = new Forest(settingS);
    ForestSetting settingR(bmxd, num);
    Forest* forestR = new Forest(settingR);
    bool ans = 0;
    // variables
    std::vector<bool> from(num+1, 0);
    std::vector<bool> to(num+1, 0);
    // function values
    Func functionS0(forestS);
    Func functionR(forestR);
    std::vector<bool> funS0(1<<(num), 0);
    std::vector<bool> funR(1<<(2*num), 0);
    for (long long i=0; i<(1<<(num)); i++) {
        funS0[i] = random01();
    }
    for (long long i=0; i<(1<<(2*num)); i++) {
        funR[i] = random01();
    }
    // build functions
    Edge eS0 = buildSetEdge(forestS, num, funS0, 0, (1<<num)-1);
    Edge eR = buildRelEdge(forestR, num, funR, 0, (1<<(2*num))-1);
    Func functionS1(forestS);
    functionS0.setEdge(eS0);
    functionR.setEdge(eR);
    // apply image
    apply(POST_IMAGE, functionS0, functionR, functionS1);
    // evaluation
    bool hasTo = 0;
    for (long long i=0; i<(1<<num); i++) {
        // to variables
        for (uint16_t k=1; k<=num; k++) {
            to[k] = i&(1<<(k-1));
        }
        int valIntS1;
        Value evalS1 = functionS1.evaluate(to);
        evalS1.getValueTo(&valIntS1, INT);
        // check if any 'from' state go to 'to' state
        for (long long j=0; j<(1<<num); j++) {
            // from variables
            for (uint16_t k=1; k<=num; k++) {
                from[k] = j&(1<<(k-1));
            }
            Value evalS0 = functionS0.evaluate(from);
            Value evalR = functionR.evaluate(from, to);
            int valIntS0, valIntR;
            evalS0.getValueTo(&valIntS0, INT);
            evalR.getValueTo(&valIntR, INT);
            hasTo |= ((bool)valIntS0 & (bool)valIntR);
            if (hasTo) break;
        }
        if (hasTo != (bool)valIntS1) {
            std::cout<<"Evaluation Failed for " << num << " variable(s), function " << i << std::endl;
            std::cout<<"\t assignment (from): ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << from[k] << " ";
            }
            std::cout<<"\n\t assignment (to): ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << to[k] << " ";
            }
            std::cout << "\nvalue shoud be: " << hasTo << "; was: " << valIntS1 << std::endl;

            std::cout << std::endl;

            DotMaker dot0(forestS, "error_from");
            dot0.buildGraph(functionS0);
            dot0.runDot("pdf");
            DotMaker dot1(forestS, "error_to");
            dot1.buildGraph(functionS1);
            dot1.runDot("pdf");
            DotMaker dot2(forestR, "error_rel");
            dot2.buildGraph(functionR);
            dot2.runDot("pdf");
            return 0;
        }
        hasTo = 0;
    }
    ans = 1;

    delete forestS;
    delete forestR;
    return ans;
}

int main(int argc, char** argv)
{
    uint16_t num;
    PredefForest bdd, bmxd;
    int TESTS = 10000;
    if (argc == 4) {
        num = atoi(argv[1]);
        bdd = (PredefForest)atoi(argv[2]);
        bmxd = (PredefForest)atoi(argv[3]);
    } else if (argc == 5) {
        num = atoi(argv[1]);
        bdd = (PredefForest)atoi(argv[2]);
        bmxd = (PredefForest)atoi(argv[3]);
        TESTS = atoi(argv[4]);
    } else {
        num = 4;
        bdd = PredefForest::REXBDD;
        bmxd = PredefForest::ESRBMXD;
    }

    std::cout<< "Image test." << std::endl;

    ForestSetting settingS(bdd, num);
    settingS.output(std::cout);
    ForestSetting settingR(bmxd, num);
    settingR.output(std::cout);

    bool pass = 1;
    for (int i=0; i<TESTS; i++) {
        pass &= testImage(num, bdd, bmxd);
        if (!pass) {
            std::cout << "test failed at " << i << std::endl;
            break;
        }
    }

    if (!pass) {
        std::cout << "Test Failed!" << std::endl;
    } else {
        std::cout << "Test Pass!" << std::endl;
    }
    return !pass;
}