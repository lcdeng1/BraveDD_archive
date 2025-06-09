#include <iostream>
#include <vector>
#include <brave_dd.h>
#include <cassert>

using namespace BRAVE_DD;

int main() {
    std::cout << "Starting ev+ tests with more detailed debug output" << std::endl;

    // Create a ForesetSetting using 
    // ForestSetting setting(PredefForest::EVQBDD,1, EncodeMechanism::EDGE_PLUS);

    // std::cout << "Checking if the setting is correct..." << std::endl;

    // assert(setting.getName() == "EVQBDD");
    // assert(setting.getRangeType() == RangeType::NNINTEGER);
    // assert(setting.getValType() == VOID);
    // assert(setting.hasPosInf());
    // assert(setting.getEncodeMechanism() == EncodeMechanism::EDGE_PLUS);

    // ForestSetting setting = ForestSetting("ev+qbdd", 10);
    // assert(setting.getName() == "EVQBDD");
    // assert(setting.getRangeType() == RangeType::NNINTEGER);
    // assert(setting.getValType() == VOID);
    // assert(setting.hasPosInf());
    // assert(setting.getEncodeMechanism() == EncodeMechanism::EDGE_PLUS);

    // -------------------------------------------------------------------

    // Testing Constant function 5
    ForestSetting setting = ForestSetting("ev+qbdd", 0);
    Forest* forest0 = new Forest(setting);
    Func func0(forest0);
    func0.constant(5);

    int dangling_edge;
    func0.getEdge().getValue().getValueTo(&dangling_edge,INT);

    assert(dangling_edge == 5 && "The constant function is embedded wrong");

    DotMaker dot0(forest0,"constant_5");
    dot0.buildGraph(func0);
    dot0.runDot("pdf");

    delete forest0;

    return 0;
} 