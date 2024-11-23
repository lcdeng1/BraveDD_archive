#include "setting.h"
#include <regex>

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                      VarDomain methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

VarDomain::VarDomain(uint16_t size)
{
    maxLevel = size;
    // ordering TBD
}
VarDomain::~VarDomain()
{
    //
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                        Setting methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

ForestSetting::ForestSetting(const unsigned numVals)
:domain(numVals), range(BOOLEAN, INT), reductions(REX), flags(ONE, COMP)
{
    // User defined number of variables
    // default settings: RexBDD
    encodingType = TERMINAL;
    mergeType = PUSH_UP;
    name = "RexBDD";
}

ForestSetting::ForestSetting(const std::string& bdd, const unsigned numVals)
:domain(numVals), range(BOOLEAN, INT), reductions(REX), flags(ONE, COMP)
{
    // convert to all lower case
    std::string bddLower;
    bddLower.resize(bdd.size());
    std::transform(bdd.begin(), bdd.end(), bddLower.begin(), ::tolower);
    // BDDs
    if (bddLower == "rexbdd") {
        // setting for RexBDD
        encodingType = TERMINAL;
        mergeType = PUSH_UP;
        name = "RexBDD";
    } else if (bddLower == "qbdd") {
        // setting for QBDD
    } else if (bddLower == "fbdd") {
        // setting for FBDD
    } else if (bddLower == "mtbdd") {
        // setting for MTBDD
    } else if (bddLower == "evbdd" || bddLower == "ev+bdd") {
        // setting for EV+BDD
    } else if (bddLower == "ev%bdd" || bddLower == "evmodbdd") {
        // setting for EV%BDD
    } else if (bddLower == "ev*bdd") {
        // setting for EV*BDD
    }
    // MxDs
    else if (bddLower == "mtbmxd") {
        // setting for MTBMxD
    } else if (bddLower == "evbmxd" || bddLower == "ev+bmxd") {
        // setting for EV+BMxD
    } else if (bddLower == "ev%bmxd" || bddLower == "evmodbmxd") {
        // setting for EV%BMxD
    } else if (bddLower == "ev*bmxd") {
        // setting for EV*BMxD
    } else {
        // Unknown predefined BDD or BMxD
        std::cout << "[BRAVE_DD] ERROR!\t Unknown BDD/MxD: " << bdd << std::endl;
        exit(0);
    }
}

ForestSetting::~ForestSetting()
{
    //
}

void ForestSetting::exportSetting(std::ostream& out)
{
    // name 
    out<<"============================== Settings ============================"<<std::endl;
    out<<"\tName:\t\t\t"<<name<<std::endl;
    // number of variables
    out<<"\tNumber of variables:\t"<<getNumVars()<<std::endl;
    // relation or set
    std::string isRel = isRelation()?"Relation":"Set";
    out<<"\tEncoding purpose:\t"<<isRel<<std::endl;
    // range type
    RangeType rt = getRangeType();
    std::string rangeType = rangeType2String(rt);
    if (rt == FINITE) {
        rangeType += ": N = ";
        rangeType += std::to_string(getMaxRange());
    }
    out<<"\tRange type:\t\t"<<rangeType<<std::endl;
    // range value type
    out<<"\tRange value type:\t"<<valueType2String(getValType())<<std::endl;
    // special values
    std::string specialVal;
    if (!hasNegInf()&&!hasPosInf()&&!hasUnDef()) {
        specialVal = ":\tNO Special";
    } else {
        specialVal = ":\t";
        if (hasNegInf()) specialVal += "NegInf  ";
        if (hasPosInf()) specialVal += "PosInf  ";
        if (hasUnDef()) specialVal += "UnDef  ";
    }
    out<<"\tRange special values"<<specialVal<<std::endl;
    // reduction type
    out<<"\tReduction type:\t\t"<<reductionType2String(getReductionType())<<std::endl;
    // reduction rules
    out<<"\tNumber of rules:\t"<<getReductionSize()<<std::endl;
    std::string rules = "";
    for (int rule=0; rule<=10; rule++) {
        if (hasReductionRule((ReductionRule)rule)) {
            rules += rule2String((ReductionRule)rule);
            rules += "  ";
        }
    }
    out<<"\tReduction rules:\t"<<rules<<std::endl;
    // flags
    out<<"\tSwap type:\t\t"<<swapSet2String(getSwapType())<<std::endl;
    out<<"\tComplement allowed:\t"<<compSet2String(getCompType())<<std::endl;
    // encoding type
    out<<"\tEncoding Mechanism:\t"<<encodeMechanism2String(getEncodeMechanism())<<std::endl;
    // merge type
    out<<"\tMege type:\t\t"<<mergeType2String(getMergeType(), isRelation())<<std::endl;
    out<<"============================ Settings End ==========================="<<std::endl;
}