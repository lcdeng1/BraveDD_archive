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
:domain(numVals), range(BOOLEAN, VOID), reductions(REX), flags(ONE, COMP)
{
    // User defined number of variables
    // default settings: RexBDD
    encodingType = TERMINAL;
    mergeType = PUSH_UP;
    name = "RexBDD";
}

ForestSetting::ForestSetting(const std::string& bdd, const unsigned numVals)
:domain(numVals), range(BOOLEAN, VOID), reductions(REX), flags(ONE, COMP)
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
    } else if (std::regex_match(bdd, std::regex("fbdd", std::regex_constants::icase))) {
        // setting for FBDD
    } else if (std::regex_match(bdd, std::regex("mtbdd", std::regex_constants::icase))) {
        // setting for MTBDD
    } else if (std::regex_match(bdd, std::regex("ev(\\+)?bdd", std::regex_constants::icase))) {
        // setting for EV+BDD
    } else if (std::regex_match(bdd, std::regex("ev(\\+)?(?:(\\%)|mod)bdd", std::regex_constants::icase))) {
        // setting for EV%BDD
    } else if (std::regex_match(bdd, std::regex("ev(?:(\\*)|star)bdd", std::regex_constants::icase))) {
        // setting for EV*BDD
    }
    // MxDs
    else if (std::regex_match(bdd, std::regex("mtbmxd", std::regex_constants::icase))) {
        // setting for MTBMxD
    } else if (std::regex_match(bdd, std::regex("ev(\\+)?bmxd", std::regex_constants::icase))) {
        // setting for EV+BMxD
    } else if (std::regex_match(bdd, std::regex("ev(\\+)?(?:(\\%)|mod)bmxd", std::regex_constants::icase))) {
        // setting for EV%BMxD
    } else if (std::regex_match(bdd, std::regex("ev(?:(\\*)|star)bmxd", std::regex_constants::icase))) {
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