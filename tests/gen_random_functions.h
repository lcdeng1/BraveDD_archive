#include "brave_dd.h"
#include <random>

using namespace BRAVE_DD;

long seed =123456789;

std::mt19937 gen(seed);   
std::uniform_int_distribution<uint32_t> random06(0, 6);

/* Random function generating value between 0 and 1 */
double random01()
{
  const long MODULUS = 2147483647L;
  const long MULTIPLIER = 48271L;
  const long Q = MODULUS / MULTIPLIER;
  const long R = MODULUS % MULTIPLIER;

  long t = MULTIPLIER * (seed % Q) - R * (seed / Q);
  if (t > 0) {
    seed = t;
  } else {
    seed = t + MODULUS;
  }
  return ((double) seed / MODULUS);
}

void decimalToAssignment(long long decimal, std::vector<bool>& assignment)
{
    for (size_t k=1; k<=assignment.size()-1; k++) {
        assignment[k] = decimal & (1<<(k-1));
    }
}

Edge buildSetEdge(Forest* forest,
                uint16_t lvl,
                std::vector<bool>& fun,
                int start, int end)
{
    std::vector<Edge> child(2);
    EdgeLabel label = 0;
    packRule(label, RULE_X);
    if (lvl == 1) {
        child[0].setEdgeHandle(makeTerminal(INT, fun[start]?1:0));
        if (forest->getSetting().getValType() == FLOAT) {
            child[0].setEdgeHandle(makeTerminal(FLOAT, fun[start]?1.0f:0.0f));
        }
        child[1].setEdgeHandle(makeTerminal(INT, fun[end]?1:0));
        if (forest->getSetting().getValType() == FLOAT) {
            child[1].setEdgeHandle(makeTerminal(FLOAT, fun[end]?1.0f:0.0f));
        }
        child[0].setRule(RULE_X);
        child[1].setRule(RULE_X);
        return forest->reduceEdge(lvl, label, lvl, child);
    }
    child[0] = buildSetEdge(forest, lvl-1, fun, start, start+(1<<(lvl-1))-1);
    child[1] = buildSetEdge(forest, lvl-1, fun, start+(1<<(lvl-1)), end);
    return forest->reduceEdge(lvl, label, lvl, child);
}

Edge buildRelEdge(Forest* forest,
    uint16_t lvl,
    std::vector<bool>& fun,
    int start, int end)
{
std::vector<Edge> child(4);
EdgeLabel label = 0;
packRule(label, RULE_X);
if (lvl == 1) {
child[0].setEdgeHandle(makeTerminal(INT, fun[start]?1:0));
if (forest->getSetting().getValType() == FLOAT) {
child[0].setEdgeHandle(makeTerminal(FLOAT, fun[start]?1.0f:0.0f));
}
child[1].setEdgeHandle(makeTerminal(INT, fun[start+1]?1:0));
if (forest->getSetting().getValType() == FLOAT) {
child[1].setEdgeHandle(makeTerminal(FLOAT, fun[start+1]?1.0f:0.0f));
}
child[2].setEdgeHandle(makeTerminal(INT, fun[start+2]?1:0));
if (forest->getSetting().getValType() == FLOAT) {
child[2].setEdgeHandle(makeTerminal(FLOAT, fun[start+2]?1.0f:0.0f));
}
child[3].setEdgeHandle(makeTerminal(INT, fun[end]?1:0));
if (forest->getSetting().getValType() == FLOAT) {
child[3].setEdgeHandle(makeTerminal(FLOAT, fun[end]?1.0f:0.0f));
}
child[0].setRule(RULE_X);
child[1].setRule(RULE_X);
child[2].setRule(RULE_X);
child[3].setRule(RULE_X);
return forest->reduceEdge(lvl, label, lvl, child);
}
int end0 = start + (1<<(2*lvl-2)) - 1;
int end1 = end0 + (1<<(2*lvl-2));
int end2 = end1 + (1<<(2*lvl-2));
child[0] = buildRelEdge(forest, lvl-1, fun, start, end0);
child[1] = buildRelEdge(forest, lvl-1, fun, end0+1, end1);
child[2] = buildRelEdge(forest, lvl-1, fun, end1+1, end2);
child[3] = buildRelEdge(forest, lvl-1, fun, end2+1, end);
return forest->reduceEdge(lvl, label, lvl, child);
}

Edge buildEvSetEdge(Forest* forest,
                    uint16_t lvl,
                    std::vector<Value>& fun,
                    int start, int end)
{
    std::vector<Edge> child(2);
    EdgeLabel label = 0;
    packRule(label, RULE_X);
    if (lvl == 1) {
        if (fun[start].getType() != VOID) {
            child[0].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA));
            child[0].setValue(fun[start]);
            
        } else {
            child[0].setEdgeHandle(makeTerminal(VOID, SpecialValue::POS_INF));
        }
        if(fun[end].getType() != VOID) {
            child[1].setEdgeHandle(makeTerminal(VOID, SpecialValue::OMEGA)); 
            child[1].setValue(fun[end]);
        } else {
            child[1].setEdgeHandle(makeTerminal(VOID, SpecialValue::POS_INF));
        }
        child[0].setRule(RULE_X); 
        child[1].setRule(RULE_X);
        return forest->reduceEdge(lvl, label, lvl, child);
    }
    child[0] = buildEvSetEdge(forest, lvl-1, fun, start, start+(1<<(lvl-1))-1);
    child[1] = buildEvSetEdge(forest, lvl-1, fun, start+(1<<(lvl-1)), end);
    return forest->reduceEdge(lvl, label, lvl, child);
}

Edge buildEvRelEdge(Forest* forest,
                    uint16_t lvl,
                    std::vector<Value>& fun,
                    int start, int end)
{
    Edge ans;
    // TBD
    return ans;
}