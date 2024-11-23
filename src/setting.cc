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
    std::string rangeType;
    if (rt == BOOLEAN) {
        rangeType = "Boolean";
    } else if (rt == FINITE) {
        rangeType = "Finite: N = ";
        rangeType += std::to_string(getMaxRange());
    } else if (rt == NNINTEGER) {
        rangeType = "Non-negative Integer";
    } else if (rt == INTEGER) {
        rangeType = "Integer";
    } else if (rt == NNREAL) {
        rangeType = "Non-negative Real";
    } else if (rt == REAL) {
        rangeType = "Real";
    } else {
        rangeType = "Unknown";
    }
    out<<"\tRange type:\t\t"<<rangeType<<std::endl;
    // range value type
    ValueType vt = getValType();
    std::string valueType;
    if (vt == INT) {
        valueType = "int";
    } else if (vt == LONG) {
        valueType = "long";
    } else if (vt == FLOAT) {
        valueType = "float";
    } else if (vt == DOUBLE) {
        valueType = "double";
    } else {
        valueType = "Unknown";
    }
    out<<"\tRange value type:\t"<<valueType<<std::endl;
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
    ReductionType rdt = getReductionType();
    std::string redType;
    if (rdt == QUASI) {
        redType = "Quasi";
    } else if (rdt == FULLY) {
        redType = "Fully";
    } else if (rdt == REX) {
        redType = "Rex";
    } else if (rdt == USER_DEFINED) {
        redType = "User-Defined";
    } else if (rdt == QUASI_QUASI) {
        redType = "Quasi-Quasi";
    } else if (rdt == FULLY_FULLY) {
        redType = "Fully-Fully";
    } else if (rdt == FULLY_IDENTITY) {
        redType = "Fully-Identity";
    } else {
        redType = "Unknown";
    }
    out<<"\tReduction type:\t\t"<<redType<<std::endl;
    // reduction rules
    out<<"\tNumber of rules:\t"<<getReductionSize()<<std::endl;
    std::string rules = "";
    if (hasReductionRule(RULE_X)) rules += "X  ";
    if (hasReductionRule(RULE_EL0)) rules += "EL0  ";
    if (hasReductionRule(RULE_EL1)) rules += "EL1  ";
    if (hasReductionRule(RULE_EH0)) rules += "EH0  ";
    if (hasReductionRule(RULE_EH1)) rules += "EH1  ";
    if (hasReductionRule(RULE_AL0)) rules += "AL0  ";
    if (hasReductionRule(RULE_AL1)) rules += "AL1  ";
    if (hasReductionRule(RULE_AH0)) rules += "AH0  ";
    if (hasReductionRule(RULE_AH1)) rules += "AH1  ";
    if (hasReductionRule(RULE_I0)) rules += "I0  ";
    if (hasReductionRule(RULE_I1)) rules += "I1";
    out<<"\tReduction rules:\t"<<rules<<std::endl;
    // flags
    SwapSet st = getSwapType();
    std::string swapType;
    if (st == NO_SWAP) swapType = "No swap";
    if (st == ONE) swapType = "Swap-one";
    if (st == ALL) swapType = "Swap-all";
    if (st == FROM) swapType = "Swap-from";
    if (st == TO) swapType = "Swap-to";
    if (st == FROM_TO) swapType = "Swap-from_to";
    out<<"\tSwap type:\t\t"<<swapType<<std::endl;
    CompSet ct = getCompType();
    std::string compType;
    if (ct==NO_COMP) compType = "No";
    if (ct==COMP) compType = "YES";
    out<<"\tComplement allowed:\t"<<compType<<std::endl;
    // encoding type
    EncodeMechanism em = getEncodeMechanism();
    std::string emType;
    if (em == TERMINAL) {
        emType = "Terminal";
    } else if (em == EDGE_PLUS) {
        emType = "Edge plus (EV+)";
    } else if (em == EDGE_PLUSMOD) {
        emType = "Edge plus mod (EV%)";
    } else if (em == EDGE_MULT) {
        emType = "Edge multiply (EV*)";
    } else {
        emType = "Unknown";
    }
    out<<"\tEncoding Mechanism:\t"<<emType<<std::endl;
    // merge type
    MergeType mt = getMergeType();
    std::string mType;
    if (mt == PUSH_UP) {
        mType = (isRelation())?"ShortenX: true; ShortenI: true":"Push up";
    } else if (mt == PUSH_DOWN) {
        mType = (isRelation())?"ShortenX: false; ShortenI: false":"Push down";
    } else if (mt == SHORTEN_I) {
        mType = (isRelation())?"ShortenX: false; ShortenI: true":"Unknown";
    } else if (mt == SHORTEN_X) {
        mType = (isRelation())?"ShortenX: true; ShortenI: false":"Unknown";
    } else {
        mType = "Unknown";
    }
    out<<"\tMege type:\t\t"<<mType<<std::endl;
    out<<"============================ Settings End ==========================="<<std::endl;
}