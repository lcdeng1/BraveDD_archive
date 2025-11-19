<<<<<<< HEAD
#include "/home/dara/Git/brave_dd/src/brave_dd.h"
#include "/home/dara/Git/brave_dd/examples/BDDToDFA/Variable.h"
#include "/home/dara/Git/brave_dd/examples/BDDToDFA/qRBDDToBoolForDFA.cc"
=======
#include "brave_dd.h"
>>>>>>> 7afcd10bb9ee720cb7c939ea0ee6b978ff475eb2
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace BRAVE_DD;



int qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments, std::string name);


/*int Variable::maxIndex;
std::vector<Variable *> Variable::registry;*/

int main()
{

    // create bdd x_1
    // needs to also have
    ForestSetting settingx_1("QBDD", 1);

    Forest *forestx_1 = new Forest(settingx_1);

    Func rootx_1(forestx_1);

    rootx_1.variable(1);

    Func resultx_1 = rootx_1;

    // create bdd x_1
    // needs to also have
    ForestSetting settingx_2("QBDD", 4);

    Forest *forestx_2 = new Forest(settingx_2);

    Func root1(forestx_2);
    Func root2(forestx_2);
    Func root3(forestx_2);
    Func root4(forestx_2);

    root1.variable(1);
    root2.variable(2);
    root3.variable(3);
    root4.variable(4);

    Func result(forestx_2);
    result = (root4 & root2) | (root3 & root1);

    DotMaker dot1(forestx_2);
    dot1.buildGraph(result, "func2");
    dot1.runDot("func2", "pdf");

    qRBDDToBoolForDFA(result, 4, 2, "Func2");



    BddxMaker bm(forestx_2, "test_QRBDD");
    bm.buildBddx(result);

    delete forestx_1;
    delete forestx_2;

    printInfo();
    return 0;
}


