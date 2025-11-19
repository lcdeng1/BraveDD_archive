/* 
 * This is an usage example of building logic expression in a BDD representation,
 * then output the BDD as .bddx format and parse a .bddx to rebuild the BDD
 * 
 * Author: Lichuan Deng
 */

 #include "brave_dd.h"

int main (int argc, const char** argv) {
    using namespace BRAVE_DD;

    /* Initialize Forest */
    ForestSetting setting(PredefForest::QBDD, 4);
    Forest* forest = new Forest(setting);
    Func x1(forest), x2(forest), x3(forest), x4(forest);
    x1.variable(1);
    x2.variable(2);
    x3.variable(3);
    x4.variable(4);
    /* Example logic expression */
    Func target = ((x4) & (x1)) | (x3 & x2);

    /* Report information */
    int align = 36;
    std::cout << std::left << std::setw(align) << "BDD name: " << setting.getName() << std::endl;
    std::cout << std::left << std::setw(align) << "Number of variables: " << setting.getNumVars() << std::endl;
    std::cout << std::left << std::setw(align) << "Number of nodes: " << forest->getNodeManUsed(target) << std::endl;

    long numOnes = 0;
    apply(CARDINALITY, target, numOnes);
    std::cout << std::left << std::setw(align) << "Cardinality: " << numOnes << std::endl;

    /* output the .bddx file */
    BddxMaker bm(forest);
    bm.buildBddx(target, "06_bddx_IO_example_QBDD");

    /* Read in the built .bddx file */
    // construct a .bddx parser
    ParserBddx parser("06_bddx_IO_example_QBDD.bddx");
    // (optional) Initialize a new Forest
    ForestSetting setting1(PredefForest::QBDD, 4);
    Forest* forest1 = new Forest(setting1);
    // parsing the input file
    parser.parse(forest1);
    // report header info
    std::cerr << std::left << std::setw(align) << "Rebuilt BDD: " << std::endl;
    std::cerr << std::left << std::setw(align) << "Number variables: " << parser.getNumVars() << std::endl;
    std::cerr << std::left << std::setw(align) << "Number nodes: " << parser.getNumNodes() << std::endl;
    std::cerr << std::left << std::setw(align) << "Number roots: " << parser.getNumRoots() << std::endl;
    // get the root edge by given the root index (default as the first root)
    Func root = parser.getRoot();
    // get all the root edges
    std::vector<Func> roots = parser.getAllRoots();
    /* Report information of the rebuilt BDD */
    std::cerr << std::left << std::setw(align) << "Read-in Num nodes (first-root): " << forest1->getNodeManUsed(root) << std::endl;
    std::cerr << std::left << std::setw(align) << "Read-in Num nodes (all-root): " << forest1->getNodeManUsed(roots) << std::endl;
    long num_state = 0;
    apply(CARDINALITY, root, num_state);
    std::cerr << std::left << std::setw(align) << "Read-in Cardinality (first-root): " << num_state << std::endl;
    for (size_t i=1; i<roots.size(); i++) {
        long num = 0;
        apply(CARDINALITY, roots[i], num);
        num_state += num;
    }
    std::cerr << std::left << std::setw(align) << "Read-in Cardinality (all-root): " << num_state << std::endl;
    /* Output the .bddx file from the rebuilt BDD */
    BddxMaker bm1(forest1);
    bm1.buildBddx(parser.getAllRoots(), "06_bddx_IO_example_QBDD_rebuilt");

    /**
     * Clean Everything
     */
    delete forest;
    delete forest1;
}