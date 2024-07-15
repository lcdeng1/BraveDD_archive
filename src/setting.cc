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
:domain(numVals), range(BOOLEAN, VOID), reductions(REX), flags(ONE, COMP)
{
    // User defined number of variables
    // default settings: RexBDD
    encodingType = TERMINAL;
    mergeType = PUSH_UP;
    name = "RexBDD";
}

BRAVE_DD::ForestSetting::ForestSetting(const std::string& bdd, const unsigned numVals)
:domain(numVals), range(BOOLEAN, VOID), reductions(REX), flags(ONE, COMP)
{
    // BDDs
    if (std::regex_match(bdd, std::regex("^(R|r)(E|e)(X|x)(B|b)(D|d)(D|d)$"))) {
        // setting for RexBDD
        encodingType = TERMINAL;
        mergeType = PUSH_UP;
        name = "RexBDD";
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
    else if (std::regex_match(bdd, std::regex("^(M|m)(T|t)(B|b)(M|m)(X|x)(D|d)$"))) {
        // setting for MTBMxD
    } else if (std::regex_match(bdd, std::regex("^(E|e)(V|v)(\\+)?(B|b)(M|m)(X|x)(D|d)$"))) {
        // setting for EV+BMxD
    } else if (std::regex_match(bdd, std::regex("^(E|e)(V|v)(\\+)?(?:(\\%)|mod)(B|b)(M|m)(X|x)(D|d)$"))) {
        // setting for EV%BMxD
    } else if (std::regex_match(bdd, std::regex("^(E|e)(V|v)(?:(\\*)|star)(B|b)(M|m)(X|x)(D|d)$"))) {
        // setting for EV*BMxD
    } else {
        // Unknown predefined BDD or BMxD
        std::cout << "[BRAVE_DD] ERROR!\t Unknown BDD/MxD: " << bdd << std::endl;
        exit(0);
    }
}

BRAVE_DD::ForestSetting::~ForestSetting()
{
    //
}