#ifndef BRAVE_DD_REDUCTIONS_H
#define BRAVE_DD_REDUCTIONS_H

#include "../defines.h"
#include "../error.h"

namespace BRAVE_DD {
    ///  Reduction type
    enum ReductionType{
        QUASI,              // -+
        FULLY,              //  +---- only applicable if variable dimension is 1
        REX,                // -+
        USER_DEFINED,       //        only if dimension is 1: user-defined combinations of reductions
        QUASI_QUASI,        // -+
        FULLY_FULLY,        //  +---- only applicable if variable dimension is 2
        FULLY_IDENTITY      // -+
    };
    static inline std::string reductionType2String(ReductionType rdt) {
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
        return redType;
    }
    //--------------------------+---------------------------+-----------------|
    // EL0    AL0 |  EL1    AL1 |  EH0    AH0 |  EH1    AH1 |  I0    X    I1
    //------------+-------------+-------------+-------------+-----------------|
    // 0      0   |  0      0   |  0      0   |  0      0   |  1     1    1
    // 0      0   |  0      0   |  1      1   |  1      1   |  0     0    0
    // 0      0   |  1      1   |  0      0   |  1      1   |  0     0    1
    // 0      1   |  0      1   |  0      1   |  0      1   |  0     1    0
    //------------+-------------+-------------+-------------+-----------------|
    // 0      1   |  2      3   |  4      5   |  6      7   |  8     9    10
    //--------------------------+---------------------------+-----------------|
    // Note:
    //      check X: rule == 1001; 
    //  otherwise
    //      check relation: rule > 7;
    //      check (?)1: rule & (0010);
    //  Complement rule:
    //      rule == X ? X :
    //          (rule & (0010)) ? (rule & (1101)) : (rule | (0010));
    //  Swap rule:
    //      rule >7 ? rule :
    //          (rule & (0010)) ? (rule & (1011)) : (rule | (0100));

    /// Reduction rule
    typedef enum {
        RULE_EL0 = 0,
        RULE_AL0 = 1,
        RULE_EL1 = 2,
        RULE_AL1 = 3,
        RULE_EH0 = 4,
        RULE_AH0 = 5,
        RULE_EH1 = 6,
        RULE_AH1 = 7,
        RULE_I0  = 8,
        RULE_X   = 9,
        RULE_I1  = 10
    } ReductionRule;
    static inline std::string rule2String(ReductionRule rule) {
        switch (rule) {
            case RULE_EL0: return "EL0";
            case RULE_AL0: return "AL0";
            case RULE_EL1: return "EL1";
            case RULE_AL1: return "AL1";
            case RULE_EH0: return "EH0";
            case RULE_AH0: return "AH0";
            case RULE_EH1: return "EH1";
            case RULE_AH1: return "AH1";
            case RULE_I0:  return "I0";
            case RULE_X:   return "X";
            case RULE_I1:  return "I1";
            default: return "UNKNOWN_RULE";
        }
    }
    /// Reduction type and set of rules
    class Reductions;
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                         Reductions class                       *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::Reductions {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
        Reductions(const ReductionType reductionType);
        Reductions(const std::vector<bool>& ruleSet);
        ~Reductions();
        //******************************************
        //  Getters
        //******************************************
        /* Dimension */
        /// Get the dimension
        inline int getDim() const {return dimension;}
        /// Get the type of reduction
        inline ReductionType getType() const {return type;}
        /// Get the size of reduction rules set
        inline int getNumRules() const {return std::count(rules.begin(),rules.end(), 1);}
        /// Check if the given ReductionRule has been set
        inline bool hasRule(ReductionRule rule) const {
            if (type == QUASI || type == QUASI_QUASI) return 0;
            if (type == FULLY || type == FULLY_FULLY) return rule == RULE_X;
            if (type == REX) return (RULE_EL0<=rule && rule<=RULE_AH1) || rule == RULE_X;
            if (type == FULLY_IDENTITY) return RULE_I0<=rule && rule<=((getNumRules()==2)?RULE_X:RULE_I1);
            if (type == USER_DEFINED) return rules[rule];
            return 0;
        };
        // more checkers? TBD
        //******************************************
        //  Setters
        //******************************************
        inline void setDim(const int dim) {dimension = dim;}
        inline void setType(const ReductionType reductionType) {type = reductionType;}
        inline void setRules(const std::vector<bool>& ruleSet) {
            if (type == USER_DEFINED) {
                // the rules can only be changed if the type is USER_DEFINED.
                rules = ruleSet;
            } else {
                throw error(BRAVE_DD::ErrCode::ALREADY_INITIALIZED, __FILE__, __LINE__);
            }
        }
        
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
        int                 dimension;  // 1: set/vector function; 2: relation/matrix function
        ReductionType       type;       // type of reductions
        std::vector<bool>   rules;      // set of allowed reduction rules
};


#endif