#ifndef BRAVE_DD_FUNCTION_H
#define BRAVE_DD_FUNCTION_H

#include "defines.h"
#include "setting.h"
#include "edge.h"

namespace BRAVE_DD {
    class Func;
    class FuncSet;

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
    
    /// Attach to a forest.
    void attach(Forest* p);
    /// Detach from the forest.
    inline void detach() { attach(nullptr); }
    inline std::string getLabel() const {return label;}
    inline void setLabel(std::string l) {label = l;}

    Func constant(SpecialValue val);


    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
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
// *                        FuncSet class                           *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::FuncSet {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    FuncSet();
    FuncSet(Forest* f);
    ~FuncSet();

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    Forest*     parent;
    Func*       set;
};

#endif