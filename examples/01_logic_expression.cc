/* 
 * This is an usage example of building logic expression in a BDD representation
 * 
 * Author: Lichuan Deng
 */

 #include "brave_dd.h"

bool isVisual = 0;

void processArgs(int argc, const char** argv)
{
    for (int i=1; i<argc; i++) {
        if ('-' == argv[i][0]) {
            if (strcmp("-v", argv[i])==0) {
                isVisual = 1;
                continue;
            }
        }
    }
}

int main (int argc, const char** argv) {
    using namespace BRAVE_DD;
    processArgs(argc, argv);
    /**
     * Forest Setting Construction
     * (name of predefined BDD, number of variables)
     * Note: this will construct the setting as predefined BDD
     */
    ForestSetting setting(PredefForest::FBDD, 4);
    /* Declare and construct Forest */
    Forest* forest = new Forest(setting);
    /* Declare BDD roots attached to the Forest */
    Func x1(forest), x2(forest), x3(forest), x4(forest);
    /* Initialize BDD roots as variables */
    x1.variable(1);
    x2.variable(2);
    x3.variable(3);
    x4.variable(4);
    /* Build the target logic expression */
    Func target = (!x4 & !x1) | (x3 & x2);

    if (isVisual) {
        /* Build BDD in DOT language for visualization */
        // this will generate a .gv file and compile it to the corresponding .pdf file
        DotMaker dot2(forest, "02_logic_expression_example_FBDD");
        dot2.buildGraph(target);
        dot2.runDot("pdf");
    }
    /* Report information */
    int align = 36;
    std::cout << std::left << std::setw(align) << "BDD name: " << setting.getName() << std::endl;
    std::cout << std::left << std::setw(align) << "Number of variables: " << setting.getNumVars() << std::endl;
    std::cout << std::left << std::setw(align) << "Number of nodes: " << forest->getNodeManUsed(target) << std::endl;
    long numOnes = 0;
    apply(CARDINALITY, target, numOnes);
    std::cout << std::left << std::setw(align) << "Number of assignments point to 1: " << numOnes << std::endl;

    /* Declare and construct Forest */
    ForestSetting setting1(PredefForest::REXBDD, 4);
    /* Declare and construct Forest */
    Forest* forest1 = new Forest(setting1);
    Func y1(forest1), y2(forest1), y3(forest1), y4(forest1);
    y1.variable(1);
    y2.variable(2);
    y3.variable(3);
    y4.variable(4);
    target = (!y4 & !y1) | (y3 & y2);

    if (isVisual) {
        /* Build BDD in DOT language for visualization */
        // this will generate a .gv file and compile it to the corresponding .pdf file
        DotMaker dot2(forest1, "02_logic_expression_example_RexBDD");
        dot2.buildGraph(target);
        dot2.runDot("pdf");
    }

    /* Report information */
    align = 36;
    std::cout << std::left << std::setw(align) << "BDD name: " << setting1.getName() << std::endl;
    std::cout << std::left << std::setw(align) << "Number of variables: " << setting1.getNumVars() << std::endl;
    std::cout << std::left << std::setw(align) << "Number of nodes: " << forest1->getNodeManUsed(target) << std::endl;
    numOnes = 0;
    apply(CARDINALITY, target, numOnes);
    std::cout << std::left << std::setw(align) << "Number of assignments point to 1: " << numOnes << std::endl;

    /**
     * Clean Everything
     */
    delete forest;
    delete forest1;
}