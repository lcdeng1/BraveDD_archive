#include "reductions.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                     Reductions methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

Reductions::Reductions(const ReductionType reductionType)
{
    dimension = 1;
    type = reductionType;
    if (type == QUASI){
        rules = std::vector<bool>(0,0);
    } else if (type == FULLY) {
        // Only apply reduction rule X
        rules = std::vector<bool>(1,1);
    } else if (type == REX) {
        // Apply all 9 rex reduction rules
        rules = std::vector<bool>(9,1);
    } else if (type == QUASI_QUASI) {
        dimension = 2;
        rules = std::vector<bool>(0,0);
    } else if (type == FULLY_FULLY) {
        dimension = 2;
        // Only apply reduction rule X
        rules = std::vector<bool>(1,1);
    } else if (type == FULLY_IDENTITY) {
        dimension = 2;
        // Apply reduction rules: I0 (0), X (1)
        rules = std::vector<bool>(2,1);
    } else {
        // Error for USER_DEFINED
        throw error(ErrCode::UNINITIALIZED, __FILE__, __LINE__);
    }

}
Reductions::Reductions(const std::vector<bool> &ruleSet)
{
    dimension = 1;
}
Reductions::~Reductions()
{
    //
}