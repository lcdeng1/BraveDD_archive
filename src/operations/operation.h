#ifndef BRAVE_DD_OPERATION_H
#define BRAVE_DD_OPERATION_H

#include "../defines.h"
#include "../forest.h"

namespace BRAVE_DD {
    class Operation;
    /// Argument and result types for apply operations.
    enum class OpndType {
        FOREST          = 0,
        BOOLEAN         = 1,
        INTEGER         = 2,
        REAL            = 3,
        HUGEINT         = 4,
        FLOATVECT       = 5,
        DOUBLEVECT      = 6,
        EXPLICIT_FUNC   = 7
    };
    /// Built-in Unary operation type
    enum class UnaryOperationType{
        UOP_COPY,
        UOP_CARDINALITY,
        UOP_MASS,
        UOP_HIGHEST,
        UOP_LOWEST,
        UOP_COMPLEMENT,
        UOP_CONCRETIZE_RST,
        UOP_CONCRETIZE_OSM,
        UOP_CONCRETIZE_TSM,
        UOP_EQUANTIFY,
        UOP_UQUANTIFY,
        UOP_REORDER
    };
    class UnaryOperation;
    class UnaryList;
    /// Built-in Binary operation type
    enum class BinaryOperationType{
        BOP_UNION,
        BOP_INTERSECTION,
        BOP_DIFFERENCE,
        BOP_MINIMUM,
        BOP_MAXIMUM,
        BOP_PLUS,
        BOP_MINUS,
        BOP_MULTIPLY,
        BOP_DIVIDE,
        BOP_EQUAL,
        BOP_NOTEQUAL,
        BOP_LESSTHAN,
        BOP_LESSTHANEQ,
        BOP_GREATERTHAN,
        BOP_GREATERTHANEQ,
        BOP_CROSS,
        BOP_PREPLUS,
        BOP_POSTPLUS,
        BOP_PREIMAGE,
        BOP_POSTIMAGE,
        BOP_VM,
        BOP_MV,
        BOP_MM
    };
    class BinaryOperation;
    class BinaryList;

    /// Numerical operation

    class NumericalOperation;
    class NumericalList;

    /// Saturation

    class SaturationOperation;
    class SaturationList;

    extern UnaryList UOPs;
    extern BinaryList BOPs;
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                       Operation class                          *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::Operation {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    Operation();
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~Operation();
    // computing tables TBD

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/

};

// ******************************************************************
// *                                                                *
// *                    UnaryOperation  class                       *
// *                                                                *
// ******************************************************************

class BRAVE_DD::UnaryOperation : public Operation {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    UnaryOperation(UnaryList& owner, UnaryOperationType type, Forest* source, Forest* target);
    UnaryOperation(UnaryList& owner, UnaryOperationType type, Forest* source, OpndType target);


    /* Main part: check forest comatability and then calls corresponding op compute */
    void compute(const Func& source, Func& target);
    void compute(const Func& source, long& target);
    void compute(const Func& source, double& target);
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~UnaryOperation();
    // computing tables TBD

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// Helper Methods ==============================================
    bool checkForestCompatibility() const;
    Edge computeCOPY(const Edge& source);
    Edge computeCOMPLEMENT(const Edge& source);
    long computeCARD(const Edge& source);
    // list
    friend class UnaryList;
    UnaryList&          parent;
    UnaryOperation*     next;
    // arguments
    Forest*             sourceForest;
    Forest*             targetForest;
    OpndType            targetType;
    UnaryOperationType  opType;
};

// ******************************************************************
// *                                                                *
// *                       UnaryList  class                         *
// *                                                                *
// ******************************************************************

class BRAVE_DD::UnaryList {
    std::string name;
    UnaryOperation* front;
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    UnaryList(const std::string n = "");
    inline void reset(const std::string n) {
        front = nullptr;
        name = n;
    }
    inline std::string getName() const {return name;}
    inline bool isEmpty() const {return !front;}
    inline UnaryOperation* add(UnaryOperation* uop) {
        if (uop) {
            uop->next = front;
            front = uop;
        }
        return uop;
    }
    inline void remove(UnaryOperation* uop) {
        if (front == uop) {
            front = front->next;
            return;
        }
        searchRemove(uop);
    }
    inline UnaryOperation* find(const UnaryOperationType opT, const Forest* sourceF, const Forest* targetF) {
        if (!front) return nullptr;
        if ((front->opType == opT) && (front->sourceForest == sourceF) && (front->targetForest == targetF)) return front;
        return mtfUnary(opT, sourceF, targetF);
    }
    inline UnaryOperation* find(const UnaryOperationType opT, const Forest* sourceF, const OpndType targetT) {
        if (!front) return nullptr;
        if ((front->opType == opT) && (front->sourceForest == sourceF) && (front->targetType == targetT)) return front;
        return mtfUnary(opT, sourceF, targetT);
    }
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    void searchRemove(UnaryOperation* uop);
    UnaryOperation* mtfUnary(const UnaryOperationType opT, const Forest* sourceF, const Forest* targetF);
    UnaryOperation* mtfUnary(const UnaryOperationType opT, const Forest* sourceF, const OpndType targetT);

};

// ******************************************************************
// *                                                                *
// *                   BinaryOperation  class                       *
// *                                                                *
// ******************************************************************

class BRAVE_DD::BinaryOperation : public Operation {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    BinaryOperation(BinaryList& owner, BinaryOperationType type, Forest* arg1, Forest* arg2, Forest* res);

    /* Main part: computation */
    void compute(const Func& arg1, const Func& arg2, Func& res);
    void compute(const Func& arg1, const ExplictFunc arg2, Func& res);
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~BinaryOperation();
    // computing tables TBD

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// Helper Methods ==============================================
    bool checkForestCompatibility() const;
    Edge computeUNION(const Edge& arg1, const Edge& arg2);
    Edge computeINTERSECTION(const Edge& arg1, const Edge& arg2);
    // list
    friend class BinaryList;
    BinaryList&         parent;
    BinaryOperation*    next;
    // arguments
    Forest*             arg1Forest;
    Forest*             arg2Forest;
    OpndType            arg2Type;
    Forest*             resForest;
    BinaryOperationType opType;
    bool                canCommute;
};

// ******************************************************************
// *                                                                *
// *                       BinaryList  class                         *
// *                                                                *
// ******************************************************************

class BRAVE_DD::BinaryList {
    std::string name;
    BinaryOperation* front;
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    BinaryList(const std::string n = "");
    inline void reset(const std::string n) {
        front = nullptr;
        name = n;
    }
    inline std::string getName() const {return name;}
    inline bool isEmpty() const {return !front;}
    inline BinaryOperation* add(BinaryOperation* bop) {
        if (bop) {
            bop->next = front;
            front = bop;
        }
        return bop;
    }
    inline void remove(BinaryOperation* bop) {
        if (front == bop) {
            front = front->next;
            return;
        }
        searchRemove(bop);
    }
    inline BinaryOperation* find(const Forest* arg1F, const Forest* arg2F, const Forest* resF) {
        if (!front) return nullptr;
        if ((front->arg1Forest == arg1F) && (front->arg2Forest == arg2F) && (front->resForest == resF)) return front;
        return mtfUnary(arg1F, arg2F, resF);
    }
    inline BinaryOperation* find(const Forest* arg1F, const OpndType arg2T, const Forest* resF) {
        if (!front) return nullptr;
        if ((front->arg1Forest == arg1F) && (front->arg2Type == arg2T) && (front->resForest == resF)) return front;
        return mtfUnary(arg1F, arg2T, resF);
    }
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    void searchRemove(BinaryOperation* bop);
    BinaryOperation* mtfUnary(const Forest* arg1F, const Forest* arg2F, const Forest* resF);
    BinaryOperation* mtfUnary(const Forest* arg1F, const OpndType arg2T, const Forest* resF);

};

// ******************************************************************
// *                                                                *
// *                NumericalOperation  class                       *
// *                                                                *
// ******************************************************************

class BRAVE_DD::NumericalOperation : public Operation {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    NumericalOperation();
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~NumericalOperation();
    // computing tables TBD

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
};

// ******************************************************************
// *                                                                *
// *                SaturationOperation  class                       *
// *                                                                *
// ******************************************************************

class BRAVE_DD::SaturationOperation : public Operation {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    SaturationOperation();
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~SaturationOperation();
    // computing tables TBD

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
};


#endif