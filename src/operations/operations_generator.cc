#include "operations_generator.h"
#include "operation.h"
#include "../function.h"
#include "../forest.h"
#include "../brave_helpers.h"

namespace BRAVE_DD {
    UnaryList       UOPs;
    BinaryList      BOPs;
    SaturationList  SOPs;
}

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                            Front End                           *
// *                                                                *
// ******************************************************************
// Unary operations
UnaryOperation* BRAVE_DD::COPY(Forest* arg, Forest* res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_COPY, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UnaryOperationType::UOP_COPY, arg, res));
}

UnaryOperation* BRAVE_DD::CARDINALITY(Forest* arg, OpndType res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_CARDINALITY, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UnaryOperationType::UOP_CARDINALITY, arg, res));
}

UnaryOperation* BRAVE_DD::COMPLEMENT(Forest* arg, Forest* res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_COMPLEMENT, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UnaryOperationType::UOP_COMPLEMENT, arg, res));
}


// ... TBD

// Binary operations
BinaryOperation* BRAVE_DD::UNION(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1 || !arg2) return nullptr;
    // commute
    if (arg1 > arg2) SWAP(arg1, arg2);
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_UNION, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_UNION, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::INTERSECTION(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1 || !arg2) return nullptr;
    // commute
    if (arg1 > arg2) SWAP(arg1, arg2);
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_INTERSECTION, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_INTERSECTION, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::PRE_IMAGE(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1 || !arg2) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_PREIMAGE, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_PREIMAGE, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::POST_IMAGE(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1 || !arg2) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_POSTIMAGE, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_POSTIMAGE, arg1, arg2, res);
    return BOPs.add(bop);
}

// ... TBD

// Saturation operation
SaturationOperation* BRAVE_DD::SATURATE(Forest* set, Forest* relations, Forest* res)
{
    if (!set || !relations) return nullptr;
    SaturationOperation* sop = SOPs.find(set, relations, res);
    if (sop) return sop;
    sop = new SaturationOperation(set, relations, res);
    return SOPs.add(sop);
}

SaturationOperation* BRAVE_DD::PRE_SATURATE(Forest* set, Forest* relations, Forest* res)
{
    if (!set || !relations) return nullptr;
    SaturationOperation* sop = SOPs.find(set, relations, res, 1);
    if (sop) return sop;
    sop = new SaturationOperation(set, relations, res);
    sop->setDirection(1);
    return SOPs.add(sop);
}

void OperationsGenerator::buildUnaryOperations(Forest* forest) {
    // Build all unary operations for the forest
    buildUnaryOperation(forest, UnaryOperationType::UOP_COPY, forest);
    buildUnaryOperation(forest, UnaryOperationType::UOP_COMPLEMENT, forest);
    buildUnaryOperation(forest, UnaryOperationType::UOP_CARDINALITY, nullptr);
    // Add more unary operations as needed
}

void OperationsGenerator::buildBinaryOperations(Forest* forest) {
    // Build all binary operations for the forest
    buildBinaryOperation(forest, BinaryOperationType::BOP_UNION, forest);
    buildBinaryOperation(forest, BinaryOperationType::BOP_INTERSECTION, forest);
    // Add more binary operations as needed
}

void OperationsGenerator::buildUnaryOperation(Forest* forest, UnaryOperationType opT, Forest* targetF) {
    // Check if the operation already exists
    UnaryOperation* op = UOPs.find(opT, forest, targetF);
    if (!op) {
        // Create the unary operation
        op = UOPs.add(new UnaryOperation(opT, forest, targetF));
    }
}

void OperationsGenerator::buildBinaryOperation(Forest* forest, BinaryOperationType opT, Forest* targetF) {
    // Check if the operation already exists
    BinaryOperation* op = BOPs.find(opT, forest, forest, targetF);
    if (!op) {
        // Create the binary operation
        op = BOPs.add(new BinaryOperation(opT, forest, forest, targetF));
    }
}