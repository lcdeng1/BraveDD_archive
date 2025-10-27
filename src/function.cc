#include "function.h"
#include "forest.h"
#include "node.h"
#include "operations/operation.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                         Func  methods                          *
// *                                                                *
// *                                                                *
// ******************************************************************
Func::Func()
{
    //
}
Func::Func(Forest* f)
{
    parent = f;
    name = "";
    // prevFunc = 0;
    // nextFunc = 0;
}
Func::Func(Forest* f, const Edge& e)
:edge(e)
{
    parent = f;
    name = "";
    // prevFunc = 0;
    // nextFunc = 0;
}
Func::~Func()
{
    //
}

/***************************** General **************************/

/**************************** Make edge *************************/
void Func::trueFunc()
{
    if (parent->setting.getEncodeMechanism() == TERMINAL) {
        /* Don't care the value on edge */
        edge.handle = makeTerminal(INT, 1);
        if (parent->setting.getValType() == FLOAT) {
            edge.handle = makeTerminal(FLOAT, 1.0f);
        }
        packRule(edge.handle, RULE_X);
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else {
        edge.handle = makeTerminal(VOID, SpecialValue::OMEGA);
        packRule(edge.handle, RULE_X);
        edge.value = Value(1);  // value type?
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    }
}
void Func::falseFunc()
{
    if (parent->setting.getEncodeMechanism() == TERMINAL) {
    edge.handle = makeTerminal(INT, 0);
        if (parent->setting.getValType() == FLOAT) {
            edge.handle = makeTerminal(FLOAT, 0.0f);
        }
        packRule(edge.handle, RULE_X);
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else {
        edge.handle = makeTerminal(VOID, SpecialValue::OMEGA);
        packRule(edge.handle, RULE_X);
        edge.value = Value(0);  // value type?
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    }
}
/* For dimention 1 and 2 */
void Func::constant(int val)
{
    if (parent->setting.getEncodeMechanism() == TERMINAL) {
        edge.handle = makeTerminal(INT, val);
        packRule(edge.handle, RULE_X);
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else if (parent->setting.getEncodeMechanism() == EDGE_PLUS || parent->setting.getEncodeMechanism() == EDGE_PLUSMOD) {
        edge.handle = makeTerminal(VOID, SpecialValue::OMEGA);
        packRule(edge.handle, RULE_X);
        edge.setValue(Value(val));
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else {
        // TBD
    }
}
void Func::constant(long val)
{
    if (parent->setting.getEncodeMechanism() == TERMINAL) {
        edge.handle = makeTerminal(LONG, val);
        packRule(edge.handle, RULE_X);
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else if (parent->setting.getEncodeMechanism() == EDGE_PLUS || parent->setting.getEncodeMechanism() == EDGE_PLUSMOD) {
        edge.handle = makeTerminal(VOID, SpecialValue::OMEGA);
        packRule(edge.handle, RULE_X);
        edge.setValue(Value(val));
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else {
        // TBD
    }
}
void Func::constant(float val)
{
    if (parent->setting.getEncodeMechanism() == TERMINAL) {
        edge.handle = makeTerminal(FLOAT, val);
        packRule(edge.handle, RULE_X);
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else if (parent->setting.getEncodeMechanism() == EDGE_PLUS || parent->setting.getEncodeMechanism() == EDGE_PLUSMOD) {
        edge.handle = makeTerminal(VOID, SpecialValue::OMEGA);
        packRule(edge.handle, RULE_X);
        edge.setValue(Value(val));
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else {
        // TBD
    }
}
void Func::constant(double val)
{
    if (parent->setting.getEncodeMechanism() == TERMINAL) {
        edge.handle = makeTerminal(FLOAT, val);
        packRule(edge.handle, RULE_X);
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else if (parent->setting.getEncodeMechanism() == EDGE_PLUS || parent->setting.getEncodeMechanism() == EDGE_PLUSMOD) {
        edge.handle = makeTerminal(VOID, SpecialValue::OMEGA);
        packRule(edge.handle, RULE_X);
        edge.setValue(Value(val));
        edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
    } else {
        // TBD
    }
}
void Func::constant(SpecialValue val)
{
    edge.handle = makeTerminal(VOID, val);
    packRule(edge.handle, RULE_X);
    edge = parent->normalizeEdge(parent->setting.getNumVars(), edge);
}
/* For dimention of 2 (Relation) */
void Func::identity(std::vector<bool> dependance)
{
    Level N = parent->getSetting().getNumVars();
    Level top = 0;
    std::vector<Edge> child(4);
    Edge reduced;
    reduced.handle = makeTerminal(1);
    if ((parent->getSetting().getValType() == FLOAT) || (parent->getSetting().getValType() == DOUBLE)) {
        reduced.handle = makeTerminal(1.0f);
    }
    reduced.setRule(RULE_I0);
    EdgeLabel root  = 0;
    packRule(root, RULE_I0);
    for (Level k=1; k<=N; k++) {
        if (dependance[k] == 1) {
            reduced = parent->mergeEdge(k-1, top, root, reduced);
            for (size_t i=0; i<child.size(); i++) {
                child[i] = reduced;
            }
            reduced = parent->reduceEdge(k, root, k, child);
            top = k;
        } else {
            continue;
        }
    }
    edge = parent->mergeEdge(N, top, root, reduced);
}
/* For dimention of 1 (Set) */
void Func::variable(Level lvl)
{
    std::vector<Edge> child(2);
    if (parent->setting.getEncodeMechanism() == TERMINAL) {
        child[0].handle = makeTerminal(INT, 0);
        child[1].handle = makeTerminal(INT, 1);
        if ((parent->getSetting().getValType() == FLOAT) || (parent->getSetting().getValType() == DOUBLE)) {
            child[0].handle = makeTerminal(FLOAT, 0.0f);
            child[1].handle = makeTerminal(FLOAT, 1.0f);
        }
        for (size_t i=0; i<child.size(); i++) {
            packRule(child[i].handle, RULE_X);
        }
    } else {
        for (size_t i=0; i<child.size(); i++) {
            child[i].handle = makeTerminal(VOID, SpecialValue::OMEGA);
            packRule(child[i].handle, RULE_X);
        }
        child[1].value = Value(1);
    }
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    edge = parent->reduceEdge(parent->getSetting().getNumVars(), root, lvl, child);
}
void Func::variable(Level lvl, Value low, Value high)
{
    std::vector<Edge> child(2);
    if (parent->setting.getEncodeMechanism() == TERMINAL) {
        child[0].handle = makeTerminal(low);
        child[1].handle = makeTerminal(high);
        for (size_t i=0; i<child.size(); i++) {
            packRule(child[i].handle, RULE_X);
        }
    } else {
        if (low.getType() == VOID) {
            child[0].handle = makeTerminal(low);
        } else {
            child[0].handle = makeTerminal(Value(SpecialValue::OMEGA));
            child[0].value = low;
        }
        if (high.getType() == VOID) {
            child[1].handle = makeTerminal(high);
        } else {
            child[1].handle = makeTerminal(Value(SpecialValue::OMEGA));
            child[1].value = high;
        }
        for (size_t i=0; i<child.size(); i++) {
            packRule(child[i].handle, RULE_X);
        }
    }
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    edge = parent->reduceEdge(parent->getSetting().getNumVars(), root, lvl, child);
}
/* For dimention of 2 (Relation) */
// Variable Func
void Func::variable(Level lvl, bool isPrime)
{
    std::vector<Edge> child(4);
    child[0].handle = makeTerminal(INT, 0);
    child[1].handle = makeTerminal(INT, isPrime?1:0);
    child[2].handle = makeTerminal(INT, isPrime?0:1);
    child[3].handle = makeTerminal(INT, 1);
    if ((parent->getSetting().getValType() == FLOAT) || (parent->getSetting().getValType() == DOUBLE)) {
        child[0].handle = makeTerminal(FLOAT, 0.0f);
        child[1].handle = makeTerminal(INT, isPrime?1.0f:0.0f);
        child[2].handle = makeTerminal(INT, isPrime?0.0f:1.0f);
        child[3].handle = makeTerminal(FLOAT, 1.0f);
    }
    for (size_t i=0; i<child.size(); i++) {
        packRule(child[i].handle, RULE_X);
    }
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    edge = parent->reduceEdge(parent->getSetting().getNumVars(), root, lvl, child);
}
void Func::variable(Level lvl, bool isPrime, Value low, Value high)
{
    // TBD
}

// TODO: add more test and error cases around them 
Edge Func::convert(Forest* evmodForest, Edge evEdge) {
    EdgeLabel label = 0;
    packRule(label, RULE_X);
    std::vector<Edge>evmodChild(2);
    Level lvl = evEdge.getNodeLevel();
    if (evEdge.getNodeLevel() == 0) {
        Edge evmodEdge;
        evmodEdge.setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA));
        evmodEdge.setValue(evEdge.getValue());
        return evmodEdge;
    }
    if (lvl == 1) {
        Edge evChild0 = parent->getChildEdge(evEdge.getNodeLevel(),evEdge.getNodeHandle(), 0);
        Edge evChild1 = parent->getChildEdge(evEdge.getNodeLevel(),evEdge.getNodeHandle(), 1);
        evmodChild[0].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA));
        evmodChild[1].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA));
        packRule(edge.handle,RULE_X);
        evmodChild[0].setValue(evChild0.getValue());
        evmodChild[1].setValue(evChild1.getValue());
        return evmodForest->reduceEdge(evEdge.getNodeLevel(), label, evEdge.getNodeLevel(), evmodChild);
    }
    Edge evChild0 = parent->getChildEdge(evEdge.getNodeLevel(),evEdge.getNodeHandle(), 0);
    Edge evChild1 = parent->getChildEdge(evEdge.getNodeLevel(),evEdge.getNodeHandle(), 1);
    evmodChild[0] = convert(evmodForest, evChild0);
    evmodChild[0].setValue(evChild0.getValue());
    evmodChild[1] = convert(evmodForest, evChild0);
    evmodChild[1].setValue(evChild0.getValue());
    return evmodForest->reduceEdge(evEdge.getNodeLevel(), label, evEdge.getNodeLevel(), evmodChild);
}

Value Func::evaluate(const std::vector<bool>& assignment) const
{
    /* check the level */
    if (parent->getSetting().getNumVars() != (assignment.size()-1)) {
        std::cout << "[BRAVE_DD] ERROR!\t Func::evaluate(): Variable number check failed in evaluation! It was "<<assignment.size()-1
        <<", it should be "<<parent->getSetting().getNumVars() << std::endl;
        exit(0);
    }
    /* final answer */
    Value ans(0);
    /* encode mechanism */
    EncodeMechanism encode = parent->getSetting().getEncodeMechanism();
    /* value type*/
    ValueType vt = parent->getSetting().getValType();
    /* edge flags type */
    SwapSet st = parent->getSetting().getSwapType();
    CompSet ct = parent->getSetting().getCompType();

    /* tmp store edge info for "for" loop */
    Edge current = edge;
    /* get target node info */
    NodeHandle targetHandle = current.getNodeHandle();
    Level targetLvl = current.getNodeLevel();
    /* info to determine next child edge */
    bool isComp = 0, isSwap = 0;
    /* flags for reduction rules */
    bool allOne = 1, existOne = 0;
    /* evaluation starting from the target node level */
    Level k = assignment.size()-1;

    /* initialized edge value for EVBDD*/
     if (encode == EDGE_PLUS || encode == EDGE_PLUSMOD) {
        // TODO: Ask Lichuan do we need long and double?
        Value cv = current.getValue();
        if (targetLvl == 0) return cv;
        ans = Value(cv);
    }
    while (true) {
#ifdef BRAVE_DD_TRACE
        std::cout<<"evaluate k: " << k;
        std::cout<<"; currt: ";
        current.print(std::cout, 0);
        std::cout << std::endl;
#endif
        /* check the incoming edge's reduction rule for terminal cases */
        ReductionRule incoming = current.getRule();
        /* if incoming edge skips levels */
        if ((targetLvl < k) && (incoming != RULE_X)) {
            // determine flags of all-ones and exist-ones
            for (Level i=k; i>targetLvl; i--) {
                allOne &= assignment[i];
                existOne |= assignment[i];
            }
            if (encode == TERMINAL) {
                // terminal value, don't care the Value on edge
                if ((allOne && isRuleAH(incoming))
                    || ((!allOne) && isRuleEL(incoming))
                    || (existOne && isRuleEH(incoming))
                    || ((!existOne) && isRuleAL(incoming))) {
                    if (vt == INT || vt == LONG) ans.setValue(hasRuleTerminalOne(incoming)?1:0, INT);
                    else ans.setValue(hasRuleTerminalOne(incoming)?1.0f:0.0f, FLOAT);
                    return ans;
                }
            } else if (encode == EDGE_PLUS || encode == EDGE_PLUSMOD) {
                // edge values plus
                if ((targetLvl == 0) 
                    && isTerminalSpecial(current.getEdgeHandle()) 
                    && !isTerminalSpecial(SpecialValue::OMEGA, current.getEdgeHandle())) {
                    return getTerminalValue(current.getEdgeHandle());
                }
                std::cout << "[BRAVE_DD] ERROR!\t evaluate(): Illegal patterns for EVBDD!" << std::endl;
                exit(0);
            } else if (encode == EDGE_MULT) {
                // edge values multiply
                // TBD
            }
        }
        if (targetLvl > 0) {
            /* short incoming edge, or long incoming edge but skips */
            k = targetLvl-1;
            isSwap = (st==ONE || st==ALL) ? current.getSwap(0) : 0;
            // get swap/comp bit only when it's allowed, since user may insert illegal nodes into nodemanager (which is allowed)
            isComp = (ct==COMP) ? current.getComp() : 0;
            current = parent->getChildEdge(targetLvl, targetHandle, isSwap^assignment[targetLvl]);
            if (isComp) current.complement();
            if (isSwap && st==ALL) current.swap();  // for swap-all
            /* update varibles */
            allOne = 1;
            existOne = 0;
            targetHandle = current.getNodeHandle();
            targetLvl = current.getNodeLevel();

            /* cumulate the edge values*/
            if (encode == EDGE_PLUS || encode == EDGE_PLUSMOD) {
                Value cv = current.getValue();
                if (vt == INT) ans = Value(ans.getIntValue() + cv.getIntValue());
                else if (vt == LONG) ans = Value(ans.getLongValue() + cv.getLongValue());
            }
#ifdef BRAVE_DD_TRACE
            std::cout<<"next currt: k="<< k <<", targetlvl=" << targetLvl << "; ";
            current.print(std::cout, 0);
            std::cout << std::endl;
#endif
            continue;
        }
        // not reach the terminal cases of reduction rules, or got the next edge
        if (targetLvl == 0) {
            // value type INT, FLOAT, or VOID (special value)
            if (encode == TERMINAL) ans = getTerminalValue(current.handle);
            if (current.getComp() && ct != NO_COMP) {
                if (ans.valueType == INT) {
                    int terminalVal = *reinterpret_cast<int*>(&targetHandle);
                    terminalVal = parent->getSetting().getMaxRange() - terminalVal; // complement if needed
                    ans.setValue(terminalVal, INT);
                } else if (ans.valueType == FLOAT) {
                    float terminalVal = *reinterpret_cast<float*>(&targetHandle);
                    terminalVal = parent->getSetting().getMaxRange() - terminalVal; // complement if needed
                    ans.setValue(terminalVal, FLOAT);
                } else if (ans.valueType == VOID) {
                    // special value: NegInf => PosInf?
                    // TBD
                }
            }
            if(encode == EDGE_PLUS) {
                // only special terminal check here
                if( isTerminalSpecial(current.getEdgeHandle()) 
                    && !isTerminalSpecial(SpecialValue::OMEGA, current.getEdgeHandle())) {
                        return getTerminalValue(current.getEdgeHandle());
                    }
            } else if (encode == EDGE_PLUSMOD) {
                // TODO: after paper discuss
                // Since the sum of the edge values are either int or long, 
                // maxRange being unsigned long seem a too big
                if( isTerminalSpecial(current.getEdgeHandle()) 
                    && !isTerminalSpecial(SpecialValue::OMEGA, current.getEdgeHandle())) {
                        return getTerminalValue(current.getEdgeHandle());
                    }
                unsigned long mod = getForest()->getSetting().getMaxRange();
                if (vt == INT) {
                    // mu is greater than the value that int can store
                    if ( mod > static_cast<unsigned long>(std::numeric_limits<int>::max())) {
                        std::cout << "[BRAVE_DD] ERROR!\t maxRange overflows valType specified" << std::endl;
                        exit(0);
                    }
                    return Value(ans.getIntValue() % static_cast<int>(mod));
                } else if (vt == LONG) {
                    // mu is greater than the value that long can store
                    if ( mod > static_cast<unsigned long>(std::numeric_limits<long>::max())) {
                        std::cout << "[BRAVE_DD] ERROR!\t maxRange overflows valType specified" << std::endl;
                        exit(0);
                    }
                    return Value(ans.getLongValue() % static_cast<long>(mod));
                } 
            }
            return ans;
        }
        k--;
    }
    return ans;
}

Value Func::evaluate(const std::vector<bool>& aFrom, const std::vector<bool>& aTo) const
{
    /* check the level */
    if ((parent->getSetting().getNumVars() != (aFrom.size()-1)) || (aFrom.size() != aTo.size())) {
        std::cout << "[BRAVE_DD] ERROR!\t Func::evaluate(): Variable number check failed in evaluation! It was "<<aFrom.size()-1
        <<", it should be "<<parent->getSetting().getNumVars() << std::endl;
        exit(0);
    }
    /* final answer */
    Value ans(0);
    /* encode mechanism */
    EncodeMechanism encode = parent->getSetting().getEncodeMechanism();
    /* edge flags type */
    SwapSet st = parent->getSetting().getSwapType();
    CompSet ct = parent->getSetting().getCompType();
    /* tmp store edge info for "for" loop */
    Edge current = edge;
    /* get target node info */
    NodeHandle targetHandle = current.getNodeHandle();
    Level targetLvl = current.getNodeLevel();
    /* info to determine next child edge */
    bool isComp = 0, isSwapF = 0, isSwapT = 0;
    bool isIdent = 1;
    /* evaluation starting from the target node level */
    Level k = aFrom.size()-1;
    while (true) {
#ifdef BRAVE_DD_TRACE
        std::cout<<"evaluate k: " << k;
        std::cout<<"; currt: ";
        current.print(std::cout, 0);
        std::cout << std::endl;
#endif
        /* check the incoming edge's reduction rule for terminal cases */
        ReductionRule incoming = current.getRule();
        /* if incoming edge skips levels */
        if ((targetLvl < k) && (incoming != RULE_X)) {
            // determine identity
            for (Level i=k; i>targetLvl; i--) {
                if (aFrom[i] != aTo[i]) isIdent = 0;
            }
            if (encode == TERMINAL) {
                // terminal value, don't care the Value on edge
                ValueType vt = (parent->getSetting().getValType() == INT
                                || parent->getSetting().getValType() == LONG) ? INT : FLOAT;
                if ((!isIdent) && isRuleI(incoming)) {
                    if (vt == INT) ans.setValue(hasRuleTerminalOne(incoming)?1:0, INT);
                    else ans.setValue(hasRuleTerminalOne(incoming)?1.0f:0.0f, FLOAT);
                    return ans;
                }
            } else if (encode == EDGE_PLUS) {
                // edge values plus
                // TBD
            } else if (encode == EDGE_PLUSMOD) {
                // edge values plus and modulo
                // TBD
            } else if (encode == EDGE_MULT) {
                // edge values multiply
                // TBD
            }
        }
        if (targetLvl > 0) {
            /* short incoming edge, or long incoming edge but skips */
            k = targetLvl-1;
            isSwapF = (st==FROM || st==FROM_TO) ? current.getSwap(0) : 0;
            isSwapT = (st==TO || st==FROM_TO) ? current.getSwap(1) : 0;

            // get swap/comp bit only when it's allowed, since user may insert illegal nodes into nodemanager (which is allowed)
            isComp = (ct==COMP) ? current.getComp() : 0;
            current = parent->getChildEdge(targetLvl, targetHandle, (char)(((isSwapF^aFrom[targetLvl])<<1) + (isSwapT^aTo[targetLvl])));
            if (isComp) current.complement();
            /* update varibles */
            isIdent = 1;
            targetHandle = current.getNodeHandle();
            targetLvl = current.getNodeLevel();
#ifdef BRAVE_DD_TRACE
            std::cout<<"next currt: k="<< k <<", targetlvl=" << targetLvl << "; ";
            current.print(std::cout, 0);
            std::cout << std::endl;
#endif
            continue;
        }
        // not reach the terminal cases of reduction rules, or got the next edge
        if (targetLvl == 0) {
            // value type INT, FLOAT, or VOID (special value)
            ans = getTerminalValue(current.handle);
            if (current.getComp() && ct != NO_COMP) {
                if (ans.valueType == INT) {
                    int terminalVal = *reinterpret_cast<int*>(&targetHandle);
                    terminalVal = parent->getSetting().getMaxRange() - terminalVal; // complement if needed
                    ans.setValue(terminalVal, INT);
                } else if (ans.valueType == FLOAT) {
                    float terminalVal = *reinterpret_cast<float*>(&targetHandle);
                    terminalVal = parent->getSetting().getMaxRange() - terminalVal; // complement if needed
                    ans.setValue(terminalVal, FLOAT);
                } else if (ans.valueType == VOID) {
                    // special value: NegInf => PosInf?
                    // TBD
                }
            }
            return ans;
        }
        k--;
    }
    return ans;
}

void Func::unionAssignments(const ExplictFunc& assignments) {
    try {
        // Check if function is attached to a forest
        if (!parent) {
            std::cerr << "[BRAVE_DD] ERROR! unionAssignments: Function not attached to a forest" << std::endl;
            return;
        }

        // Get the number of assignments
        size_t numAssignments = assignments.size();
        if (numAssignments == 0) {
            // Nothing to do, return the original function
            return;
        }

        // Get the number of bits to process (skip the 0th element)
        int numBits = assignments.getNumBits();
        if (numBits <= 0) {
            throw error(ErrCode::MISCELLANEOUS, __FILE__, __LINE__);
        }
        
        // Create a full new BDD instead of trying to modify the existing one
        Func resultFunc(parent);
        resultFunc.falseFunc();  // Start with a false function
        
        // Process each assignment individually
        for (size_t i = 0; i < numAssignments; i++) {
            const std::vector<bool>& assignment = assignments.getAssignment(i);
            // const Value& outcome = assignments.getOutcome(i);
            
            // Create a path for this assignment
            // For simplicity, build each path level by level from the bottom up
            Level numVars = parent->getSetting().getNumVars();
            
            // Start with a terminal 1 node
            Edge terminalOneEdge, terminalZeroEdge;
            EdgeHandle constOne = makeTerminal(INT, 1), constZero = makeTerminal(INT, 0);
            packRule(constOne, RULE_X);
            packRule(constZero, RULE_X);
            Value v(0);
            terminalOneEdge.setEdgeHandle(constOne);
            terminalZeroEdge.setEdgeHandle(constZero);
            
            // Build the path from the bottom up
            Edge currentEdge = terminalOneEdge;
            
            for (Level level = 1; level <= numVars; level++) {
                Level varIndex = numVars - level + 1;  // Variable index (1-based)
                
                // Create a node at this level
                std::vector<Edge> children(2);
                
                // Connect the path based on the assignment
                if (assignment[varIndex]) {
                    // Variable is 1 in the assignment
                    children[0] = terminalZeroEdge;  // 0-child
                    children[1] = currentEdge;       // 1-child
                } else {
                    // Variable is 0 in the assignment
                    children[0] = currentEdge;       // 0-child
                    children[1] = terminalZeroEdge;  // 1-child
                }
                
                // Create the node
                EdgeLabel label = 0;
                packRule(label, RULE_X);
                currentEdge = parent->reduceEdge(numVars, label, varIndex, children);
            }
            
            // Now we have a BDD path for this assignment
            // Add it to the result function using union operation
            try {
                BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_UNION, parent, parent, parent);
                if (!bop) {
                    bop = BOPs.add(new BinaryOperation(BinaryOperationType::BOP_UNION, parent, parent, parent));
                }
                
                Func pathFunc(parent, currentEdge);
                
                // Union the current result with this path
                if (i == 0) {
                    resultFunc = pathFunc;  // First assignment - just use the path
                } else {
                    Func temp(parent);
                    bop->compute(resultFunc, pathFunc, temp);
                    resultFunc = temp;
                }
            } catch (const std::exception& e) {
                std::cerr << "[BRAVE_DD] ERROR in unionAssignments union operation: " << e.what() << std::endl;
            }
        }
        
        // Set the final result
        edge = resultFunc.getEdge();
        
        // Ensure the result has proper type flags
        // edge = ensureTerminalTypeFlags(parent, edge);
    }
    catch (const std::exception& e) {
        std::cerr << "[BRAVE_DD] ERROR in unionAssignments: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "[BRAVE_DD] UNKNOWN ERROR in unionAssignments" << std::endl;
    }
}

/* Expert function for union assignments TBD */
Edge Func::unionAssignmentRecursive(Level n, Edge& root, ExplictFunc assignments)
{
    /* declare the final answer */
    Edge ans;
    /* terminal case */
    if (n==0) {
        // return the edge pointing to terminal node
    }
    /*
     * here means the root edge can not capture the assignment,
     * then we do it recursively!
     *      we need to expand root edge if it's a long edge
     */
    Level skipLvl;
    skipLvl = n - unpackLevel(root.handle);
    /* determine the down edges */
    //
    if (skipLvl == 0) {
        // it is a short edge, they are the child edges of root target node
    } else {
        // it is a long edge, expand based on its reduction rule if needed <==== helper function?
    }
    /*
     * build new node at level n, child edges are down edges or recursive calls based on assignment[n]
     */
    // temporary node information 
    /* reduce */
    // parent->reduceEdge
    return ans;
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      ExplictFunc  methods                      *
// *                                                                *
// *                                                                *
// ******************************************************************
ExplictFunc::ExplictFunc()
{
    defaultVal.setValue(0, INT);
}

ExplictFunc::~ExplictFunc()
{
    // Nothing to clean up
}

void ExplictFunc::addAssignment(const std::vector<bool>& assignment, const Value& outcome)
{
    assignments.push_back(assignment);
    outcomes.push_back(outcome);
}

const std::vector<std::vector<bool>>& ExplictFunc::getAssignments() const
{
    return assignments;
}

const std::vector<Value>& ExplictFunc::getOutcomes() const
{
    return outcomes;
}

Value ExplictFunc::getDefaultValue() const
{
    return defaultVal;
}

void ExplictFunc::setDefaultValue(const Value& val)
{
    defaultVal = val;
}

size_t ExplictFunc::size() const
{
    return assignments.size();
}

int ExplictFunc::getNumBits() const
{
    if (assignments.empty()) {
        return 0;
    }
    return static_cast<int>(assignments[0].size());
}

const std::vector<bool>& ExplictFunc::getAssignment(int idx) const
{
    if (idx < 0 || idx >= static_cast<int>(assignments.size())) {
        static std::vector<bool> empty;
        std::cerr << "[BRAVE_DD] ERROR! ExplictFunc::getAssignment: Index out of bounds" << std::endl;
        return empty;
    }
    return assignments[idx];
}

const Value& ExplictFunc::getOutcome(int idx) const
{
    if (idx < 0 || idx >= static_cast<int>(outcomes.size())) {
        static Value empty;
        std::cerr << "[BRAVE_DD] ERROR! ExplictFunc::getOutcome: Index out of bounds" << std::endl;
        return empty;
    }
    return outcomes[idx];
}

char** ExplictFunc::getAllAssignmentsAsCharArray() const
{
    size_t numAssignments = assignments.size();
    if (numAssignments == 0) {
        return nullptr;
    }

    // Get the number of bits
    size_t numBits = assignments[0].size() - 1; // Skip the 0th element
    
    // Allocate memory for the char** array
    char** result = new char*[numAssignments];
    
    // Convert each assignment to a char array
    for (size_t i = 0; i < numAssignments; i++) {
        result[i] = new char[numBits + 1]; // +1 for null terminator
        
        // Skip the 0th element in assignments
        for (size_t j = 0; j < numBits; j++) {
            result[i][j] = assignments[i][j+1] ? '1' : '0';
        }
        result[i][numBits] = '\0'; // Null terminate
    }
    
    return result;
}

Func ExplictFunc::buildFunc(Forest* forest) const
{
    // number of variables does not match, assuming we are set-encoding, rel-encoding TBD
    if (forest->getSetting().getNumVars() != getNumBits()) {
        std::cout << "[BRAVE_DD] ERROR!\t ExplictFunc::buildFunc(): Number of variable does not match.\n";
        exit(1);    // TBD, maybe throw error msg
    }
    // the final answer
    Func ans(forest);
    // copy the ExplictFunc
    ExplictFunc EF = *this;
    Edge edge = EF.buildEdge(forest, getNumBits(), 0, size());
    // passing to final answer
    ans.setEdge(edge);
    return ans;
}

Edge ExplictFunc::buildEdge(Forest* forest, Level lvl, size_t start, size_t size)
{
    // the edge to return
    Edge ans;
    // empty size or terminal by level, build constant edge to default value or outcome value
    if ((size == 0) || (lvl == 0)) {
        EncodeMechanism em = forest->getSetting().getEncodeMechanism();
        Value val = (size == 0) ? defaultVal : outcomes[start];
        ValueType valTP = val.getType();
        if (em == TERMINAL) {
                ans.handle = makeTerminal(val);
        } else {
            ans.handle = makeTerminal(VOID, SpecialValue::OMEGA);
            if (valTP != VOID) {
                ans.setValue(val);
            } else {
                ans.handle = makeTerminal(val);
            }
        }
        packRule(ans.handle, RULE_X);
        // return (size == 0) ? forest->normalizeEdge(lvl, ans) : ans;
        return ans;
    }
    // two-finger algorithm to sort 0,1 values in position lvl
    size_t left = start;
    size_t right = start + size - 1;
    for (;;) {
        // move left to first 1 value
        for ( ; left < right; left++) {
            if (assignments[left][lvl-1] == 1) break;
        }
        // move right to first 0 value
        for ( ; left < right; right--) {
            if (assignments[right][lvl-1] == 0) break;
        }
        // stop?
        if (left >= right) break;
        // we have a 1 before a 0, swap them;
        SWAP(assignments[left], assignments[right]);
        SWAP(outcomes[left], outcomes[right]);
        // for sure we can move them one spot
        ++left;
        --right;
    }
    if ((left < (size + start)) && (assignments[left][lvl-1] == 0)) left++;

    // build a ndoe and recursively call
    std::vector<Edge> child(2);
    child[0] = buildEdge(forest, lvl-1, start, left-start);
    child[1] = buildEdge(forest, lvl-1, left, size-left+start);
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    return forest->reduceEdge(lvl, root, lvl, child);
}

void ExplictFunc::freeCharArray(char** array, int size) const
{
    if (array == nullptr) {
        return;
    }
    
    for (int i = 0; i < size; i++) {
        delete[] array[i];
    }
    delete[] array;
} 