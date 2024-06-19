#include "brave_dd.h"

using namespace BRAVE_DD;

int main () {
    printInfo();

    /**
     * Forest Setting Construction 1
     * Note: this will construct the setting with a given 
     *       number of variables and default BDD specifications.
     */
    ForestSetting* setting1 = new ForestSetting(10);

    /**
     * Forest Setting Construction 2
     * Note: this will construct the setting as predefined BDD
     *       with the given name.
     */
    ForestSetting* setting2 = new ForestSetting("RexBDD", 10, 0);

    /**
     * Configuring the setting users need
     * Note: it will check the consistency.
     */
    try
    {
        /* Configure here (any setting order) */
        setting1->setNumVars(5);                    // Number of variables
        setting1->setRangeType(FINITE);             // Range type
        setting1->setValType(INT);                  // Value type
        setting1->setMaxRange(6);                   // Max range, N
        setting1->setNegInf(0);                     // Negative infinity
        setting1->setPosInf(0);                     // Positive infinity
        setting1->setUnDef(0);                      // Undefined
        setting1->setEncodeMechanism(EDGE_PLUS);    // Encoding mechanism
        setting1->setDim(1);                        // Dimension
        setting1->setReductionType(FULLY);          // Reudction type
        // may add or delete rules here             // Add/delete reduction rules
        setting1->setSwapType(NO_SWAP);             // Swap flag
        setting1->setCompType(COMP);                // Complement flag
        setting1->setMergeType(PUSH_UP);            // Merge type (this will be removed)
        setting1->setName("v5f6-CEVBDD");           // Name
        /* Check consistency */
        setting1->checkConsistency();
    }
    catch(const BRAVE_DD::error& e)
    {
        std::cerr << e.what() << '\n';
        exit(e.getCode());
    }
    
    /**
     * Forest construction
     * 
     */
    Forest* forest1 = new Forest(setting1);
    Forest* forest2 = new Forest(setting2);


    /**
     * Some Operations
     * 
     */
    // Edges
    Edge edge1(forest1), edge2(forest1), edge3(forest2), edge4(forest2);
    forest1->makeConstant(0, 5, &edge1);
    forest1->makeConstant(1, 5, &edge2);
    forest2->makeVariable(5, &edge3);
    forest2->makeVariable(5, &edge4);
    edge1 = !edge2;
    edge1 = edge1 + edge2;
    edge1 += edge2;
    edge3 = edge3 & edge4;
    edge3 |= edge4;

    // Forests
    
    // more operations TBD



    /**
     * Clean Everything
     * 
     */
    forest1->~Forest();
    forest2->~Forest();
    

    return 0;
}