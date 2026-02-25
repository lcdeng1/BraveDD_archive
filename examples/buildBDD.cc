#include "BDDToDFA/Variable.h"
#include "BDDToDFA/qRBDDToBoolForDFA.cc"
#include "brave_dd.h"
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace BRAVE_DD;



//qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments, std::string name);


/*int Variable::maxIndex;
std::vector<Variable *> Variable::registry;*/

int main()
{
    //need file name
    std::string fileName = "Func4.bddx";

    //need number of levels

    

    ParserBddx parser(fileName);
    ForestSetting settingx("QBDD", 4); 
    Forest *forestx = new Forest(settingx);
    Func result(forestx);
    parser.parse(forestx);
    Func root = parser.getRoot();
    std::string funcName = "func4";

    DotMaker dot4(forestx);
    dot4.buildGraph(root, funcName);
    dot4.runDot(funcName, "pdf");

    //need width of the bdd
    uint32_t width = root.width();

    //params: BRAVE_DD::Func qrbdd, int numStates, int numAssignments, int numVerticies, func name)
    qRBDDToBoolForDFA convert(root, 5, 2, forestx->getNodeManUsed(root) + 2, funcName);

    
    //qRBDDToBoolForDFA(toTest, 2, 2, funcName, 1, 3);
    
    

    

    convert.printToFile(convert.qRBDDToDFASAT(), "satFunctionForQRBDDtoDFA" + funcName, "txt");

    //convert.printToFile(convert.SATtoReadableLatex(funcName), "satFunctionForQRBDDtoDFALatex" + funcName , "txt");

    convert.runKissat(funcName);

    /*while (!convert.isSatisfiable(funcName)){

    }*/

    convert.printToFile(convert.satToDFAOutput(funcName), "DFAFormat"+ funcName, "txt");

    convert.runDFAToBDDx(funcName);



    BddxMaker bm(forestx);
    bm.buildBddx(result, "Func4.bddx");

    ParserBddx parser2("DFAFormat" + funcName + "(OUTPUT).bddx");
    //ParserBddx parser2("DFAFormatfunc3(OUTPUT).bddx"); //(!root4 & root2) | (root3 & root1);



    // parsing the input file
    parser2.parse(forestx); 
    Func rootB = parser2.getRoot();

    DotMaker dot4_2(forestx);
    dot4_2.buildGraph(result, "func4");
    dot4_2.runDot("func4", "pdf");

    if (root == rootB){
        printf("YAY!\n");
    }
    else {
        printf("BOO!\n");
    }




    /*BddxMaker bm(forestx_4);
    bm.buildBddx(toTest, "test_QRBDD");*/

    /*BddxMaker bm(forestx);
    bm.buildBddx(result, "Func4.bddx");

    ParserBddx parser2("DFAFormat" + funcName + "(OUTPUT).bddx");
    //ParserBddx parser2("Func4.bddx"); //(!root4 & root2) | (root3 & root1);



    // parsing the input file
    parser2.parse(forestx); 
    Func rootB = parser2.getRoot();

    DotMaker dot4_2(forestx);
    dot4_2.buildGraph(root, "func4");
    dot4_2.runDot("func4", "pdf");

    if (root == rootB){
        printf("YAY!\n");
    }
    else {
        printf("BOO!\n");
    }

    //delete forestx_1;
    //delete forestx_4;*/

    printInfo();
    return 0;
}


