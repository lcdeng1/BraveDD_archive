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

    // ForestSetting setting = ForestSetting(10);
    ForestSetting setting = ForestSetting("ev+qbdd", 1);


    Forest* forest0 = new Forest(setting);
    
    Func func0(forest0);

    // func.constant(5);
    func0.constant(5);

    int first_ev;
    func0.getEdge().getValue().getValueTo(&first_ev,INT);

    DotMaker dot0(forest0,"omega");
    dot0.buildGraph(func0);
    dot0.runDot("pdf");

    int last_ev;
    func0.getEdge().getValue().getValueTo(&last_ev,INT);
    std::cout<< "At the end " << last_ev << std::endl;

    delete forest0;

    // Forest* forest1 = new Forest(setting);
    
    // Func func1(forest1);

    // func.constant(5);
    // func1.constant(SpecialValue::POS_INF);

    // DotMaker dot1(forest1,"infty");
    // dot1.buildGraph(func1);
    // dot1.runDot("pdf");

    // delete forest1;



    return 0;
} 