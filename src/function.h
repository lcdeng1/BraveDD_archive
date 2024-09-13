#ifndef BRAVE_DD_FUNCTION_H
#define BRAVE_DD_FUNCTION_H

#include "defines.h"
#include "setting.h"
#include "edge.h"

namespace BRAVE_DD {
    class Func;
    class FuncArray;

};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                          Func class                            *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::Func {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    Func();
    Func(Forest* f);
    Func(Forest* f, const Edge& e);
    ~Func();

    inline Forest* getForest() const {return parent;};
    inline bool isAttachedTo(const Forest* p) const {return getForest() == p;}
    inline bool isSameForest(const Func &e) const {return parent == e.getForest();}
    
    
    void release();
    inline std::string getLabel() const {return label;}
    inline void setLabel(std::string l) {label = l;}

    /**************************** Make Func *************************/
    // Constant false Func
    void falseFunc();
    // Constant true Func
    void trueFunc();
    // Constant Func
    /* For dimention 1 and 2 */
    void constant(bool val);
    void constant(int val);
    void constant(long val);
    void constant(float val);
    void constant(double val);
    void constant(SpecialValue val);
    /* For dimention of 2 (Relation) */
    void identity(std::vector<bool> dependance);
    void identity(std::list<int> identities);   // levels staying identity
    // Variable Func
    /* For dimention of 1 (Set) */
    void variable(uint16_t lvl);
    void variable(uint16_t lvl, bool low, bool high);
    void variable(uint16_t lvl, int low, int high);
    void variable(uint16_t lvl, long low, long high);
    void variable(uint16_t lvl, float low, float high);
    void variable(uint16_t lvl, double low, double high);
    // ^ wrappers TBD
    /* For dimention of 2 (Relation) */
    void variable(uint16_t lvl, bool isPrime);
    void variable(uint16_t lvl, bool isPrime, bool low, bool high);
    // ^ wrappers TBD

    // Assignment operator
    // Func operator=(Func e);


    /************************* Within Operations ********************/
    unsigned evaluate(Func func, bool* a); // goto Func, return type TBD
    uint64_t countNodes(Func func);



    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// Attach to a forest and link to its registry.
    void attach(Forest* p);
    void init(Func& f);
    inline bool equals(const Func f) const {
        //
        return
            (parent == f.parent) &&
            (edge.handle == f.edge.handle) &&
            (edge.value.getType());
    }
    friend class Forest;
    Forest*     parent;     // parent forest
    Edge        edge;       // edge information
    std::string label;      // Optional, only used for I/O; defaults to empty string
    /* For the Funcs registry in the parent forest */
    Func*       prevFunc;   // Previous Func edge in parent forest
    Func*       nextFunc;   // Next Func edge in parent forest
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                        FuncArray class                         *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::FuncArray {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    FuncArray();
    FuncArray(Forest* f, int size);
    ~FuncArray();

    inline Forest* getForest() const {return parent;};
    inline bool isAttachedTo(const Forest* p) const {return getForest() == p;}
    inline bool isSameForestSet(const FuncArray &e) const {return parent == e.getForest();}
    
    /// Attach to a forest.
    void attach(Forest* p);
    /// Detach from the forest.
    inline void detach() {attach(nullptr);}
    void add(Func f);


    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    friend class Forest;
    Forest*     parent;     // parent forest
    Func*       set;        // set of the Funcs
};

#endif