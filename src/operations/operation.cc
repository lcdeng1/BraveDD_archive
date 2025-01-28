#include "operation.h"

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

// ******************************************************************
// *                                                                *
// *                    UnaryOperation  class                       *
// *                                                                *
// ******************************************************************
UnaryOperation::UnaryOperation(UnaryList& owner, UnaryOperationType type, Forest* source, Forest* target)
:parent(owner), opType(type)
{
    sourceForest = source;
    targetForest = target;
    targetType = OpndType::FOREST;
}

void UnaryOperation::compute(const Func& source, Func& target)
{
    if (!checkForestCompatibility()) {
        throw error(ErrCode::INVALID_OPERATION, __FILE__, __LINE__);
    }
    Edge ans;
    // copy to target forest
    ans = computeCOPY(source.getEdge());
    if (opType == UnaryOperationType::UOP_COPY) {
        return;
    } else if (opType == UnaryOperationType::UOP_COMPLEMENT) {
        ans = computeCOMPLEMENT(ans);
    } else {
        // TBD
    }
    target.setEdge(ans);
    // other info TBD
}
void UnaryOperation::compute(const Func& source, long& target)
{
    if (!checkForestCompatibility()) {
        throw error(ErrCode::INVALID_OPERATION, __FILE__, __LINE__);
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
Edge UnaryOperation::computeCOPY(const Edge& source)
{
    // Direct return if the target forest is the source forest
    if (sourceForest == targetForest) {
        return source;
    }
    Edge ans;
    //TBD
    return ans;
}
Edge UnaryOperation::computeCOMPLEMENT(const Edge& source)
{
    // Assuming this is within the same forest
    Edge ans;
    //TBD
    return ans;
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
    if (!front) return;
    UnaryOperation* prev = front;
    UnaryOperation* curr = front->next;
    while (curr) {
        if (curr == uop) {
            prev->next = curr->next;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                 BinaryOperation  methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************
BinaryOperation::BinaryOperation(BinaryList& owner, BinaryOperationType type, Forest* source1, Forest* source2, Forest* res)
:parent(owner), opType(type)
{
    source1Forest = source1;
    source2Forest = source2;
    resForest = res;
}

void BinaryOperation::compute(const Func& source1, const Func& source2, Func& res)
{
    if (!checkForestCompatibility()) {
        throw error(ErrCode::INVALID_OPERATION, __FILE__, __LINE__);
    }
    Edge ans;
    // copy sources to the same target forest
    Func source1res(res.getForest());
    Func source2res(res.getForest());
    UnaryOperation* cp1 = UOPs.find(UnaryOperationType::UOP_COPY, source1.getForest(), res.getForest());
    if (!cp1) {
        cp1 = UOPs.add(new UnaryOperation(UOPs, UnaryOperationType::UOP_COPY, source1.getForest(), res.getForest()));
    }
    cp1->compute(source1, source1res);
    UnaryOperation* cp2 = UOPs.find(UnaryOperationType::UOP_COPY, source2.getForest(), res.getForest());
    if (!cp2) {
        cp2 = UOPs.add(new UnaryOperation(UOPs, UnaryOperationType::UOP_COPY, source2.getForest(), res.getForest()));
    }
    cp2->compute(source2, source2res);
    // ans = computeCOPY(source.getEdge());
    if (opType == BinaryOperationType::BOP_UNION) {
        ans = computeUNION(source1res.getEdge(), source2res.getEdge());
    } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
        ans = computeINTERSECTION(source1res.getEdge(), source2res.getEdge());
    } else {
        // TBD
    }
    res.setEdge(ans);
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
Edge BinaryOperation::computeUNION(const Edge& source1, const Edge& source2)
{
    //
    Edge ans;
    //TBD
    return ans;
}
Edge BinaryOperation::computeINTERSECTION(const Edge& source1, const Edge& source2)
{
    //
    Edge ans;
    //TBD
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
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}


// ******************************************************************
// *                                                                *
// *                                                                *
// *                 NumericalOperation  methods                    *
// *                                                                *
// *                                                                *
// ******************************************************************
NumericalOperation::NumericalOperation()
{
    //
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                 SaturationOperation  methods                   *
// *                                                                *
// *                                                                *
// ******************************************************************
SaturationOperation::SaturationOperation()
{
    //
}