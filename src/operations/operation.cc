#include "operation.h"
#include "../IO/out_dot.h"

// #define BRAVE_DD_OPERATION_TRACE

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                      Operation  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************

Operation::Operation()
{
    //
}
Operation::~Operation()
{
    //
}

// ******************************************************************
// *                                                                *
// *                    UnaryOperation  class                       *
// *                                                                *
// ******************************************************************
UnaryOperation::UnaryOperation(UnaryOperationType type, Forest* source, Forest* target)
:opType(type)
{
    sourceForest = source;
    targetForest = target;
    targetType = OpndType::FOREST;
}
UnaryOperation::UnaryOperation(UnaryOperationType type, Forest* source, OpndType target)
:opType(type)
{
    sourceForest = source;
    targetForest = source;
    targetType = target;
}
UnaryOperation::~UnaryOperation()
{
    cache.~ComputeTable();
    sourceForest = nullptr;
    targetForest = nullptr;
}

void UnaryOperation::compute(const Func& source, Func& target)
{
    if (!checkForestCompatibility()) {
        throw error(ErrCode::INVALID_OPERATION, __FILE__, __LINE__);
    }
    uint16_t numVars = sourceForest->getSetting().getNumVars();
    Edge ans;
    // copy to target forest
    ans = computeCOPY(numVars, source.getEdge());
    if (opType == UnaryOperationType::UOP_COMPLEMENT) {
        // target forest allows complement flag
        if ((targetForest->getSetting().getCompType() != NO_COMP)
            && (targetForest->getSetting().getEncodeMechanism() == TERMINAL)) {
            ans.complement();
            // normalize edge
            if (!targetForest->getSetting().hasReductionRule(ans.getRule())) {
                ans = targetForest->normalizeEdge(numVars, ans);
            }
            target.setEdge(ans);
            return;
        }
        // here is the forest that does not allow complement bit, recursively compute
        ans = computeCOMPLEMENT(numVars, ans);
    } else {
        // TBD
    }
    target.setEdge(ans);
    // other info TBD
}

void UnaryOperation::compute(const Func& source, long& target)
{
    uint16_t numVars = sourceForest->getSetting().getNumVars();
    if (opType == UnaryOperationType::UOP_CARDINALITY) {
        target = computeCARD(numVars, source.getEdge());
    }
    // TBD
}

void UnaryOperation::compute(const Func& source, double& target)
{
    if (!checkForestCompatibility()) {
        throw error(ErrCode::INVALID_OPERATION, __FILE__, __LINE__);
    }
    // TBD
}

bool UnaryOperation::checkForestCompatibility() const
{
    bool ans = 1;
    // TBD
    return ans;
}

Edge UnaryOperation::computeCOPY(const uint16_t lvl, const Edge& source)
{
    // Direct return if the target forest is the source forest
    if (sourceForest == targetForest) {
        return source;
    }
    // Terminal case

    // check compute table

    Edge ans;
    //TBD
    return ans;
}

Edge UnaryOperation::computeCOMPLEMENT(const uint16_t lvl, const Edge& source)
{
    /* Assuming this is within the same target forest
     Here is forest that does not allow complement bit */
    
    Edge ans;    
    // terminal case
    if (source.getNodeLevel() == 0) {
        ans = source;
        ans.complement();
        return targetForest->normalizeEdge(lvl, ans);
    } else {
        // check cache
        if (cache.check(lvl, source, ans)) return ans;
        std::vector<Edge> childEdges;
        if (targetForest->getSetting().isRelation()) {
            childEdges = std::vector<Edge>(4);
        } else {
            childEdges = std::vector<Edge>(2);
        }
        for (char i=0; (size_t)i<childEdges.size(); i++) {
            childEdges[i] = targetForest->getChildEdge(source.getNodeLevel(), source.getNodeHandle(), i);
            childEdges[i] = computeCOMPLEMENT(source.getNodeLevel()-1, childEdges[i]);
        }
        EdgeLabel label = 0;
        packRule(label, compRule(source.getRule()));
        ans = targetForest->reduceEdge(lvl, label, source.getNodeLevel(), childEdges);
    }
    // cache
    cache.add(lvl, source, ans);
    return ans;
}

long UnaryOperation::computeCARD(const uint16_t lvl, const Edge& source)
{
    // base cases:
    if (lvl == 0) {
        if (targetForest->getSetting().getEncodeMechanism() == TERMINAL) {
            return source.getComp() ^ isTerminalOne(source.getEdgeHandle());
            // other value TBD
        } // other encodings TBD
    }
    if (source.getNodeLevel() == 0) {
        if (targetForest->getSetting().getEncodeMechanism() == TERMINAL) {
            if (source.isConstantZero()) {
                return 0;
            } else if (source.isConstantOne()) {
                return (targetForest->getSetting().isRelation()) ? (0x01 << (2*lvl)) : (0x01 << lvl);
            } else if ((source.getRule() == RULE_EL1) || (source.getRule() == RULE_EH1)
                        || (source.getRule() == RULE_AH0) || (source.getRule() == RULE_AL0)) {
                return (0x01 << lvl) - 1;
            } else if ((source.getRule() == RULE_EL0) || (source.getRule() == RULE_EH0)
                        || (source.getRule() == RULE_AH1) || (source.getRule() == RULE_AL1)) {
                return 1;
            } else if (source.getRule() == RULE_I0) {
                return 0x01 << lvl;
            } else if (source.getRule() == RULE_I1) {
                return (0x01 << (2*lvl)) - (0x01 << lvl);
            }
        }
    }
    // check the result of the target node
    Edge cacheEdge = source;
    cacheEdge.setRule(RULE_X);
    long num = 0;
    if (!cache.check(source.getNodeLevel(), cacheEdge, num)) {
        // compute recursively
        std::vector<Edge> child;
        if (targetForest->getSetting().isRelation()) {
            child = std::vector<Edge>(4);
        } else {
            child = std::vector<Edge>(2);
        }
        long newNum = 0;
        for (char i=0; i<(char)child.size(); i++) {
            child[i] = targetForest->cofact(source.getNodeLevel(), source, i);
            newNum += computeCARD(source.getNodeLevel()-1, child[i]);
        }
        num = newNum;
        // save to the cache
        cache.add(source.getNodeLevel(), cacheEdge, newNum);
    }
    // consider the incoming edge rule
    int multiplier = 1;
    int adder = 0;
    if (source.getRule() == RULE_X) {
        multiplier = (targetForest->getSetting().isRelation()) ? (0x01 << (2*(lvl-source.getNodeLevel()))) : (0x01 << (lvl-source.getNodeLevel()));
    } else if ((source.getRule() == RULE_EL1) || (source.getRule() == RULE_EH1)) {
        adder = (0x01 << lvl) - (0x01 << source.getNodeLevel());
    } else if ((source.getRule() == RULE_AL0) || (source.getRule() == RULE_AH0)) {
        multiplier = (0x01 << (lvl - source.getNodeLevel())) - 1;
    } else if ((source.getRule() == RULE_AL1) || (source.getRule() == RULE_AH1)) {
        multiplier = (0x01 << (lvl - source.getNodeLevel())) - 1;
        adder = 0x01 << source.getNodeLevel();
    } else if (source.getRule() == RULE_I0) {
        multiplier = 0x01 << (lvl - source.getNodeLevel());
    } else if (source.getRule() == RULE_I1) {
        multiplier = 0x01 << (lvl - source.getNodeLevel());
        adder = (0x01 << (2*(lvl-source.getNodeLevel()))) - (0x01 << (lvl-source.getNodeLevel()));
    }
    return multiplier * num + adder;
}


// ******************************************************************
// *                                                                *
// *                       UnaryList  methods                       *
// *                                                                *
// ******************************************************************
UnaryList::UnaryList(const std::string n)
{
    reset(n);
}

UnaryOperation* UnaryList::mtfUnary(const UnaryOperationType opT, const Forest* sourceF, const Forest* targetF)
{
    UnaryOperation* prev = front;
    UnaryOperation* curr = front->next;
    while (curr) {
        if ((curr->opType == opT) && (curr->sourceForest == sourceF) && (curr->targetForest == targetF)) {
            // Move to front
            prev->next = curr->next;
            curr->next = front;
            front = curr;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return nullptr;
}

UnaryOperation* UnaryList::mtfUnary(const UnaryOperationType opT, const Forest* sourceF, const OpndType targetT)
{
    UnaryOperation* prev = front;
    UnaryOperation* curr = front->next;
    while (curr) {
        if ((curr->opType == opT) && (curr->sourceForest == sourceF) && (curr->targetType == targetT)) {
            // Move to front
            prev->next = curr->next;
            curr->next = front;
            front = curr;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return nullptr;
}

void UnaryList::searchRemove(UnaryOperation* uop)
{
    if (!front || !uop) return;
    UnaryOperation* prev = front;
    UnaryOperation* curr = front->next;
    while (curr) {
        if (curr == uop) {
            prev->next = curr->next;
            delete curr;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void UnaryList::searchRemove(Forest* forest)
{
    if (!front || !forest) return;
    // check front first
    while (front && ((front->sourceForest == forest) || (front->targetForest == forest))) {
        UnaryOperation* toRemove = front;
        front = front->next;
        delete toRemove;
    }
    // check the remaining
    UnaryOperation* curr = front;
    while (curr && curr->next) {
        if ((curr->next->sourceForest == forest) || (curr->next->targetForest == forest)) {
            UnaryOperation* toRemove = curr->next;
            curr->next = curr->next->next;
            delete toRemove;
        } else {
            curr = curr->next;
        }
    }
}

void UnaryList::searchSweepCache(Forest* forest)
{
    if (!front || !forest) return;
    UnaryOperation* curr = front;
    bool isSource = curr->sourceForest == forest;
    bool isTarget = curr->targetForest == forest;
    while (curr) {
        if (isSource || isTarget) {
            // sweep cache
            // if opType is COPY, check if it's source or target forest
            if (curr->opType == UnaryOperationType::UOP_COPY) {
                curr->cache.sweep(forest, (isSource) ? 1 : 0);
            } else if (curr->opType == UnaryOperationType::UOP_COMPLEMENT) {
                if (isTarget) {
                    curr->cache.sweep(forest, 0);
                    curr->cache.sweep(forest, 1);
                }
            } else if (curr->opType == UnaryOperationType::UOP_CARDINALITY) {
                curr->cache.sweep(forest, 1);
            } else {
                // other operations, TBD
                std::cout << "other operation not implemented" << std::endl;
            }
        }
        curr = curr->next;
        if (curr) {
            isSource = curr->sourceForest == forest;
            isTarget = curr->targetForest == forest;
        }
    }
}

void UnaryList::reportCacheStat(std::ostream& out, int format) const
{
    UnaryOperation* curr = front;
    uint64_t n = 0;
    out << "UnaryList:\n";
    while (curr) {
        out << "CT " << n << "=============================\n";
        out << "Source Forest: " << curr->sourceForest << "\n";
        out << "Result Forest: " << curr->targetForest << "\n";
        out << "Operation Type: " << (int)curr->opType << "\n";
        out << "----------------------------------\n";
        curr->cache.reportStat(out, format);
        curr = curr->next;
        n++;
    }
}


// ******************************************************************
// *                                                                *
// *                                                                *
// *                 BinaryOperation  methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************
BinaryOperation::BinaryOperation(BinaryOperationType type, Forest* source1, Forest* source2, Forest* res)
:opType(type)
{
    source1Forest = source1;
    source2Forest = source2;
    resForest = res;
}
BinaryOperation::~BinaryOperation()
{
    cache.~ComputeTable();
    source1Forest = nullptr;
    source2Forest = nullptr;
    resForest = nullptr;
}

void BinaryOperation::compute(const Func& source1, const Func& source2, Func& res)
{
    if (!checkForestCompatibility()) {
        throw error(ErrCode::INVALID_OPERATION, __FILE__, __LINE__);
    }
    Edge ans;
    Func source1Equ;
    Func source2Equ;
    uint16_t numVars = resForest->getSetting().getNumVars();
    // Unary operation to copy from source1 forest to target forest
    UnaryOperation* cp1 = UOPs.find(UnaryOperationType::UOP_COPY, source1.getForest(), res.getForest());
    if (!cp1) {
        cp1 = UOPs.add(new UnaryOperation(UnaryOperationType::UOP_COPY, source1.getForest(), res.getForest()));
    }
    // copy sources to the same target forest, if they are both set-of-states or relation
    if (source1Forest->getSetting().isRelation() == source2Forest->getSetting().isRelation()) {
        source1Equ = Func(res.getForest());
        cp1->compute(source1, source1Equ);
        source2Equ = Func(res.getForest());
        UnaryOperation* cp2 = UOPs.find(UnaryOperationType::UOP_COPY, source2.getForest(), res.getForest());
        if (!cp2) {
            cp2 = UOPs.add(new UnaryOperation(UnaryOperationType::UOP_COPY, source2.getForest(), res.getForest()));
        }
        cp2->compute(source2, source2Equ);
    }
    // compute the result
    if ((opType == BinaryOperationType::BOP_UNION)
        || (opType == BinaryOperationType::BOP_INTERSECTION)) { // more operations
        ans = computeElmtWise(numVars, source1Equ.getEdge(), source2Equ.getEdge());
    } else if (opType == BinaryOperationType::BOP_PREIMAGE) {
        ans = computeIMAGE(numVars, source1.getEdge(), source2.getEdge(), 1);
        Func ansEqu(source1.getForest(), ans);
        cp1->compute(ansEqu, res);
        return;
    } else if (opType == BinaryOperationType::BOP_POSTIMAGE) {
        ans = computeIMAGE(numVars, source1.getEdge(), source2.getEdge());
        Func ansEqu(source1.getForest(), ans);
        cp1->compute(ansEqu, res);
        return;
    } else {
        // TBD
    }
    // passing result
    res.setEdge(ans);
    // cache.reportStat(std::cout);
}

void BinaryOperation::compute(const Func& source1, const ExplictFunc source2, Func& res)
{
    //
}

bool BinaryOperation::checkForestCompatibility() const
{
    bool ans = 1;
    // TBD
    return ans;
}

Edge BinaryOperation::computeElmtWise(const uint16_t lvl, const Edge& source1, const Edge& source2)
{
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "compute elementwise: lvl: " << lvl << "; e1: ";
    source1.print(std::cout);
    std::cout << "; e2: ";
    source2.print(std::cout);
    std::cout << std::endl;
#endif
    // the final answer
    Edge ans;
    Edge e1, e2;
    // normalize edges
    e1 = resForest->normalizeEdge(lvl, source1);
    e2 = resForest->normalizeEdge(lvl, source2);
    // Base case 1: two edges are the same
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "checking base case 1\n";
#endif
    if (e1 == e2) {
        if ((opType == BinaryOperationType::BOP_UNION)
            || (opType == BinaryOperationType::BOP_INTERSECTION)) { // more operations?
            return e1;
        }
    }
    // Base case 2: two edges are complemented
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "checking base case 2\n";
#endif
    if (e1.isComplementTo(e2)) {
        if (opType == BinaryOperationType::BOP_UNION) {
            EdgeHandle constant = makeTerminal(INT, 1);
            if (resForest->getSetting().getValType() == FLOAT) {
                constant = makeTerminal(FLOAT, 1.0f);
            }
            packRule(constant, RULE_X);
            ans.setEdgeHandle(constant);
            ans = resForest->normalizeEdge(lvl, ans);
            return ans;
        } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
            EdgeHandle constant = makeTerminal(INT, 0);
            if (resForest->getSetting().getValType() == FLOAT) {
                constant = makeTerminal(FLOAT, 0.0f);
            }
            packRule(constant, RULE_X);
            ans.setEdgeHandle(constant);
            ans = resForest->normalizeEdge(lvl, ans);
            return ans;
        } else {
            // more operations, TBD
        }
    }
    // reordering
    uint16_t m1, m2;
    m1 = e1.getNodeLevel();
    m2 = e2.getNodeLevel();
    if (m1<m2) {
        SWAP(e1, e2);
        SWAP(m1, m2);
    }
    // Base case 3: one edge is constant ONE edge
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "checking base case 3\n";
#endif
    if (e1.isConstantOne() || e2.isConstantOne()) {
        if (opType == BinaryOperationType::BOP_UNION) {
            EdgeHandle constant = makeTerminal(INT, 1);
            if (resForest->getSetting().getValType() == FLOAT) {
                constant = makeTerminal(FLOAT, 1.0f);
            }
            packRule(constant, RULE_X);
            ans.setEdgeHandle(constant);
            ans = resForest->normalizeEdge(lvl, ans);
            return ans;
        } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
            return resForest->normalizeEdge(lvl, (e1.isConstantOne()) ? e2 : e1);
        } else {
            // more operations, TBD
        }
    }
    // Base case 4: one edge is constant ZERO edge
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "checking base case 4\n";
#endif
    if (e1.isConstantZero() || e2.isConstantZero()) {
        if (opType == BinaryOperationType::BOP_UNION) {
            return resForest->normalizeEdge(lvl, (e1.isConstantZero()) ? e2 : e1);
        } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
            EdgeHandle constant = makeTerminal(INT, 0);
            if (resForest->getSetting().getValType() == FLOAT) {
                constant = makeTerminal(FLOAT, 0.0f);
            }
            packRule(constant, RULE_X);
            ans.setEdgeHandle(constant);
            ans = resForest->normalizeEdge(lvl, ans);
            return ans;
        } else {
            // more operations, TBD
        }
    }

    // check cache here
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "checking cache\n";
#endif
    if (cache.check(lvl, e1, e2, ans) && !resForest->getSetting().isRelation()) {
        return ans;
    } else if (cache.check(m1, e1, e2, ans) && resForest->getSetting().isRelation()) {
        EdgeLabel root = 0;
        if (e1.getRule() == e2.getRule()) {
            packRule(root, e1.getRule());
            ans = resForest->mergeEdge(lvl, m1, root, ans);
            return ans;
        } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
            // assuming I1 is not considered, TBD
            if (lvl-m1 == 0) {
                packRule(root, RULE_X);
            } else {
                packRule(root, RULE_I0);
            }
            ans = resForest->mergeEdge(lvl, m1, root, ans);
            return ans;
        } else {
            // for operation like UNION
            packRule(root, RULE_X);
            std::vector<Edge> tmp(4);
            if (isRuleI(e1.getRule())) {
                tmp[1] = e2;
                tmp[2] = e2;
            } else {
                tmp[1] = e1;
                tmp[2] = e1;
            }
            tmp[1].setRule(RULE_X);
            tmp[2].setRule(RULE_X);
            for (uint16_t k=m1+1; k<=lvl; k++) {
                tmp[0] = ans;
                tmp[3] = ans;
                ans = resForest->reduceEdge(k, root, k, tmp);
            }
            return ans;
        }
    }

    // Here means we can not find the result in cache, computation is needed
    // Case that edge1 is a short edge
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "check if recursive computing\n";
#endif
    if (m1 == lvl) {
        std::vector<Edge> child1, child2, tmp;
        if (resForest->getSetting().isRelation()) {
            child1 = std::vector<Edge>(4);
            child2 = std::vector<Edge>(4);
            tmp = std::vector<Edge>(4);
        } else {
            child1 = std::vector<Edge>(2);
            child2 = std::vector<Edge>(2);
            tmp = std::vector<Edge>(2);
        }
        for (char i=0; i<(char)child1.size(); i++) {
            child1[i] = resForest->cofact(lvl, e1, i);
            child2[i] = resForest->cofact(lvl, e2, i);
            tmp[i] = computeElmtWise(lvl-1, child1[i], child2[i]);
        }
        EdgeLabel root = 0;
        packRule(root, RULE_X);
        ans = resForest->reduceEdge(lvl, root, lvl, tmp);
        // save to cache
        cache.add(lvl, e1, e2, ans);
        return ans;
    }

    // Here we have lvl>m1>=m2
    if (!resForest->getSetting().isRelation()) {
        // For BDDs, it's time to decide pattern types and use pattern operations
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "patterns computing\n";
#endif
        char t1, t2;
        t1 = rulePattern(e1.getRule());
        t2 = rulePattern(e2.getRule());
        if (t1 == 'L') {
            if (t2 == 'L' || t2 == 'U') {
                ans = operateLL(lvl, e1, e2);
            } else {
                ans = operateLH(lvl, e1, e2);
            }
        } else if (t1 == 'H') {
            if (t2 == 'H' || t2 == 'U') {
                ans = operateHH(lvl, e1, e2);
            } else {
                ans = operateLH(lvl, e2, e1);
            }
        } else {
            if (t2 == 'L' || t2 == 'U') {
                ans = operateLL(lvl, e1, e2);
            } else {
                ans = operateHH(lvl, e1, e2);
            }
        }
        // save cache
        cache.add(lvl, e1, e2, ans);
        return ans;
    } else {
        // For MXDs, recursively compute the bottom part, then cache and merge
        std::vector<Edge> child1(4), child2(4), tmp(4);
        for (char i=0; i<(char)child1.size(); i++) {
            child1[i] = resForest->cofact(m1, e1, i);
            child2[i] = resForest->cofact(m1, e2, i);
            tmp[i] = computeElmtWise(m1-1, child1[i], child2[i]);
        }
        EdgeLabel root = 0;
        packRule(root, RULE_X);
        ans = resForest->reduceEdge(m1, root, m1, tmp);
        // save to cache
        cache.add(m1, e1, e2, ans);
        // merge edge
        if (e1.getRule() == e2.getRule()) {
            packRule(root, e1.getRule());
            ans = resForest->mergeEdge(lvl, m1, root, ans);
        } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
            // assuming I1 is not considered, TBD
            packRule(root, RULE_I0);
            ans = resForest->mergeEdge(lvl, m1, root, ans);
        } else {
            // for operation like UNION
            packRule(root, RULE_X);
            if (isRuleI(e1.getRule())) {
                tmp[1] = e2;
                tmp[2] = e2;
            } else {
                tmp[1] = e1;
                tmp[2] = e1;
            }
            tmp[1].setRule(RULE_X);
            tmp[2].setRule(RULE_X);
            for (uint16_t k=m1+1; k<=lvl; k++) {
                tmp[0] = ans;
                tmp[3] = ans;
                ans = resForest->reduceEdge(k, root, k, tmp);
            }
        }
        return ans;
    }
}

Edge BinaryOperation::computeUNION(const uint16_t lvl, const Edge& source1, const Edge& source2)
{
    Edge ans;
    Edge e1, e2;
    // normalize edges
    e1 = resForest->normalizeEdge(lvl, source1);
    e2 = resForest->normalizeEdge(lvl, source2);

    // Base case 1: two edges are the same
    if (e1 == e2) return e1;
    // Base case 2: two edges are complemented
    // Base case 3: one edge is constant ONE edge
    if (e1.isComplementTo(e2) || e1.isConstantOne() || e2.isConstantOne()) {
        EdgeHandle constant = makeTerminal(INT, 1);
        if (resForest->getSetting().getValType() == FLOAT) {
            constant = makeTerminal(FLOAT, 1.0f);
        }
        packRule(constant, RULE_X);
        ans.setEdgeHandle(constant);
        ans = resForest->normalizeEdge(lvl, ans);
        return ans;
    }
    // Base case 4: one edge is constant ZERO edge
    if (e1.isConstantZero()) return e2;
    if (e2.isConstantZero()) return e1;

    uint16_t m1, m2;
    m1 = e1.getNodeLevel();
    m2 = e2.getNodeLevel();
    // ordering
    if (m1<m2) {
        SWAP(e1, e2);
        SWAP(m1, m2);
    }
    
    // check cache here
    if (cache.check(lvl, e1, e2, ans)) return ans;

    // Case that edge1 is a short edge
    if (m1 == lvl) {
        Edge x1, y1, x2, y2;
        x1 = resForest->cofact(lvl, e1, 0);
        y1 = resForest->cofact(lvl, e1, 1);
        x2 = resForest->cofact(lvl, e2, 0);
        y2 = resForest->cofact(lvl, e2, 1);
        std::vector<Edge> child(2);
        child[0] = computeUNION(lvl-1, x1, x2);
        child[1] = computeUNION(lvl-1, y1, y2);
        EdgeLabel root = 0;
        packRule(root, RULE_X);
        ans = resForest->reduceEdge(lvl, root, lvl, child);

        // save to cache
        cache.add(lvl, e1, e2, ans);
        return ans;
    }

    // Here we have m1>=m2, it's time to decide pattern types and use pattern operations
    char t1, t2;
    t1 = rulePattern(e1.getRule());
    t2 = rulePattern(e2.getRule());
    if (t1 == 'L') {
        if (t2 == 'L' || t2 == 'U') {
            ans = operateLL(lvl, e1, e2);
        } else {
            ans = operateLH(lvl, e1, e2);
        }
    } else if (t1 == 'H') {
        if (t2 == 'H' || t2 == 'U') {
            ans = operateHH(lvl, e1, e2);
        } else {
            ans = operateLH(lvl, e2, e1);
        }
    } else {
        if (t2 == 'L' || t2 == 'U') {
            ans = operateLL(lvl, e1, e2);
        } else {
            ans = operateHH(lvl, e1, e2);
        }
    }
    // save cache
    cache.add(lvl, e1, e2, ans);
    return ans;
}

Edge BinaryOperation::computeINTERSECTION(const uint16_t lvl, const Edge& source1, const Edge& source2)
{
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "compute INTERSECTION: lvl: " << lvl << "; e1: ";
    source1.print(std::cout);
    std::cout << "; e2: ";
    source2.print(std::cout);
    std::cout << std::endl;
#endif

    Edge ans;
    Edge e1, e2;
    // normalize edges
    e1 = resForest->normalizeEdge(lvl, source1);
    e2 = resForest->normalizeEdge(lvl, source2);

    // Base case 1: two edges are the same
    if (e1 == e2) return e1;
    // Base case 2: two edges are complemented
    // Base case 3: one edge is constant ZERO edge
    if (e1.isComplementTo(e2) || e1.isConstantZero() || e2.isConstantZero()) {
        EdgeHandle constant = makeTerminal(INT, 0);
        if (resForest->getSetting().getValType() == FLOAT) {
            constant = makeTerminal(FLOAT, 0.0f);
        }
        packRule(constant, RULE_X);
        ans.setEdgeHandle(constant);
        ans = resForest->normalizeEdge(lvl, ans);
        return ans;
    }
    // Base case 4: one edge is constant ONE edge
    if (e1.isConstantOne()) return e2;
    if (e2.isConstantOne()) return e1;

    uint16_t m1, m2;
    m1 = e1.getNodeLevel();
    m2 = e2.getNodeLevel();
    // ordering
    if (m1 < m2) {
        SWAP(e1, e2);
        SWAP(m1, m2);
    }
    
    // check cache here
    if (cache.check(lvl, e1, e2, ans)) return ans;

    // Case that edge1 is a short edge
    if (m1 == lvl) {
        Edge x1, y1, x2, y2;
        x1 = resForest->cofact(lvl, e1, 0);
        y1 = resForest->cofact(lvl, e1, 1);
        x2 = resForest->cofact(lvl, e2, 0);
        y2 = resForest->cofact(lvl, e2, 1);
        std::vector<Edge> child(2);
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "\trecursive 0 from level: " << lvl << std::endl;
#endif
        child[0] = computeINTERSECTION(lvl-1, x1, x2);
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "\trecursive 1 from level: " << lvl << std::endl;
#endif
        child[1] = computeINTERSECTION(lvl-1, y1, y2);
        EdgeLabel root = 0;
        packRule(root, RULE_X);
        ans = resForest->reduceEdge(lvl, root, lvl, child);

        // save to cache
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "\tcase 1: save to cache: ";
    ans.print(std::cout);
    std::cout << std::endl;
#endif
        cache.add(lvl, e1, e2, ans);
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "\tsave done\n";
#endif
        return ans;
    }

    // Here we have m1>=m2, it's time to decide pattern types and use pattern operations
    char t1, t2;
    t1 = rulePattern(e1.getRule());
    t2 = rulePattern(e2.getRule());
    if (t1 == 'L') {
        if (t2 == 'L' || t2 == 'U') {
            ans = operateLL(lvl, e1, e2);
        } else {
            ans = operateLH(lvl, e1, e2);
        }
    } else if (t1 == 'H') {
        if (t2 == 'H' || t2 == 'U') {
            ans = operateHH(lvl, e1, e2);
        } else {
            ans = operateLH(lvl, e2, e1);
        }
    } else {
        if (t2 == 'L' || t2 == 'U') {
            ans = operateLL(lvl, e1, e2);
        } else {
            ans = operateHH(lvl, e1, e2);
        }
    }
    // save cache
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "\tcase 2: save to cache: ";
    ans.print(std::cout);
    std::cout << std::endl;
#endif
    cache.add(lvl, e1, e2, ans);
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "\tsave done\n";
#endif
    return ans;
}

Edge BinaryOperation::computeIMAGE(const uint16_t lvl, const Edge& source1, const Edge& trans, bool isPre)
{
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << ((isPre)?"Pre":"Post") << "IMAGE: lvl: " << (uint16_t)lvl << "; s: ";
    source1.print(std::cout);
    std::cout << "; r: ";
    trans.print(std::cout);
    std::cout << std::endl;
#endif

    Edge ans;
    Edge s, r;
    // normalize edges
    s = source1Forest->normalizeEdge(lvl, source1);
    r = source2Forest->normalizeEdge(lvl, trans);

    // Base case 1: empty state or relation
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "checking base case 1\n";
#endif
    if (s.isConstantZero() || r.isConstantZero()) {
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "base case 1\n";
#endif
        EdgeHandle constant = makeTerminal(INT, 0);
        if (source1Forest->getSetting().getValType() == FLOAT) {
            constant = makeTerminal(FLOAT, 0.0f);
        }
        packRule(constant, RULE_X);
        ans.setEdgeHandle(constant);
        ans = source1Forest->normalizeEdge(lvl, ans);
        return ans;
    }

    // Base case 2: identity or redundant relation
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "checking base case 2\n";
#endif
    if (r.getNodeLevel() == 0) {
        if ((r.getRule() == RULE_I0) && (isTerminalOne(r.getEdgeHandle()) || isTerminalZero(r.getEdgeHandle())) && (r.getComp() ^ isTerminalOne(r.getEdgeHandle()))) {
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "base case 2: identity\n";
#endif
            return s;
        } else if (r.isConstantOne()) {
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "base case 2: redundant\n";
#endif
            EdgeHandle constant = makeTerminal(INT, 1);
            if (source1Forest->getSetting().getValType() == FLOAT) {
                constant = makeTerminal(FLOAT, 1.0f);
            }
            packRule(constant, RULE_X);
            ans.setEdgeHandle(constant);
            ans = source1Forest->normalizeEdge(lvl, ans);
            return ans;
        }
    }

    uint16_t m = (s.getNodeLevel() > r.getNodeLevel()) ? s.getNodeLevel() : r.getNodeLevel();
    // assertion: m>0
    Edge sCache = s;
    Edge rCache = r;
    if (s.getNodeLevel() == m) sCache.setRule(RULE_X);
    if (r.getNodeLevel() == m) rCache.setRule(RULE_X);
    // check cache
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "checking cache\n";
#endif
    if (!cache.check(m, sCache, rCache, ans)) {
        // not cached, computing needed
        std::vector<Edge> child(2);
        EdgeHandle constant = makeTerminal(INT, 0);
        if (source1Forest->getSetting().getValType() == FLOAT) {
            constant = makeTerminal(FLOAT, 0.0f);
        }
        packRule(constant, RULE_X);
        for (size_t i=0; i<2; i++) {
            child[i].setEdgeHandle(constant);
            child[i] = source1Forest->normalizeEdge(m-1, child[i]);
        }
        // recursive computing
        Edge sRec, rRec, resRec;
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "recursive computing\n";
#endif
        for (char i=0; i<4; i++) {
            char s0Idx = (isPre) ? (i&(0x01)) : ((i&(0x01<<1))>>1);
            char s1Idx = (isPre) ? ((i&(0x01<<1))>>1) : (i&(0x01));
            sRec = source1Forest->cofact(m, s, s0Idx);
            rRec = source2Forest->cofact(m, r, i);
            resRec = computeIMAGE(m-1, sRec, rRec, isPre);
            // Union of BDDs operation required
            BinaryOperation* un = BOPs.find(BinaryOperationType::BOP_UNION, source1Forest, source1Forest, source1Forest);
            if (!un) {
                un = BOPs.add(new BinaryOperation(BinaryOperationType::BOP_UNION, source1Forest, source1Forest, source1Forest));
            }
            child[s1Idx] = un->computeElmtWise(m-1, child[s1Idx], resRec);
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "after union child[" << (int)s1Idx << "]: ";
    child[s1Idx].print(std::cout);
    std::cout << std::endl;
#endif
        }
        EdgeLabel root = 0;
        packRule(root, RULE_X);
        ans = source1Forest->reduceEdge(m, root, m, child);
        // cache
        cache.add(m, sCache, rCache, ans);
    }
    // merge with the incoming edge rule
    EdgeLabel incoming = 0;
    packRule(incoming, (r.getRule() == RULE_I0)?s.getRule():RULE_X);    // TBD for rex
    ans = source1Forest->mergeEdge(lvl, m, incoming, ans);
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "after " << ((isPre)?"Pre":"Post") << "IMAGE: lvl: " << lvl << "; s: ";
    source1.print(std::cout);
    std::cout << "; r: ";
    trans.print(std::cout);
    std::cout << std::endl;
    std::cout << "\tresult: ";
    ans.print(std::cout);
    std::cout << std::endl;
    if (ans.getNodeLevel() == 2 && ans.getNodeHandle() == 4) {
        Func functionErr(resForest);
        functionErr.setEdge(ans);
        DotMaker dot(resForest, "error_edge");
        dot.buildGraph(functionErr);
        dot.runDot("pdf");
    }
#endif
    return ans;
}

Edge BinaryOperation::operateLL(const uint16_t lvl, const Edge& e1, const Edge& e2)
{
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "operateLL: lvl: " << lvl << "; e1: ";
    e1.print(std::cout);
    std::cout << "; e2: ";
    e2.print(std::cout);
    std::cout << std::endl;
#endif

    uint16_t m1, m2;
    m1 = e1.getNodeLevel();
    m2 = e2.getNodeLevel();

    Edge x1, x2, y1, y2;
    x1 = e1.part(0);
    x2 = e2.part(0);
    y1 = e1.part(1);
    y2 = (m1==m2) ? e2.part(1) : resForest->cofact(m1+1, e2, 1);

    Edge x, y;
    if (opType == BinaryOperationType::BOP_UNION) {
        x = computeUNION(m1, x1, x2);
        y = computeUNION(m1, y1, y2);
    } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
        x = computeINTERSECTION(m1, x1, x2);
        y = computeINTERSECTION(m1, y1, y2);
    }
    // more operations TBD
    Edge ans = resForest->buildHalf(lvl, m1+1, x, y, 1);
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "build Low with x: ";
    x.print(std::cout);
    std::cout << "; y: ";
    y.print(std::cout);
    std::cout << std::endl;
    std::cout << "after operateHH from lvl: " << lvl << "; result: ";
    ans.print(std::cout);
    std::cout << std::endl;
#endif
    return ans;

}

Edge BinaryOperation::operateHH(const uint16_t lvl, const Edge& e1, const Edge& e2)
{
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "operateHH: lvl: " << lvl << "; e1: ";
    e1.print(std::cout);
    std::cout << "; e2: ";
    e2.print(std::cout);
    std::cout << std::endl;
#endif
    uint16_t m1, m2;
    m1 = e1.getNodeLevel();
    m2 = e2.getNodeLevel();

    Edge x1, x2, y1, y2;
    x1 = e1.part(0);
    x2 = (m1==m2) ? e2.part(0) : resForest->cofact(m1+1, e2, 0);
    y1 = e1.part(1);
    y2 = e2.part(1);

    Edge x, y;
    if (opType == BinaryOperationType::BOP_UNION) {
        x = computeUNION(m1, x1, x2);
        y = computeUNION(m1, y1, y2);
    } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
        x = computeINTERSECTION(m1, x1, x2);
        y = computeINTERSECTION(m1, y1, y2);
    }
    // more operations TBD
    Edge ans = resForest->buildHalf(lvl, m1+1, x, y, 0);
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "build High with x: ";
    x.print(std::cout);
    std::cout << "; y: ";
    y.print(std::cout);
    std::cout << std::endl;
    std::cout << "after operateHH from lvl: " << lvl << "; result: ";
    ans.print(std::cout);
    std::cout << std::endl;
#endif
    return ans;

}

Edge BinaryOperation::operateLH(const uint16_t lvl, const Edge& e1, const Edge& e2)
{
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "operateLH: lvl: " << lvl << "; e1: ";
    e1.print(std::cout);
    std::cout << "; e2: ";
    e2.print(std::cout);
    std::cout << std::endl;
#endif
    uint16_t m1, m2, m;
    m1 = e1.getNodeLevel();
    m2 = e2.getNodeLevel();

    Edge x1, x2, y1, y2;
    x1 = e1.part(0);
    y2 = e2.part(1);
    if (m1 == m2) {
        y1 = e1.part(1);
        x2 = e2.part(0);
        m = m1;
    } else if (m1>m2) {
        y1 = e1.part(1);
        x2 = resForest->cofact(m1+1, e2, 0);
        m = m1;
    } else {
        y1 = resForest->cofact(m2+1, e1, 1);
        x2 = e2.part(0);
        m = m2;
    }
    Edge x, y, z;
    if (opType == BinaryOperationType::BOP_UNION) {
        x = computeUNION(m, x1, x2);
        z = computeUNION(m, y1, y2);
        if (lvl - m == 1) {
            EdgeLabel root = 0;
            packRule(root, RULE_X);
            std::vector<Edge> child(2);
            child[0] = x;
            child[1] = z;
            return resForest->reduceEdge(lvl, root, lvl, child);
        }
        y = computeUNION(m, x1, y2);
    } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
        x = computeINTERSECTION(m, x1, x2);
        z = computeINTERSECTION(m, y1, y2);
        if (lvl - m == 1) {
            EdgeLabel root = 0;
            packRule(root, RULE_X);
            std::vector<Edge> child(2);
            child[0] = x;
            child[1] = z;
            return resForest->reduceEdge(lvl, root, lvl, child);
        }
        y = computeINTERSECTION(m, x1, y2);
    }
    // more operations TBD
    Edge ans = resForest->buildUmb(lvl, m+1, x, y, z);
#ifdef BRAVE_DD_OPERATION_TRACE
    std::cout << "build Umbrella with x: ";
    x.print(std::cout);
    std::cout << "; y: ";
    y.print(std::cout);
    std::cout << "; z: ";
    z.print(std::cout);
    std::cout << std::endl;
    std::cout << "after operateHH from lvl: " << lvl << "; result: ";
    ans.print(std::cout);
    std::cout << std::endl;
#endif
    return ans;
}

// ******************************************************************
// *                                                                *
// *                       BinaryList  methods                      *
// *                                                                *
// ******************************************************************
BinaryList::BinaryList(const std::string n)
{
    reset(n);
}

BinaryOperation* BinaryList::mtfBinary(const BinaryOperationType opT, const Forest* source1F, const Forest* source2F, const Forest* resF)
{
    BinaryOperation* prev = front;
    BinaryOperation* curr = front->next;
    while (curr) {
        if ((curr->opType == opT) && (curr->source1Forest == source1F) && (curr->source2Forest == source2F) && (curr->resForest == resF)) {
            // Move to front
            prev->next = curr->next;
            curr->next = front;
            front = curr;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return nullptr;
}

BinaryOperation* BinaryList::mtfBinary(const BinaryOperationType opT, const Forest* source1F, const OpndType source2T, const Forest* resF)
{
    BinaryOperation* prev = front;
    BinaryOperation* curr = front->next;
    while (curr) {
        if ((curr->opType == opT) && (curr->source1Forest == source1F) && (curr->source2Type == source2T) && (curr->resForest == resF)) {
            // Move to front
            prev->next = curr->next;
            curr->next = front;
            front = curr;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return nullptr;
}

void BinaryList::searchRemove(BinaryOperation* bop)
{
    if (!front) return;
    BinaryOperation* prev = front;
    BinaryOperation* curr = front->next;
    while (curr) {
        if (curr == bop) {
            prev->next = curr->next;
            delete curr;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void BinaryList::searchRemove(Forest* forest)
{
    if (!front) return;
    // check front first
    while (front && ((front->source1Forest == forest) || (front->source2Forest == forest) || (front->resForest == forest))) {
        BinaryOperation* toRemove = front;
        front = front->next;
        delete toRemove;
    }
    // check the remaining
    BinaryOperation* curr = front;
    while (curr && curr->next) {
        if ((curr->next->source1Forest == forest) || (curr->next->source2Forest == forest) || (curr->next->resForest == forest)) {
            BinaryOperation* toRemove = curr->next;
            curr->next = curr->next->next;
            delete toRemove;
        } else {
            curr = curr->next;
        }
    }
}

void BinaryList::searchSweepCache(Forest* forest)
{
    if (!front || !forest) return;
    BinaryOperation* curr = front;
    bool isSource1 = curr->source1Forest == forest;
    bool isSource2 = curr->source2Forest == forest;
    bool isRes = curr->resForest == forest;
    while (curr) {
        if (isSource1 || isSource2 || isRes) {
            // sweep cache
            if ((curr->opType == BinaryOperationType::BOP_PREIMAGE) || (curr->opType == BinaryOperationType::BOP_POSTIMAGE)) {
                if (isSource1) {
                    curr->cache.sweep(forest, 0);
                    curr->cache.sweep(forest, 1);
                }
                if (isSource2) {
                    curr->cache.sweep(forest, 2);
                }

            } else if ((curr->opType == BinaryOperationType::BOP_UNION)
                    || (curr->opType == BinaryOperationType::BOP_INTERSECTION)) {
                if (isRes) {
                    curr->cache.sweep(forest, 0);
                    curr->cache.sweep(forest, 1);
                    curr->cache.sweep(forest, 2);
                }
            }
            // other operations TBD
        }
        curr = curr->next;
        if (curr) {
            isSource1 = curr->source1Forest == forest;
            isSource2 = curr->source2Forest == forest;
            isRes = curr->resForest == forest;
        }
    }
}

void BinaryList::reportCacheStat(std::ostream& out, int format) const
{
    BinaryOperation* curr = front;
    uint64_t n = 0;
    out << "BinaryList:\n";
    while (curr) {
        out << "CT " << n << " =============================\n";
        out << "Source1 Forest: " << curr->source1Forest << "\n";
        out << "Source2 Forest: " << curr->source2Forest << "\n";
        out << "Result Forest: " << curr->resForest << "\n";
        out << "Operation Type: " << (int)curr->opType << "\n";
        out << "----------------------------------\n";
        curr->cache.reportStat(out, format);
        curr = curr->next;
        n++;
    }
}


// // ******************************************************************
// // *                                                                *
// // *                                                                *
// // *                 NumericalOperation  methods                    *
// // *                                                                *
// // *                                                                *
// // ******************************************************************
// NumericalOperation::NumericalOperation()
// {
//     //
// }

// // ******************************************************************
// // *                                                                *
// // *                                                                *
// // *                 SaturationOperation  methods                   *
// // *                                                                *
// // *                                                                *
// // ******************************************************************
// SaturationOperation::SaturationOperation()
// {
//     //
// }