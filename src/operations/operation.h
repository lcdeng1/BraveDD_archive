#ifndef BRAVE_DD_OPERATION_H
#define BRAVE_DD_OPERATION_H

#include "../defines.h"
#include "../forest.h"
#include "compute_table.h"

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
    static inline std::string BOP2String(BinaryOperationType bop) {
        std::string optype;
        switch (bop)
        {
        case BinaryOperationType::BOP_UNION:
            optype = "Union";
            break;
        case BinaryOperationType::BOP_INTERSECTION:
            optype = "Intersection";
            break;
        case BinaryOperationType::BOP_MINIMUM:
            optype = "Minimum";
            break;
        case BinaryOperationType::BOP_MAXIMUM:
            optype = "Maximum";
            break;
        case BinaryOperationType::BOP_PLUS:
            optype = "Plus";
            break;
        case BinaryOperationType::BOP_PREIMAGE:
            optype = "PreImage";
            break;
        case BinaryOperationType::BOP_POSTIMAGE:
            optype = "PostImage";
            break;
        
        default:
            optype = "Unknown";
            break;
        }
        return optype;
    }
    class BinaryOperation;
    class BinaryList;

    /// Numerical operation
    // class NumericalOperation;
    // class NumericalList;

    /// Saturation
    class SaturationOperation;
    class SaturationList;

    extern UnaryList        UOPs;
    extern BinaryList       BOPs;
    extern SaturationList   SOPs;
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
    ComputeTable        cache;

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
    UnaryOperation(UnaryOperationType type, Forest* source, Forest* target);
    UnaryOperation(UnaryOperationType type, Forest* source, OpndType target);


    /* Main part: check forest comatability and then calls corresponding op compute */
    void compute(const Func& source, Func& target);
    void compute(const Func& source, long& target);
    void compute(const Func& source, double& target);
    void compute(const Func& source, Func& target, const Value val);
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~UnaryOperation();

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// Helper Methods ==============================================
    bool checkForestCompatibility() const;
    Edge computeCOPY(const uint16_t lvl, const Edge& source);
    Edge computeCOMPLEMENT(const uint16_t lvl, const Edge& source);
    long computeCARD(const uint16_t lvl, const Edge& source);
    // concretizing
    Edge computeRESTRICT(const uint16_t lvl, const Edge& source, const Value val);
    Edge computeOSM(const uint16_t lvl, const Edge& source, const Value val);
    Edge computeTSM(const uint16_t lvl, const Edge& source, const Value val);
    // used for concretizing
    unsigned char compareOSM(const Edge& e1, const Edge& e2);
    bool hasCommonTSM(const Edge& e1, const Edge& e2);
    Edge commonTSM(const uint16_t lvl, const Edge& e1, const Edge& e2);
    // list
    friend class UnaryList;
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
            UnaryOperation* toRemove = front;
            front = front->next;
            delete toRemove;
            return;
        }
        searchRemove(uop);
    }
    // find and remove the operation including the given forest
    inline void remove(Forest* forest) { searchRemove(forest); }
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
    inline void sweepCache(Forest* forest) { searchSweepCache(forest); }
    void reportCacheStat(std::ostream& out, int format=0) const;
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    void searchRemove(UnaryOperation* uop);
    void searchRemove(Forest* forest);
    void searchSweepCache(Forest* forest);
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
    BinaryOperation(BinaryOperationType type, Forest* source1, Forest* source2, Forest* res);
    BinaryOperation(BinaryOperationType type, Forest* source1, OpndType source2, Forest* res);

    /* Main part: computation */
    void compute(const Func& source1, const Func& source2, Func& res);
    void compute(const Func& source1, const ExplictFunc& source2, Func& res);
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~BinaryOperation();

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// Helper Methods ==============================================
    bool checkForestCompatibility() const;
    Edge computeElmtWise(const uint16_t lvl, const Edge& source1, const Edge& source2);
    Edge computeUnion(const uint16_t lvl, const Edge& source1, const Edge& source2);
    Edge computeIntersection(const uint16_t lvl, const Edge& source1, const Edge& source2);
    Edge computeImage(const uint16_t lvl, const Edge& source1, const Edge& trans, bool isPre = 0);
    Edge computePlus(const uint16_t lvl, const Edge& source1, const Edge& source2);
    // elementwise related
    Edge operateLL(const uint16_t lvl, const Edge& e1, const Edge& e2);
    Edge operateHH(const uint16_t lvl, const Edge& e1, const Edge& e2);
    Edge operateLH(const uint16_t lvl, const Edge& e1, const Edge& e2);
    // list
    friend class BinaryList;
    BinaryOperation*    next;
    friend class SaturationOperation;
    // arguments
    Forest*             source1Forest;
    Forest*             source2Forest;
    OpndType            source2Type;
    Forest*             resForest;
    BinaryOperationType opType;
};

// ******************************************************************
// *                                                                *
// *                       BinaryList  class                        *
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
            BinaryOperation* toRemove = front;
            front = front->next;
            delete toRemove;
            return;
        }
        searchRemove(bop);
    }
    // find and remove the operation including the given forest
    inline void remove(Forest* forest) { searchRemove(forest); }
    inline BinaryOperation* find(const BinaryOperationType opT, const Forest* source1F, const Forest* source2F, const Forest* resF) {
        if (!front) return nullptr;
        if ((front->opType == opT) && (front->source1Forest == source1F) && (front->source2Forest == source2F) && (front->resForest == resF)) return front;
        return mtfBinary(opT, source1F, source2F, resF);
    }
    inline BinaryOperation* find(const BinaryOperationType opT, const Forest* source1F, const OpndType source2T, const Forest* resF) {
        if (!front) return nullptr;
        if ((front->opType == opT) && (front->source1Forest == source1F) && (front->source2Type == source2T) && (front->resForest == resF)) return front;
        return mtfBinary(opT, source1F, source2T, resF);
    }
    inline void sweepCache(Forest* forest) { searchSweepCache(forest); }
    void reportCacheStat(std::ostream& out, int format=0) const;
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    void searchRemove(BinaryOperation* bop);
    void searchRemove(Forest* forest);
    void searchSweepCache(Forest* forest);
    BinaryOperation* mtfBinary(const BinaryOperationType opT, const Forest* source1F, const Forest* source2F, const Forest* resF);
    BinaryOperation* mtfBinary(const BinaryOperationType opT, const Forest* source1F, const OpndType source2T, const Forest* resF);
};

// ******************************************************************
// *                                                                *
// *                NumericalOperation  class                       *
// *                                                                *
// ******************************************************************

// class BRAVE_DD::NumericalOperation : public Operation {
//     /*-------------------------------------------------------------*/
//     public:
//     /*-------------------------------------------------------------*/
//     NumericalOperation();
//     /*-------------------------------------------------------------*/
//     protected:
//     /*-------------------------------------------------------------*/
//     virtual ~NumericalOperation();
//     // computing tables TBD

//     /*-------------------------------------------------------------*/
//     private:
//     /*-------------------------------------------------------------*/
// };

// ******************************************************************
// *                                                                *
// *                SaturationOperation  class                       *
// *                                                                *
// ******************************************************************

class BRAVE_DD::SaturationOperation : public Operation {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    SaturationOperation(Forest* source1, Forest* source2, Forest* res);

    /* set relations */
    void setRelations(const std::vector<Func>& rels);
    /* set direction */
    void setDirection(const bool dir);
    /* Main part: computation */
    void compute(const Func& source1, Func& res);

    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~SaturationOperation();
    // computing table only for relation product
    ComputeTable        cacheRel;

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// Helper Methods ==============================================
    bool checkForestCompatibility() const;
    Edge computeSaturation(const uint16_t lvl, const Edge& source1, const size_t begin);
    Edge computeSaturationDistance(const uint16_t lvl, const Edge& source1, const size_t begin);
    Edge computeImageSat(const uint16_t lvl, const Edge& source1, const Edge& trans, const size_t begin);
    Edge computeImageSatDistance(const uint16_t lvl, const Edge& source1, const Edge& trans, const size_t begin);
    // sort relation functions
    void sortRelations();
    // locate the first event that its level lower than k, return -1 if not found
    int indexOfTopLessThan(const uint16_t k);
    // list
    friend class SaturationList;
    SaturationOperation*    next;
    // arguments
    Forest*                 source1Forest;
    Forest*                 source2Forest;
    Forest*                 resForest;
    std::vector<Func>       relations;
    bool                    isPre;
};

// ******************************************************************
// *                                                                *
// *                     SaturationList  class                      *
// *                                                                *
// ******************************************************************

class BRAVE_DD::SaturationList {
    std::string name;
    SaturationOperation* front;
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    SaturationList(const std::string n = "");
    inline void reset(const std::string n) {
        front = nullptr;
        name = n;
    }
    inline std::string getName() const {return name;}
    inline bool isEmpty() const {return !front;}
    inline SaturationOperation* add(SaturationOperation* sop) {
        if (sop) {
            sop->next = front;
            front = sop;
        }
        return sop;
    }
    inline void remove(SaturationOperation* sop) {
        if (front == sop) {
            SaturationOperation* toRemove = front;
            front = front->next;
            delete toRemove;
            return;
        }
        searchRemove(sop);
    }
    // find and remove the operation including the given forest
    inline void remove(Forest* forest) { searchRemove(forest); }
    inline SaturationOperation* find(const Forest* source1F, const Forest* source2F, const Forest* resF, const bool dir = 0) {
        if (!front) return nullptr;
        if ((front->source1Forest == source1F) && (front->source2Forest == source2F) && (front->resForest == resF) && (front->isPre == dir)) return front;
        return mtfSaturation(source1F, source2F, resF);
    }
    inline void sweepCache(Forest* forest) { searchSweepCache(forest); }
    void reportCacheStat(std::ostream& out, int format=0) const;

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// Helper Methods ==============================================
    void searchRemove(SaturationOperation* bop);
    void searchRemove(Forest* forest);
    void searchSweepCache(Forest* forest);
    SaturationOperation* mtfSaturation(const Forest* source1F, const Forest* source2F, const Forest* resF);
};

#endif