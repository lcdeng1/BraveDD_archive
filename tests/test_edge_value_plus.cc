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

    // // Testing constant function 5 with no variable
    // ForestSetting setting0 = ForestSetting("ev+qbdd", 0);
    // Forest* forest0 = new Forest(setting0);
    // Func func0(forest0);
    // func0.constant(5);

    // int dangling_edge0;
    // func0.getEdge().getValue().getValueTo(&dangling_edge0,INT);

    // assert(dangling_edge0 == 5 && "The constant function is embedded wrong");

    // DotMaker dot0(forest0,"constant_5_0_vars");
    // dot0.buildGraph(func0);
    // dot0.runDot("pdf");

    // delete forest0;

    //-----------------------------------------------------------------------------

    // Testing constant function 5 with 5 variables
    
    //-----------------------------------------------------------------------------

    // ForestSetting setting1 = ForestSetting(PredefForest::QBDD, 3);
    
    // Forest* forest1 = new Forest(setting1);
    // Func func1 =  Func(forest1);
    // func1.constant(0);

    // int dangling_edge1;
    // func1.getEdge().getValue().getValueTo(&dangling_edge1,INT);


    // DotMaker dot1(forest1,"constant_0_3_vars_qbdd");
    // dot1.buildGraph(func1);
    // dot1.runDot("pdf");

    // delete forest1;

    // std::cout << "\n\n\n\\n\n##################" << std::endl;
    // std::cout << "##################" << std::endl;
    // std::cout << "##################\\n\n\n\n\n" << std::endl;

    ForestSetting setting2 = ForestSetting("ev+qbdd", 5);
    setting2.setValType(INT);

    Forest* forest2 = new Forest(setting2);
    Func func2(forest2);
    func2.constant(5);

    DotMaker dot2(forest2,"evqbdd");
    dot2.buildGraph(func2);
    dot2.runDot("pdf");

    delete forest2;

    return 0;
} 