#include "reductions.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                     Reductions methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

Reductions::Reductions()
{
    dimension = 1;
    type = REX;
    rules = std::vector<bool>(13,1);
    rules[RULE_I0] = 0;
    rules[RULE_I1] = 0;
    rules[RULE_OR] = 0;
    rules[RULE_AND]= 0;
}
Reductions::Reductions(const ReductionType reductionType)
{
    setType(reductionType);
}
Reductions::Reductions(const std::vector<bool> &ruleSet)
{
    dimension = 1;
}
Reductions::~Reductions()
{
    //
}