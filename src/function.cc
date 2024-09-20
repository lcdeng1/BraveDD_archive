#include "function.h"

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
    //
}
Func::Func(Forest* f, const Edge& e)
:edge(e)
{
    //
}
Func::~Func()
{
    //
}

/**************************** Make edge *************************/
void Func::trueFunc()
{
    // TBD
}
void Func::falseFunc()
{
    // TBD
}
/* For dimention 1 and 2 */
void Func::constant(EdgeValue val)
{
    // TBD
}
/* For dimention of 2 (Relation) */
void Func::identity(std::vector<bool> dependance)
{
    // TBD
}
/* For dimention of 1 (Set) */
void Func::variable(uint16_t lvl)
{
    // TBD
}
void Func::variable(uint16_t lvl, EdgeValue low, EdgeValue high)
{
    // TBD
}
/* For dimention of 2 (Relation) */
// Variable Func
void Func::variable(uint16_t lvl, bool isPrime)
{
    // TBD
}
void Func::variable(uint16_t lvl, bool isPrime, EdgeValue low, EdgeValue high)
{
    // TBD
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                         FuncArray  methods                     *
// *                                                                *
// *                                                                *
// ******************************************************************
FuncArray::FuncArray()
{
    //
}
FuncArray::FuncArray(Forest* f, int size)
{
    //
}
FuncArray::~FuncArray()
{
    //
}

void FuncArray::add(Func f)
{
    // TBD
}