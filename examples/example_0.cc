#include "brave_dd.h"

using namespace BRAVE_DD;

int main () {
    printInfo();

    /**
     * Forest Setting Construction 1
     * Note: this will construct the setting with a given 
     *       number of variables and default BDD specifications.
     */
    // ForestSetting* setting1 = new ForestSetting(10);
    ForestSetting setting1(10);
    /* Configure here (any setting order) */
    setting1.setNumVars(5);                    // Number of variables
    setting1.setRangeType(FINITE);             // Range type
    setting1.setValType(INT);                  // Value type
    setting1.setMaxRange(6);                   // Max range, N
    setting1.setNegInf(0);                     // Negative infinity
    setting1.setPosInf(0);                     // Positive infinity
    setting1.setUnDef(0);                      // Undefined
    setting1.setEncodeMechanism(EDGE_PLUS);    // Encoding mechanism
    setting1.setDim(1);                        // Dimension
    setting1.setReductionType(FULLY);          // Reudction type
    setting1.addReductionRule(RULE_AH0);
    // ^ may add or delete rules here          // Add/delete reduction rules
    setting1.setSwapType(NO_SWAP);             // Swap flag
    setting1.setCompType(COMP);                // Complement flag
    setting1.setMergeType(PUSH_UP);            // Merge type (this will be removed)
    setting1.setName("v5f6-CEVBDD");           // Name


    /**
     * Forest Setting Construction 2
     * (name of predefined BDD, number of variables, is relation)
     * Note: this will construct the setting as predefined BDD
     *       with the given name.
     */
    // ForestSetting* setting2 = new ForestSetting("RexBDD", 10);
    ForestSetting setting2("RexBDD", 10);
    
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

    // Roots
    Root root1(forest1), root2(forest1);
    root1 = forest1->constant(5);
    root2 = forest1->variable(5);
    root1 += root2;

    EdgeLabel l;
    ReductionRule r = unpackRule(l);

    // Forests
    
    // more operations TBD


    /**
     * Clean Everything
     * 
     */
    delete forest1;
    delete forest2;

    return 0;
}