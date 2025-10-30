#include "setting.h"

using namespace BRAVE_DD;

int main(int argc, char** argv) {
    std::string bdd;
    if (argc == 1) bdd = "rexbdd";
    else bdd = argv[1];

    // basic test case
    Level numVals = 10;
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
    CompSet compSet = NO_COMP;
    MergeType mergeType = PUSH_DOWN;
    std::string name = "v5f6-EVBDD";

    // default setting
    ForestSetting setting1(bdd, numVals);
    // change setting1
    setting1.setNumVars(numVals);               // Number of variables
    setting1.setRangeType(rangeType);           // Range type
    setting1.setValType(valueType);             // Value type
    setting1.setMaxRange(maxRange);             // Max range, N
    setting1.setNegInf(hasNegInf);              // Negative infinity
    setting1.setPosInf(hasPosInf);              // Positive infinity
    setting1.setUnDef(hasUnDef);                // Undefined
    setting1.setEncodeMechanism(encode);        // Encoding mechanism
    setting1.setDim(dim);                       // Dimension
    setting1.setReductionType(reductionType);   // Reudction type
    // setting1.addReductionRule(RULE_I0);
    // setting1.addReductionRule(RULE_I1);
    setting1.setSwapType(swapType);             // Swap flag
    setting1.setCompType(compSet);              // Complement flag
    setting1.setMergeType(mergeType);           // Merge type (this will be removed)
    setting1.setName(name);                     // Name
    
    // check
    bool hasError = 0;
    if (setting1.getNumVars() != numVals) {
        std::cerr << "[Error]: number of variables" << std::endl;
        hasError = 1;
    }
    if (setting1.getRangeType() != rangeType) {
        std::cerr << "[Error]: range type" << std::endl;
        hasError = 1;
    }
    if (setting1.getValType() != valueType) {
        std::cerr << "[Error]: value type" << std::endl;
        hasError = 1;
    }
    if (setting1.getMaxRange() != (unsigned long)maxRange) {
        std::cerr << "[Error]: max range" << std::endl;
        hasError = 1;
    }
    if ((setting1.hasNegInf() != hasNegInf)
        || (setting1.hasPosInf() != hasPosInf)
        || (setting1.hasUnDef() != hasUnDef)) {
        std::cerr << "[Error]: special values" << std::endl;
        hasError = 1;
    }
    if (setting1.getEncodeMechanism() != encode) {
        std::cerr << "[Error]: encode mechanism" << std::endl;
        hasError = 1;
    }
    if (setting1.isRelation() != (dim > 1)) {
        std::cerr << "[Error]: dim" << std::endl;
        hasError = 1;
    }
    if (setting1.getReductionType() != reductionType) {
        std::cerr << "[Error]: reduction type" << std::endl;
        hasError = 1;
    }
    if (setting1.getSwapType() != swapType) {
        std::cerr << "[Error]: swap type" << std::endl;
        hasError = 1;
    }
    if (setting1.getCompType() != compSet) {
        std::cerr << "[Error]: complement set" << std::endl;
        hasError = 1;
    }
    if (setting1.getMergeType() != mergeType) {
        std::cerr << "[Error]: merge type" << std::endl;
        hasError = 1;
    }
    if (setting1.getName() != name) {
        std::cerr << "[Error]: name" << std::endl;
        hasError = 1;
    }
    return hasError;
}