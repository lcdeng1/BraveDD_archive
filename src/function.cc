#include "function.h"
#include "node.h"

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
void Func::constant(Value val)
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
void Func::variable(uint16_t lvl, Value low, Value high)
{
    // TBD
}
/* For dimention of 2 (Relation) */
// Variable Func
void Func::variable(uint16_t lvl, bool isPrime)
{
    // TBD
}
void Func::variable(uint16_t lvl, bool isPrime, Value low, Value high)
{
    // TBD
}

// 0 element not used! Doc!!
// FuncValue Func::evaluate(const std::vector<bool>& assignment) const {
//     // check applicability based on setting TBD
//     // TBD
// }
// FuncValue Func::evaluate(const std::vector<bool>& aFrom, const std::vector<bool>& aTo) const {
//     // TBD
// }


// /* Expert function for union assignments TBD */
// Edge Func::unionAssignmentRecursive(uint16_t n, Edge root, std::vector<bool> assignment, EdgeValue outcome)
// {
//     /* the final answer */
//     Edge ans(RULE_X, outcome);
//     /* terminal case */
//     if (n==0) {
//         // return the edge pointing to terminal node
//     }
//     /*
//      * here means the root edge can not capture the assignment,
//      * then we do it recursively!
//      *      we need to expand root edge if it's a long edge
//      */
//     uint16_t skipLvl;
//     skipLvl = n - unpackLevel(root.handle);
//     /* determine the down edges */
//     Edge down0(), down1();
//     if (skipLvl == 0) {
//         // it is a short edge, they are the child edges of root target node
//     } else {
//         // it is a long edge, expand based on its reduction rule if needed <==== helper function?
//     }
//     /*
//      * build new node at level n, child edges are down edges or recursive calls based on assignment[n]
//      */
//     Node tmp(this->parent->getSetting());
//     /* reduce */
//     this->parent->reduceEdge(n,unpackLabel(ans.handle), tmp, &ans);
//     return ans;
// }

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