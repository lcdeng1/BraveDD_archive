#include "helpers.h"
#include "operations/operation.h"
#include <cstring>
#include <vector>
#include <algorithm>

namespace BRAVE_DD {

// Helper function to convert a minterm string to a binary assignment
std::vector<bool> mintermToAssignment(const char* minterm, uint32_t K) {
    std::vector<bool> assignment(K, false);
    for (uint32_t i = 0; i < K; i++) {
        assignment[i] = (minterm[i] == '1');
    }
    return assignment;
}

// Helper function to create a terminal node with the given outcome
Edge makeTerminalEdge(Forest* F, int outcome) {
    Edge edge;
    EdgeHandle handle;
    Value val;
    if (F->getSetting().getValType() == INT) {
        handle = makeTerminal(INT, outcome);
        val.setValue(&outcome, INT);
    } else if (F->getSetting().getValType() == FLOAT) {
        float fOutcome = static_cast<float>(outcome);
        handle = makeTerminal(FLOAT, fOutcome);
        val.setValue(&fOutcome, FLOAT);
    }
    packRule(handle, RULE_X);
    edge = Edge(handle, val);
    return edge;
}

// Helper function to create a variable node at the given level
Edge makeVariableEdge(Forest* F, uint16_t level) {
    std::vector<Edge> children(2);
    children[0] = makeTerminalEdge(F, 0);
    children[1] = makeTerminalEdge(F, 1);
    
    EdgeLabel root = 0;
    packRule(root, RULE_X);
    
    return F->reduceEdge(F->getSetting().getNumVars(), root, level, children);
}

} // namespace BRAVE_DD

// Implementation of union_minterm function
BRAVE_DD::Edge union_minterm(BRAVE_DD::Forest* F, BRAVE_DD::Edge* root, char* minterm, int outcome, uint32_t K) {
    if (!F || !root || !minterm) {
        return BRAVE_DD::Edge(); // Return empty edge on error
    }
    
    // If root is empty, create a new edge for this minterm
    if (root->getEdgeHandle() == 0) {
        *root = BRAVE_DD::makeTerminalEdge(F, outcome);
        return *root;
    }
    
    // Convert minterm to assignment
    std::vector<bool> assignment = BRAVE_DD::mintermToAssignment(minterm, K);
    
    // Start from the root
    BRAVE_DD::Edge current = *root;
    uint16_t currentLevel = current.getNodeLevel();
    
    // Traverse the BDD based on the assignment
    for (uint32_t i = 0; i < K; i++) {
        // If we've reached a terminal node, we need to create a new path
        if (currentLevel == 0) {
            // If the terminal has the same outcome, we're done
            BRAVE_DD::Value termVal = BRAVE_DD::getTerminalValue(current.getEdgeHandle());
            int terminalValue = 0;
            termVal.getValueTo(&terminalValue, BRAVE_DD::INT);
            if (termVal.getType() == BRAVE_DD::INT && terminalValue == outcome) {
                return *root;
            }
            
            // Otherwise, we need to create a new path
            // Find the highest level where the paths differ
            uint16_t diffLevel = 0;
            for (uint16_t j = 0; j < K; j++) {
                BRAVE_DD::Value currVal = BRAVE_DD::getTerminalValue(current.getEdgeHandle());
                int currValue = 0;
                currVal.getValueTo(&currValue, BRAVE_DD::INT);
                if (assignment[j] != (currVal.getType() == BRAVE_DD::INT && currValue == 1)) {
                    diffLevel = j + 1;
                    break;
                }
            }
            
            // Create a new variable node at the diff level
            BRAVE_DD::Edge newVar = BRAVE_DD::makeVariableEdge(F, diffLevel);
            
            // Set the appropriate child based on the assignment
            if (assignment[diffLevel - 1]) {
                newVar = BRAVE_DD::makeTerminalEdge(F, outcome);
            } else {
                newVar = *root;
            }
            
            // Update the root
            *root = newVar;
            return *root;
        }
        
        // If we're at a non-terminal node, follow the appropriate child
        if (assignment[currentLevel - 1]) {
            current = F->getChildEdge(currentLevel, current.getNodeHandle(), 1);
        } else {
            current = F->getChildEdge(currentLevel, current.getNodeHandle(), 0);
        }
        
        currentLevel = current.getNodeLevel();
    }
    
    // If we've reached a terminal node with the same outcome, we're done
    BRAVE_DD::Value termVal = BRAVE_DD::getTerminalValue(current.getEdgeHandle());
    int terminalValue = 0;
    termVal.getValueTo(&terminalValue, BRAVE_DD::INT);
    if (currentLevel == 0 && termVal.getType() == BRAVE_DD::INT && terminalValue == outcome) {
        return *root;
    }
    
    // Otherwise, we need to create a new path
    uint16_t diffLevel = 0;
    for (uint16_t j = 0; j < K; j++) {
        BRAVE_DD::Value currVal = BRAVE_DD::getTerminalValue(current.getEdgeHandle());
        int currValue = 0;
        currVal.getValueTo(&currValue, BRAVE_DD::INT);
        if (assignment[j] != (currVal.getType() == BRAVE_DD::INT && currValue == 1)) {
            diffLevel = j + 1;
            break;
        }
    }
    
    // Create a new variable node at the diff level
    BRAVE_DD::Edge newVar = BRAVE_DD::makeVariableEdge(F, diffLevel);
    
    // Set the appropriate child based on the assignment
    if (assignment[diffLevel - 1]) {
        newVar = BRAVE_DD::makeTerminalEdge(F, outcome);
    } else {
        newVar = *root;
    }
    
    // Update the root
    *root = newVar;
    return *root;
}

// Implementation of union_minterms_radixscan function
BRAVE_DD::Edge union_minterms_radixscan(BRAVE_DD::Forest* F, uint32_t K, BRAVE_DD::Edge* root, char** minterms, int outcome, unsigned int N) {
    if (!F || !root || !minterms || N == 0) {
        return BRAVE_DD::Edge(); // Return empty edge on error
    }
    
    // If root is empty, create a new edge for the first minterm
    if (root->getEdgeHandle() == 0) {
        *root = BRAVE_DD::makeTerminalEdge(F, outcome);
    }
    
    // Group minterms by their first bit
    std::vector<std::vector<char*>> groups(2);
    for (unsigned int i = 0; i < N; i++) {
        if (minterms[i][0] == '0') {
            groups[0].push_back(minterms[i] + 1); // Skip the first bit
        } else {
            groups[1].push_back(minterms[i] + 1); // Skip the first bit
        }
    }
    
    // Process each group recursively
    for (int i = 0; i < 2; i++) {
        if (!groups[i].empty()) {
            // Create a new edge for this group
            BRAVE_DD::Edge groupRoot;
            
            // Recursively process the group
            if (K > 1) {
                groupRoot = union_minterms_radixscan(F, K - 1, &groupRoot, groups[i].data(), outcome, groups[i].size());
            } else {
                // If K == 1, we've reached the end of the minterm
                groupRoot = BRAVE_DD::makeTerminalEdge(F, outcome);
            }
            
            // Create a new variable node at the current level
            BRAVE_DD::Edge newVar = BRAVE_DD::makeVariableEdge(F, K);
            
            // Set the appropriate child based on the group
            if (i == 0) {
                // For group 0, set the 0-child to the group root
                std::vector<BRAVE_DD::Edge> children(2);
                children[0] = groupRoot;
                children[1] = BRAVE_DD::makeTerminalEdge(F, 0);
                
                BRAVE_DD::EdgeLabel label = 0;
                BRAVE_DD::packRule(label, BRAVE_DD::RULE_X);
                
                newVar = F->reduceEdge(F->getSetting().getNumVars(), label, K, children);
            } else {
                // For group 1, set the 1-child to the group root
                std::vector<BRAVE_DD::Edge> children(2);
                children[0] = BRAVE_DD::makeTerminalEdge(F, 0);
                children[1] = groupRoot;
                
                BRAVE_DD::EdgeLabel label = 0;
                BRAVE_DD::packRule(label, BRAVE_DD::RULE_X);
                
                newVar = F->reduceEdge(F->getSetting().getNumVars(), label, K, children);
            }
            
            // Union the new variable with the root
            *root = F->reduceEdge(F->getSetting().getNumVars(), root->getEdgeHandle(), K, {*root, newVar});
        }
    }
    
    return *root;
}

// Implementation of getValue function
int getValue(const BRAVE_DD::Edge& edge) {
    BRAVE_DD::Value val = edge.getValue();
    int intVal = 0;
    val.getValueTo(&intVal, BRAVE_DD::INT);
    return intVal;
}
