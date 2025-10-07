#ifndef BRAVE_DD_APPLY_H
#define BRAVE_DD_APPLY_H

#include "../defines.h"
#include "../setting.h"
#include "../forest.h"
#include "operation.h"
#include "operations_generator.h"

namespace BRAVE_DD {
    /* Unary */
    typedef UnaryOperation* (*UnaryBuiltin1)(Forest* arg, Forest* res);
    typedef UnaryOperation* (*UnaryBuiltin2)(Forest* arg, OpndType res);
    /* Binary */
    typedef BinaryOperation* (*BinaryBuiltin1)(Forest* arg1, Forest* arg2, Forest* res);
    typedef BinaryOperation* (*BinaryBuiltin2)(Forest* arg1, OpndType arg2, Forest* res);
    /* Saturation */
    typedef SaturationOperation* (*SaturationBuiltin)(Forest* arg1, Forest* arg2, Forest* res);

    // ******************************************************************
    // *                          Unary  apply                          *
    // ******************************************************************
    inline void apply(UnaryBuiltin1 ub, const Func& arg, Func& res)
    {
        UnaryOperation* uop = ub(arg.getForest(), res.getForest());
        uop->compute(arg, res);
    }
    inline void apply(UnaryBuiltin2 ub, const Func& arg, long& res)
    {
        UnaryOperation* uop = ub(arg.getForest(), OpndType::INTEGER);
        uop->compute(arg, res);
    }
    inline void apply(UnaryBuiltin2 ub, const Func& arg, double& res)
    {
        UnaryOperation* uop = ub(arg.getForest(), OpndType::REAL);
        uop->compute(arg, res);
    }
    // for concretizing with don't care value
    inline void apply(UnaryBuiltin1 ub, const Func& arg0, const Func& arg1, Func& res)
    {
        UnaryOperation* uop = ub(arg0.getForest(), res.getForest());
        uop->compute(arg0, arg1, res);
    }
    inline void apply(UnaryBuiltin1 ub, const Func& arg, Value val, Func& res)
    {
        UnaryOperation* uop = ub(arg.getForest(), res.getForest());
        uop->compute(arg, val, res);
    }
    // ******************************************************************
    // *                         Binary  apply                          *
    // ******************************************************************
    inline void apply(BinaryBuiltin1 bb, const Func& arg1, const Func& arg2, Func& res)
    {
        BinaryOperation* bop = bb(arg1.getForest(), arg2.getForest(), res.getForest());
        bop->compute(arg1, arg2, res);
    }
    inline void apply(BinaryBuiltin2 bb, const Func& arg1, const ExplictFunc& arg2, Func& res)
    {
        BinaryOperation* bop = bb(arg1.getForest(), OpndType::EXPLICIT_FUNC, res.getForest());
        bop->compute(arg1, arg2, res);
    }
    // ******************************************************************
    // *                     Saturation  apply                          *
    // ******************************************************************
    inline void apply(SaturationBuiltin sb, const Func& set, const std::vector<Func>& relations, Func& res)
    {
        SaturationOperation* sop = sb(set.getForest(), relations[0].getForest(), res.getForest());
        // set the relations used for this operation
        sop->setRelations(relations);
        sop->compute(set, res);
    }
};

#endif