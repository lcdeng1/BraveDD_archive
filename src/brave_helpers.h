#ifndef BRAVE_DD_BRAVE_HELPERS_H
#define BRAVE_DD_BRAVE_HELPERS_H

#include "forest.h"
#include "edge.h"
#include <vector>
#include <map>
#include <string>

namespace BRAVE_DD {

// Define ForestHandle type to match what's used in function.cc
typedef uint32_t ForestHandle;

// Declare the global forests vector
extern std::vector<Forest*> forests;

// Function to register a Forest and return its handle
ForestHandle registerForest(Forest* forest);

// Helper function to create a terminal node with the given outcome
Edge makeTerminalEdge(Forest* F, int outcome);

// Helper function to create a terminal zero edge
Edge createTerminalZero(Forest* F);

// Helper function to ensure a terminal edge has proper type flags
Edge ensureTerminalTypeFlags(Forest* F, Edge edge);

// Helper function to create a variable node at the given level
Edge makeVariableEdge(Forest* F, uint16_t level);

// Implementation for the union_minterm function
Edge union_minterm(Forest* F, Edge* root, char* assignment, int outcome, uint32_t K);

// Implementation for the radixscan-based minterm union function
Edge union_minterms_radixscan(Forest* F, uint32_t K, Edge* root, char** minterms, int outcome, unsigned int N);

// Helper functions (provide functions in a separate namespace to avoid conflict with terminal.h)
namespace brave_helpers {
    bool isTerminal(EdgeHandle edgeHnd);
    bool isTerminalSpecial(EdgeHandle edgeHnd);
    bool isTerminalOne(EdgeHandle edgeHnd);
    bool isTerminalZero(EdgeHandle edgeHnd);
}

// Use the brave_helpers namespace for these functions to avoid conflicts
using brave_helpers::isTerminal;
using brave_helpers::isTerminalSpecial;
using brave_helpers::isTerminalOne;
using brave_helpers::isTerminalZero;

uint32_t makeIntTerminalNode(uint32_t value);

// Helper function to convert vector<bool> to string
std::string vectorBoolToString(const std::vector<bool>& v);

// Implementation for makeTerminal function
EdgeHandle makeTerminal(ValueType type, uint32_t value);

// Forest type enumeration for type checking
enum class ForestType { 
    FBDD, 
    REXBDD, 
    QBDD, 
    ZBDD 
};

// Helper function to get the forest type
ForestType getForestType(ForestHandle forest);

// Helper function to create a variable node at the given level with the given children
EdgeHandle makeVariableEdge(Forest* F, uint32_t varIndex, EdgeHandle zeroChild, EdgeHandle oneChild);

} // namespace BRAVE_DD

#endif // BRAVE_DD_BRAVE_HELPERS_H 