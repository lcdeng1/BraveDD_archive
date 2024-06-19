#include "setting.h"
#include <regex>

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      VarDomain methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

BRAVE_DD::VarDomain::VarDomain(unsigned size)
{
    maxLevel = size;
    // ordering TBD
}
BRAVE_DD::VarDomain::~VarDomain()
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

BRAVE_DD::ForestSetting::ForestSetting(const unsigned numVals)
{
    // User defined number of variables
    domain = new VarDomain(numVals);
    // default settings: RexBDD
    range = new Range(BOOLEAN, VOID);
    reductions = new Reductions(REX);
    flags = new Flags(ONE, COMP);
    encodingType = TERMINAL;
    mergeType = PUSH_UP;
    name = "RexBDD";
}

BRAVE_DD::ForestSetting::ForestSetting(const std::string& bdd, const unsigned numVals, const bool isRel)
{
    // BDDs
    if (std::regex_match(bdd, std::regex("^(R|r)(E|e)(X|x)(B|b)(D|d)(D|d)$"))) {
        // setting for RexBDD
    } else if (std::regex_match(bdd, std::regex("^(M|m)(T|t)(B|b)(D|d)(D|d)$"))) {
        // setting for MTBDD
    } else if (std::regex_match(bdd, std::regex("^(F|f)(B|b)(D|d)(D|d)$"))) {
        // setting for FBDD
    } else if (std::regex_match(bdd, std::regex("^(E|e)(V|v)(\\+)?(B|b)(D|d)(D|d)$"))) {
        // setting for EV+BDD
    } else if (std::regex_match(bdd, std::regex("^(E|e)(V|v)(\\+)?(?:(\\%)|mod)(B|b)(D|d)(D|d)$"))) {
        // setting for EV%BDD
    } else if (std::regex_match(bdd, std::regex("^(E|e)(V|v)(?:(\\*)|star)(B|b)(D|d)(D|d)$"))) {
        // setting for EV*BDD
    }
    // MxDs
    if (std::regex_match(bdd, std::regex("^(M|m)(T|t)(B|b)(M|m)(X|x)(D|d)$"))) {
        // setting for MTBMxD
    } else if (std::regex_match(bdd, std::regex("^(E|e)(V|v)(\\+)?(B|b)(M|m)(X|x)(D|d)$"))) {
        // setting for EV+BMxD
    } else if (std::regex_match(bdd, std::regex("^(E|e)(V|v)(\\+)?(?:(\\%)|mod)(B|b)(M|m)(X|x)(D|d)$"))) {
        // setting for EV%BMxD
    } else if (std::regex_match(bdd, std::regex("^(E|e)(V|v)(?:(\\*)|star)(B|b)(M|m)(X|x)(D|d)$"))) {
        // setting for EV*BMxD
    } else {
        // Unknown predefined BDD or BMxD
    }
}

BRAVE_DD::ForestSetting::~ForestSetting()
{
    delete domain;
    delete range;
    delete reductions;
    delete flags;
}