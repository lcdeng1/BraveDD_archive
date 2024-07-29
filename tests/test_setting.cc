#include "brave_dd.h"

using namespace BRAVE_DD;

int main() {
    printInfo();

    // basic test case
    unsigned numVals = 10;
    RangeType rangeType = FINITE;
    ValueType valueType = INT;
    int maxRange = 20;
    bool hasNegInf = 0;
    bool hasPosInf = 0;
    bool hasUnDef = 0;
    EncodeMechanism encode = TERMINAL;
    int dim = 1;
    ReductionType reductionType = FULLY;
    SwapSet swapType = ONE;
    CompSet CompSet = NO_COMP;

    // default setting
    ForestSetting setting1(numVals);

    // build forest
    Forest forest1(setting1);
    
    // change setting1 totally different from forest1
    setting1.setNumVars(5);                    // Number of variables
    setting1.setRangeType(FINITE);             // Range type
    setting1.setValType(INT);                  // Value type
    setting1.setMaxRange(6);                   // Max range, N
    setting1.setNegInf(1);                     // Negative infinity
    setting1.setPosInf(0);                     // Positive infinity
    setting1.setUnDef(0);                      // Undefined
    setting1.setEncodeMechanism(EDGE_PLUS);    // Encoding mechanism
    setting1.setDim(2);                        // Dimension
    setting1.setReductionType(FULLY);          // Reudction type
    // ^ may add or delete rules here          // Add/delete reduction rules
    setting1.setSwapType(NO_SWAP);             // Swap flag
    setting1.setCompType(NO_COMP);             // Complement flag
    setting1.setMergeType(PUSH_DOWN);          // Merge type (this will be removed)
    setting1.setName("v5f6-EVBDD");            // Name

    if (forest1.getNumLevel() == setting1.getNumVars()
        || forest1.getRangeType() == setting1.getRangeType()
        || forest1.getValType() == setting1.getValType()
        || forest1.getMaxRange() == setting1.getMaxRange()
        || forest1.hasNegInf() == setting1.hasNegInf()
        || forest1.hasPosInf() == setting1.hasPosInf()
        || forest1.hasUnDef() == setting1.hasUnDef()
        || forest1.getEncodeMechanism() == setting1.getEncodeMechanism()
        || forest1.isRelation() == setting1.isRelation()
        || forest1.getReductionType() == setting1.getReductionType()
        || forest1.getSwapType() == setting1.getSwapType()
        || forest1.getCompType() == setting1.getCompType()
        || forest1.getForestName() == setting1.getName()) {
            exit(1);
        }


    return 0;
}