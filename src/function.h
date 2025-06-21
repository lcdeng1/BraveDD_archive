#ifndef BRAVE_DD_FUNCTION_H
#define BRAVE_DD_FUNCTION_H

#include "defines.h"
#include "setting.h"
#include "edge.h"

namespace BRAVE_DD {
    class Func;
    class ExplictFunc;
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

    /***************************** General **************************/
    inline Forest* getForest() const {return parent;}
    inline Edge getEdge() const {return edge;}
    inline bool isAttachedTo(const Forest* p) const {return getForest() == p;}
    inline bool isSameForest(const Func &e) const {return parent == e.getForest();}

    inline std::string getName() const {return name;}
    inline void setEdge(const Edge& e) {edge = e;}
    inline void setName(std::string l) {name = l;}

    /**************************** Make Func *************************/
    // Constant false Func
    void falseFunc();
    // Constant true Func
    void trueFunc();
    // Constant Func
    /* For dimention 1 and 2 */
    void constant(int val);
    void constant(long val);
    void constant(float val);
    void constant(double val);
    void constant(SpecialValue val);
    /* For dimention of 2 (Relation) */
    void identity(std::vector<bool> dependance);    // level k is identity if dependance[k]==0
    void identity(std::list<int> identities);       // levels staying identity
    // Variable Func
    /* For dimention of 1 (Set) */
    void variable(uint16_t lvl);
    void variable(uint16_t lvl, Value low, Value high);
    /* For dimention of 2 (Relation) */
    void variable(uint16_t lvl, bool isPrime);
    void variable(uint16_t lvl, bool isPrime, Value low, Value high);

    // Convert EV+ to EVMOD
    Edge convert(Forest* evmodForest, Edge evEdge);

    // Assignment operator
    // Func operator=(Func e);

    inline bool operator==(const Func& f) const {
        return equals(f);
    }


    /************************* Within Operations ********************/

    /** Compute and get the encoded function value by giving the assignment.
     *  Note: the first element of "assignment" (assignment[0]) is not used!
     * 
     */
    Value evaluate(const std::vector<bool>& assignment) const;
    Value evaluate(const std::vector<bool>& aFrom, const std::vector<bool>& aTo) const;

    // ExplictFunc including info of assignments, outcomes, 
    void unionAssignments(const ExplictFunc& assignments);

    uint64_t countNodes(Func func);



    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    // ======================Helper Methods====================
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
    Edge unionAssignmentRecursive(uint16_t num, Edge& root, ExplictFunc assignments);


    // ========================================================
    friend class Forest;
    Forest*     parent;     // parent forest
    Edge        edge;       // edge information
    std::string name;       // Optional, only used for I/O; defaults to empty string
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      ExplictFunc class                         *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::ExplictFunc {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    ExplictFunc();
    ~ExplictFunc();

    // Add an assignment with its outcome value
    void addAssignment(const std::vector<bool>& minterm, const Value& outcome);
    
    // Getters for assignments and outcomes
    const std::vector<std::vector<bool>>& getAssignments() const;
    const std::vector<Value>& getOutcomes() const;
    
    // Default value handling
    Value getDefaultValue() const;
    void setDefaultValue(const Value& val);
    
    // Get number of assignments
    size_t size() const;
    
    // Get number of bits in assignments
    int getNumBits() const;
    
    // Get specific assignment and outcome
    const std::vector<bool>& getAssignment(int idx) const;
    const Value& getOutcome(int idx) const;
    
    // Convert assignments to char arrays for radix scan
    char** getAllAssignmentsAsCharArray() const;
    
    // Free memory from char array conversion
    void freeCharArray(char** array, int size) const;

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    std::vector<std::vector<bool>>     assignments;
    std::vector<Value>                 outcomes;
    Value                              defaultVal;
};

#endif