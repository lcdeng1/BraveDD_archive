#include "brave_helpers.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <vector>
#include "terminal.h"
#include "defines.h"
#include "settings/reductions.h"
#include "settings/edge_flags.h"
#include "operations/operation.h"
#include "forest.h"

#include "error.h"

namespace BRAVE_DD
{   
    std::vector<Forest*> forests;

    uint32_t registerForest(Forest *f)
    {
        if (!f) {
            throw error(ErrCode::MISCELLANEOUS, __FILE__, __LINE__);
            return 0;
        }
        
        // Check if forest is already registered
        for (size_t i = 0; i < forests.size(); i++) {
            if (forests[i] == f) {
                return i;
            }
        }
        
        // Add forest to the list
        forests.push_back(f);
        return forests.size() - 1;
    }

    uint32_t makeIntTerminalNode(uint32_t value) {
        uint32_t nodeHandle = 0;
        // Use the proper makeTerminal function to ensure type flags are set
        nodeHandle = makeTerminal(INT, value);
        return nodeHandle;
    }

    namespace brave_helpers {
        bool isTerminal(EdgeHandle edgeHnd) {
            if (unpackLevel(edgeHnd) == 0) {
                // Additional check for type flags to ensure it's a valid terminal
                return (edgeHnd & INT_VALUE_FLAG_MASK) || 
                       (edgeHnd & FLOAT_VALUE_FLAG_MASK) || 
                       (edgeHnd & SPECIAL_VALUE_FLAG_MASK);
            }
            return false;
        }

        bool isTerminalSpecial(EdgeHandle edgeHnd) {
            if (unpackLevel(edgeHnd) == 0 && (edgeHnd & SPECIAL_VALUE_FLAG_MASK)) {
                return true;
            }
            return false;
        }

        bool isTerminalOne(EdgeHandle edgeHnd) {
            if (unpackLevel(edgeHnd) == 0 && unpackTarget(edgeHnd)==1 && 
                ((edgeHnd & INT_VALUE_FLAG_MASK) || (edgeHnd & FLOAT_VALUE_FLAG_MASK))) {
                return true;
            }
            return false;
        }

        bool isTerminalZero(EdgeHandle edgeHnd) {
            if (unpackLevel(edgeHnd) == 0 && unpackTarget(edgeHnd)==0 && 
                ((edgeHnd & INT_VALUE_FLAG_MASK) || (edgeHnd & FLOAT_VALUE_FLAG_MASK))) {
                return true;
            }
            return false;
        }
    }

    // Implementation for makeTerminal function
    EdgeHandle makeTerminal(ValueType type, uint32_t value) {
        EdgeHandle handle = 0;
        
        // Set the value
        packTarget(handle, value);
        
        // Set the level to 0 to indicate terminal
        packLevel(handle, 0);
        
        // Ensure we always set a type flag - default to INT if type is invalid
        if (type == INT) {
            handle |= INT_VALUE_FLAG_MASK;
        } else if (type == FLOAT) {
            handle |= FLOAT_VALUE_FLAG_MASK;
        } else if (type == VOID) {
            handle |= SPECIAL_VALUE_FLAG_MASK;
        } else {
            // For safety, default to INT type if unspecified
            handle |= INT_VALUE_FLAG_MASK;
        }
        
        // Add default rule
        packRule(handle, RULE_X);
        
        return handle;
    }

    // Implementation for makeTerminalEdge, ensuring proper terminal value handling
    Edge makeTerminalEdge(Forest* F, int outcome) {
        Edge edge;
        if (!F) {
            return edge;  // Return empty edge if forest is invalid
        }
        
        // Create the terminal edge with proper type flags
        EdgeHandle handle = 0;
        Value val;
        
        // Use the appropriate value type based on forest settings
        if (F->getSetting().getValType() == FLOAT) {
            float fOutcome = static_cast<float>(outcome);
            handle = makeTerminal(FLOAT, *reinterpret_cast<uint32_t*>(&fOutcome));
            
            // Set the value for the edge
            val.setValue(fOutcome, FLOAT);
        } else {
            // Default to INT type
            handle = makeTerminal(INT, outcome);
            
            // Set the value for the edge
            val.setValue(outcome, INT);
        }
        
        // Add rule information if not already set
        if (unpackRule(handle) == 0) {
            packRule(handle, RULE_X);
        }
        
        // Set the handle and value in the edge
        edge.setEdgeHandle(handle);
        edge.setValue(val);
        
        // Ensure terminal has type flags
        return ensureTerminalTypeFlags(F, edge);
    }

    // Implementation for one minterm
    Edge union_minterm(Forest* F, Edge* root, char* minterm, int outcome, uint32_t K) {
        if (F == nullptr) {
            return Edge();
        }
        
        if (minterm == nullptr) {
            return (root != nullptr) ? *root : Edge();
        }
        
        // Debug output
        #ifdef BRAVE_DD_DEBUG
        std::cout << "[BRAVE_DD] DEBUG union_minterm: Processing minterm '" << minterm 
                  << "', length=" << strlen(minterm) << ", K=" << K << std::endl;
        #endif
        
        // Validate minterm string to ensure it only contains 0s and 1s
        size_t mintermLength = strlen(minterm);
        for (size_t i = 0; i < mintermLength; i++) {
            if (minterm[i] != '0' && minterm[i] != '1') {
                std::cerr << "[BRAVE_DD] ERROR! Invalid character in minterm: '" << minterm[i] 
                          << "' at position " << i << std::endl;
                return (root != nullptr) ? *root : Edge();
            }
        }
        
        // Validate minterm length
        if (mintermLength < K) {
            std::cerr << "[BRAVE_DD] ERROR! Minterm too short: length=" << mintermLength 
                      << ", expected at least " << K << std::endl;
            return (root != nullptr) ? *root : Edge();
        }
        
        if (K == 0) {
            // Terminal case - we've processed all bits
            Edge terminalEdge = makeTerminalEdge(F, outcome);
            
            // Ensure proper type flags
            terminalEdge = ensureTerminalTypeFlags(F, terminalEdge);
            
            // If we have a root edge, merge using union operation
            if (root != nullptr && root->getEdgeHandle() != 0) {
                try {
                    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_UNION, F, F, F);
                    if (!bop) {
                        bop = BOPs.add(new BinaryOperation(BinaryOperationType::BOP_UNION, F, F, F));
                    }
                    
                    Func f1(F, *root);
                    Func f2(F, terminalEdge);
                    Func result(F);
                    
                    bop->compute(f1, f2, result);
                    Edge resultEdge = result.getEdge();
                    
                    // Ensure the result has proper type flags
                    return ensureTerminalTypeFlags(F, resultEdge);
                } catch (const std::exception& e) {
                    std::cerr << "[BRAVE_DD] ERROR! Exception in union_minterm union operation: " << e.what() << std::endl;
                    return terminalEdge;
                }
            }
            
            return terminalEdge;
        }
        
        // We're at a decision level (not a terminal) - parse the current bit
        char currentBit = minterm[0];
        
        // Create child edges
        Edge zeroChild, oneChild;
        
        // Check if we have a root edge to start with
        if (root != nullptr && root->getEdgeHandle() != 0) {
            uint16_t rootLevel = root->getNodeLevel();
            if (rootLevel == 0) {
                // Root is a terminal - create a variable node with proper children
                if (brave_helpers::isTerminalZero(root->getEdgeHandle())) {
                    // Root is 0 - create a node with zero and terminal children
                    zeroChild = *root;  // zero child is the terminal 0
                    
                    // one child depends on the current bit
                    if (currentBit == '1') {
                        // This path should lead to a terminal with the outcome
                        oneChild = union_minterm(F, nullptr, minterm + 1, outcome, K - 1);
                    } else {
                        // This path should lead to a terminal 0
                        oneChild = createTerminalZero(F);
                    }
                } else {
                    // Root is not 0 - likely 1 or another value - use it for both children
                    zeroChild = *root;
                    oneChild = *root;
                    
                    // Update the appropriate child based on current bit
                    if (currentBit == '0') {
                        zeroChild = union_minterm(F, &zeroChild, minterm + 1, outcome, K - 1);
                    } else {
                        oneChild = union_minterm(F, &oneChild, minterm + 1, outcome, K - 1);
                    }
                }
            } else if (rootLevel == K) {
                // Root is at the same level - get its children
                NodeHandle nodeHandle = root->getNodeHandle();
                zeroChild = F->getChildEdge(rootLevel, nodeHandle, 0);
                oneChild = F->getChildEdge(rootLevel, nodeHandle, 1);
                
                // Update the appropriate child based on current bit
                if (currentBit == '0') {
                    zeroChild = union_minterm(F, &zeroChild, minterm + 1, outcome, K - 1);
                } else {
                    oneChild = union_minterm(F, &oneChild, minterm + 1, outcome, K - 1);
                }
            } else {
                // Root is at a different level - use it for both children initially
                zeroChild = *root;
                oneChild = *root;
                
                // Update the appropriate child based on current bit
                if (currentBit == '0') {
                    zeroChild = union_minterm(F, &zeroChild, minterm + 1, outcome, K - 1);
                } else {
                    oneChild = union_minterm(F, &oneChild, minterm + 1, outcome, K - 1);
                }
            }
        } else {
            // No root edge - create new children
            // Default both children to terminal 0
            zeroChild = createTerminalZero(F);
            oneChild = createTerminalZero(F);
            
            // Update the appropriate child based on current bit
            if (currentBit == '0') {
                zeroChild = union_minterm(F, nullptr, minterm + 1, outcome, K - 1);
            } else {
                oneChild = union_minterm(F, nullptr, minterm + 1, outcome, K - 1);
            }
        }
        
        // Create a new node with the updated children
        std::vector<Edge> children = {zeroChild, oneChild};
        EdgeLabel label = 0;
        packRule(label, RULE_X);
        
        try {
            Edge resultEdge = F->reduceEdge(F->getSetting().getNumVars(), label, K, children);
            
            // Ensure terminal nodes have proper type flags
            return ensureTerminalTypeFlags(F, resultEdge);
        } catch (const std::exception& e) {
            std::cerr << "[BRAVE_DD] ERROR! Exception in reduceEdge: " << e.what() << std::endl;
            
            // On error, try to return a meaningful edge
            if (currentBit == '0' && zeroChild.getEdgeHandle() != 0) {
                return zeroChild;
            } else if (currentBit == '1' && oneChild.getEdgeHandle() != 0) {
                return oneChild;
            } else if (root != nullptr && root->getEdgeHandle() != 0) {
                return *root;
            } else {
                return Edge();
            }
        }
    }

    // Helper function to ensure a terminal edge has proper type flags
    Edge ensureTerminalTypeFlags(Forest* F, Edge edge) {
        if (!F) {
            return edge; // Can't fix without a forest
        }
        
        EdgeHandle handle = edge.getEdgeHandle();
        
        // Only process terminal nodes (level 0)
        if (unpackLevel(handle) == 0) {
            // Check if any type flag is already set
            bool hasTypeFlag = (handle & INT_VALUE_FLAG_MASK) || 
                              (handle & FLOAT_VALUE_FLAG_MASK) ||
                              (handle & SPECIAL_VALUE_FLAG_MASK);
                              
            if (!hasTypeFlag) {
                // No type flags - we need to add them
                NodeHandle targetValue = unpackTarget(handle);
                
                // Save other flags that should be preserved
                bool isComplement = edge.getComp();
                bool isSwap0 = edge.getSwap(0);
                bool isSwap1 = edge.getSwap(1);
                ReductionRule rule = unpackRule(handle);
                
                // Create a completely new handle with correct type flag
                handle = 0;
                packLevel(handle, 0);  // Set level to 0 (terminal)
                packTarget(handle, targetValue);  // Set target value
                packRule(handle, rule);  // Restore rule
                
                // Set appropriate type flag based on forest's value type
                ValueType forestValType = F->getSetting().getValType();
                Value val;
                
                if (forestValType == FLOAT) {
                    handle |= FLOAT_VALUE_FLAG_MASK;
                    
                    // Update value in edge object too
                    float floatVal = static_cast<float>(targetValue);
                    val.setValue(floatVal, FLOAT);
                } else {
                    // Default to INT type
                    handle |= INT_VALUE_FLAG_MASK;
                    
                    // Update value in edge object too
                    val.setValue(static_cast<int>(targetValue), INT);
                }
                
                // Create a new edge with the fixed handle and value
                Edge fixedEdge;
                fixedEdge.setEdgeHandle(handle);
                fixedEdge.setValue(val);
                
                // Restore the flags using the proper Edge methods
                if (isComplement) fixedEdge.setComp(true);
                if (isSwap0) fixedEdge.setSwap(0, true);
                if (isSwap1) fixedEdge.setSwap(1, true);
                
                return fixedEdge;
            }
        }
        
        return edge;
    }

    // Implementation for the radixscan-based minterm union function using a radix-like approach
    Edge union_minterms_radixscan(Forest* F, uint32_t K, Edge* root, char** minterms, int outcome, unsigned int N) {
        // Debug output for tracking function calls
        #ifdef BRAVE_DD_DEBUG
        std::cout << "[BRAVE_DD] DEBUG union_minterms_radixscan: K=" << K << ", N=" << N << std::endl;
        for (unsigned int i = 0; i < N; i++) {
            std::cout << "  minterm[" << i << "]: " << minterms[i] << std::endl;
        }
        #endif
        
        // 1. Handle basic edge cases
        if (F == nullptr) {
            return Edge();
        }
        
        if (N == 0) {
            // No assignments - return the original function or an empty edge
            return (root != nullptr) ? *root : Edge();
        }
        
        // 2. Terminal case - we've processed all bits
        if (K == 0) {
            // Create terminal node with the outcome value
            if (outcome == 0) {
                return createTerminalZero(F);
            } else {
                // Use makeTerminalEdge to ensure proper type flags
                Edge terminal = makeTerminalEdge(F, outcome);
                return ensureTerminalTypeFlags(F, terminal);
            }
        }
        
        // 3. Handle single assignment with simple function
        if (N == 1) {
            Edge result;
            if (root != nullptr && root->getEdgeHandle() != 0) {
                // If we have a root, start with it and apply the assignment
                result = *root;
            }
            
            // Add the minterm to the function
            result = union_minterm(F, &result, minterms[0], outcome, K);
            
            // Ensure terminal nodes have proper type flags
            return ensureTerminalTypeFlags(F, result);
        }
        
        // 4. Partition assignments based on first bit (classic radix scan approach)
        std::vector<char*> zeroBitMinterms;
        std::vector<char*> oneBitMinterms;
        
        // Split assignments by first bit
        for (unsigned int i = 0; i < N; i++) {
            if (minterms[i][0] == '0') {
                zeroBitMinterms.push_back(minterms[i]);
            } else {
                oneBitMinterms.push_back(minterms[i]);
            }
        }
        
        #ifdef BRAVE_DD_DEBUG
        std::cout << "  Split: " << zeroBitMinterms.size() << " '0' minterms, " 
                  << oneBitMinterms.size() << " '1' minterms" << std::endl;
        #endif
        
        // 5. Process each partition recursively
        Edge zeroEdge, oneEdge;
        
        // Get current root's children if it's a non-terminal at the current level
        Edge rootZeroChild, rootOneChild;
        bool hasRootChildren = false;
        
        if (root != nullptr && root->getEdgeHandle() != 0) {
            uint16_t rootLevel = root->getNodeLevel();
            if (rootLevel == K) {
                // Extract the children from current root
                NodeHandle nodeHandle = root->getNodeHandle();
                rootZeroChild = F->getChildEdge(rootLevel, nodeHandle, 0);
                rootOneChild = F->getChildEdge(rootLevel, nodeHandle, 1);
                hasRootChildren = true;
                
                #ifdef BRAVE_DD_DEBUG
                std::cout << "  Using root's children at level " << K << std::endl;
                #endif
            } else if (rootLevel == 0) {
                // Root is a terminal node - check if it's already a terminal 1
                if (brave_helpers::isTerminalOne(root->getEdgeHandle())) {
                    // If it's a terminal 1, we're done - return it
                    return *root;
                }
                // If it's a terminal 0, we'll just create a new BDD
                #ifdef BRAVE_DD_DEBUG
                std::cout << "  Root is a terminal node, building new BDD" << std::endl;
                #endif
            } else {
                // Root level is different - we'll need to do a proper union later
                #ifdef BRAVE_DD_DEBUG
                std::cout << "  Root is at different level: " << rootLevel << ", current K: " << K << std::endl;
                #endif
            }
        }
        
        // Process the '0' path
        if (!zeroBitMinterms.empty()) {
            // Create shifted minterms (skip first bit) for the '0' branch
            std::vector<char*> shiftedZeros;
            for (char* minterm : zeroBitMinterms) {
                shiftedZeros.push_back(minterm + 1);
            }
            
            // Use the 0-child of the root if available
            Edge* zeroRoot = hasRootChildren ? &rootZeroChild : nullptr;
            
            // Process zero-branch minterms recursively
            zeroEdge = union_minterms_radixscan(
                F, K-1, zeroRoot,
                shiftedZeros.data(), 
                outcome, 
                static_cast<unsigned int>(zeroBitMinterms.size())
            );
        } else if (hasRootChildren) {
            // No assignments in this partition - use the root's 0-child
            zeroEdge = rootZeroChild;
        } else {
            // No root child and no assignments - use terminal 0
            zeroEdge = createTerminalZero(F);
        }
        
        // Process the '1' path
        if (!oneBitMinterms.empty()) {
            // Create shifted minterms (skip first bit) for the '1' branch
            std::vector<char*> shiftedOnes;
            for (char* minterm : oneBitMinterms) {
                shiftedOnes.push_back(minterm + 1);
            }
            
            // Use the 1-child of the root if available
            Edge* oneRoot = hasRootChildren ? &rootOneChild : nullptr;
            
            // Process one-branch minterms recursively
            oneEdge = union_minterms_radixscan(
                F, K-1, oneRoot,
                shiftedOnes.data(), 
                outcome, 
                static_cast<unsigned int>(oneBitMinterms.size())
            );
        } else if (hasRootChildren) {
            // No assignments in this partition - use the root's 1-child
            oneEdge = rootOneChild;
        } else {
            // No root child and no assignments - use terminal 0
            oneEdge = createTerminalZero(F);
        }
        
        // 6. Create a node at current level K with the two child edges
        std::vector<Edge> children = {zeroEdge, oneEdge};
        EdgeLabel label = 0;
        packRule(label, RULE_X);
        
        // 7. Create the node via forest's reduce operation
        Edge resultEdge = F->reduceEdge(F->getSetting().getNumVars(), label, K, children);
        
        // Ensure type flags on the result
        resultEdge = ensureTerminalTypeFlags(F, resultEdge);
        
        // 8. If we have a root edge at a different level, merge with it using union operation
        if (root != nullptr && root->getEdgeHandle() != 0 && !hasRootChildren) {
            uint16_t rootLevel = root->getNodeLevel();
            if (rootLevel != K) {
                // We need to merge using the BOP_UNION operation
                try {
                    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_UNION, F, F, F);
                    if (!bop) {
                        bop = BOPs.add(new BinaryOperation(BinaryOperationType::BOP_UNION, F, F, F));
                    }
                    
                    Func f1(F, *root);
                    Func f2(F, resultEdge);
                    Func result(F);
                    
                    bop->compute(f1, f2, result);
                    resultEdge = result.getEdge();
                    
                    #ifdef BRAVE_DD_DEBUG
                    std::cout << "  Merged with root at level " << rootLevel << std::endl;
                    #endif
                } catch (const std::exception& e) {
                    // On error, log but continue with the current result
                    std::cerr << "[BRAVE_DD] ERROR in union merge: " << e.what() << std::endl;
                }
            }
        }
        
        // Final safety check for terminal type flags
        return ensureTerminalTypeFlags(F, resultEdge);
    }
    
    // Helper function to create a properly formatted terminal zero edge
    Edge createTerminalZero(Forest* F) {
        if (!F) {
            // Safety check - return empty edge if forest is invalid
            return Edge();
        }
        
        // Use the makeTerminalEdge function to ensure proper type flags
        Edge edge = makeTerminalEdge(F, 0);
        
        // Double-check that it has type flags
        return ensureTerminalTypeFlags(F, edge);
    }

    // Convert a vector<bool> to a string representation
    std::string vectorBoolToString(const std::vector<bool>& v) {
        std::string result;
        for (bool b : v) {
            result += b ? '1' : '0';
        }
        return result;
    }

    // Implementation of getForestType
    ForestType getForestType(ForestHandle forest) {
        if (forest >= forests.size() || forests[forest] == nullptr) {
            std::cerr << "[BRAVE_DD] ERROR! getForestType: Invalid forest handle" << std::endl;
            return ForestType::REXBDD; // Default to RexBDD
        }
        
        const std::string& name = forests[forest]->getSetting().getName();
        if (name == "RexBDD") {
            return ForestType::REXBDD;
        } else if (name == "FBDD") {
            return ForestType::FBDD;
        } else if (name == "QBDD") {
            return ForestType::QBDD;
        } else if (name == "ZBDD") {
            return ForestType::ZBDD;
        } else {
            // Default to RexBDD for unrecognized forest types
            return ForestType::REXBDD;
        }
    }

    // Implementation for makeVariableEdge
    EdgeHandle makeVariableEdge(Forest* F, uint32_t varIndex, EdgeHandle zeroChild, EdgeHandle oneChild) {
        if (F == nullptr) {
            #ifdef BRAVE_DD_DEBUG
            std::cerr << "ERROR: Attempted to create variable edge with nullptr forest" << std::endl;
            #endif
            return 0;
        }
        
        #ifdef BRAVE_DD_DEBUG
        std::cout << "Creating variable edge with index " << varIndex 
                << ", zeroChild=" << zeroChild 
                << ", oneChild=" << oneChild << std::endl;
        #endif
        
        // Create a variable node with the given index and children
        std::vector<Edge> children(2);
        
        // Set up child edges
        Edge zeroEdge, oneEdge;
        zeroEdge.setEdgeHandle(zeroChild);
        oneEdge.setEdgeHandle(oneChild);
        
        children[0] = zeroEdge;
        children[1] = oneEdge;
        
        // Create a node at the appropriate level
        uint16_t nodeLevel = varIndex + 1;  // Convert to 1-indexed
        
        // Set the edge rule
        EdgeLabel label = 0;
        packRule(label, RULE_X);
        
        // Create the node and return its handle
        Edge result = F->reduceEdge(F->getSetting().getNumVars(), label, nodeLevel, children);
        return result.getEdgeHandle();
    }
} // namespace BRAVE_DD 