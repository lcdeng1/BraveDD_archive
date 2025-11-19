/**
 * @file Variable.cc
 * @brief Implementation of the Variable class for BDD-based DFA construction.
 * @details All methods are O(1) unless otherwise noted.
 */

#include "Variable.h"
#include <stdexcept> // For std::runtime_error

// Static member definitions
int Variable::maxIndex = 1; // O(1) initialization
std::vector<Variable*> Variable::registry = {nullptr}; // O(1) initialization

/**
 * @brief Default constructor.
 * @details Time Complexity: O(1)
 */
Variable::Variable() : myIndex(0) {}

/**
 * @brief Constructor for v_AlphBelongsToq_i type.
 * @details Time Complexity: O(1)
 */
Variable::Variable(int varType, int alphaT, int iT)
    : varTypeG(varType), alpha(alphaT), i(iT), myIndex(0) {}

/**
 * @brief Constructor for deltaq_iXIsq_j type.
 * @details Time Complexity: O(1)
 */
Variable::Variable(int varType, int iT, int jT, int xT)
    : varTypeG(varType), i(iT), j(jT), x(xT), myIndex(0) {}

/**
 * @brief Activates the variable and assigns a unique index.
 * @details Time Complexity: O(1)
 */
void Variable::activate() {
    if (myIndex > 0) return; // O(1) check
    myIndex = maxIndex++;    // O(1) assignment
    registry.push_back(this); // O(1) insertion
}

/**
 * @brief Initializes the registry and resets maxIndex.
 * @details Time Complexity: O(N) due to registry.clear()
 */
void Variable::init() {
    maxIndex = 1;             // O(1)
    registry.clear();         // O(N), where N is current registry size
    registry.push_back(nullptr); // O(1)
}

/**
 * @brief Retrieves a variable from the registry by index.
 * @details Time Complexity: O(1)
 */
Variable* Variable::getVariableIndex(int i) {
    return registry.at(i); // O(1)
}

/**
 * @brief Returns the index of this variable.
 * @details Time Complexity: O(1)
 */
int Variable::getMyIndex() const {
    return myIndex;
}

/**
 * @brief Returns the type of this variable.
 * @details Time Complexity: O(1)
 */
int Variable::getVarType() const {
    return varTypeG;
}

/**
 * @brief Returns the current maximum index.
 * @details Time Complexity: O(1)
 */
int Variable::getMaxIndex() {
    return maxIndex;
}

/**
 * @brief Returns the registry of all variables.
 * @details Time Complexity: O(1)
 */
const std::vector<Variable*>& Variable::getRegistry() {
    return registry;
}