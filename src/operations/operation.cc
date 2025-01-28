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
BinaryOperation::BinaryOperation(BinaryList& owner, BinaryOperationType type, Forest* arg1, Forest* arg2, Forest* res)
:parent(owner), opType(type)
{
    arg1Forest = arg1;
    arg2Forest = arg2;
    resForest = res;
}

void BinaryOperation::compute(const Func& arg1, const Func& arg2, Func& res)
{
    if (!checkForestCompatibility()) {
        throw error(ErrCode::INVALID_OPERATION, __FILE__, __LINE__);
    }
    Edge ans;
    // copy sources to the same target forest
    Func arg1res(res.getForest());
    Func arg2res(res.getForest());
    UnaryOperation* cp1 = UOPs.find(UnaryOperationType::UOP_COPY, arg1.getForest(), res.getForest());
    if (!cp1) {
        cp1 = UOPs.add(new UnaryOperation(UOPs, UnaryOperationType::UOP_COPY, arg1.getForest(), res.getForest()));
    }
    cp1->compute(arg1, arg1res);
    UnaryOperation* cp2 = UOPs.find(UnaryOperationType::UOP_COPY, arg2.getForest(), res.getForest());
    if (!cp2) {
        cp2 = UOPs.add(new UnaryOperation(UOPs, UnaryOperationType::UOP_COPY, arg2.getForest(), res.getForest()));
    }
    cp2->compute(arg2, arg2res);
    // ans = computeCOPY(source.getEdge());
    if (opType == BinaryOperationType::BOP_UNION) {
        ans = computeUNION(arg1res.getEdge(), arg2res.getEdge());
    } else if (opType == BinaryOperationType::BOP_INTERSECTION) {
        ans = computeINTERSECTION(arg1res.getEdge(), arg2res.getEdge());
    } else {
        // TBD
    }
    res.setEdge(ans);
}

void BinaryOperation::compute(const Func& arg1, const ExplictFunc arg2, Func& res)
{
    //
}

bool BinaryOperation::checkForestCompatibility() const
{
    bool ans = 1;
    // TBD
    return ans;
}
Edge BinaryOperation::computeUNION(const Edge& arg1, const Edge& arg2)
{
    //
    Edge ans;
    //TBD
    return ans;
}
Edge BinaryOperation::computeINTERSECTION(const Edge& arg1, const Edge& arg2)
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

BinaryOperation* BinaryList::mtfBinary(const BinaryOperationType opT, const Forest* arg1F, const Forest* arg2F, const Forest* resF)
{
    BinaryOperation* prev = front;
    BinaryOperation* curr = front->next;
    while (curr) {
        if ((curr->opType == opT) && (curr->arg1Forest == arg1F) && (curr->arg2Forest == arg2F) && (curr->resForest == resF)) {
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

BinaryOperation* BinaryList::mtfBinary(const BinaryOperationType opT, const Forest* arg1F, const OpndType arg2T, const Forest* resF)
{
    BinaryOperation* prev = front;
    BinaryOperation* curr = front->next;
    while (curr) {
        if ((curr->opType == opT) && (curr->arg1Forest == arg1F) && (curr->arg2Type == arg2T) && (curr->resForest == resF)) {
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