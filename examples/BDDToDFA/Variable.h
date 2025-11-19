#ifndef VARIABLE_H
#define VARIABLE_H

#include <vector>

/**
 * @brief Represents a symbolic variable used in BDD-based DFA construction.
 * 
 * Maintains a registry of all created variables and assigns unique indices.
 * 
 * Time Complexity Notes:
 * - Constructor: O(1)
 * - activate(): O(1)
 * - init(): O(1)
 * - getvariableIndex(): O(1)
 * - getMyIndex(), getVarType(), getMaxIndex(): O(1)
 * - getRegistry(): O(1)
 */
class Variable {
private:
    /// Static member to track the maximum index assigned (shared across all instances)
    static int maxIndex;

    /// Instance-specific index value
    int myIndex;

    /// Registry of all variables
    static std::vector<Variable*> registry;

public:
    /// Variable type: v_AlphBelongsToq_i = 0, deltaq_iXIsq_j = 1, belongsAndDelta = 2
    int varTypeG;

    /// Alphabet index
    int alpha;

    /// State i
    int i;

    /// State j
    int j;

    /// Transition index x
    int x;

    /**
     * @brief Default constructor.
     * @details Time Complexity: O(1)
     */
    Variable();

    /**
     * @brief Constructor for v_AlphBelongsToq_i type.
     * @details Time Complexity: O(1)
     */
    Variable(int varType, int alphaT, int iT);

    /**
     * @brief Constructor for deltaq_iXIsq_j type.
     * @details Time Complexity: O(1)
     */
    Variable(int varType, int iT, int jT, int xT);

    /**
     * @brief Activates the variable and assigns a unique index.
     * @details Time Complexity: O(1)
     */
    void activate();

    /**
     * @brief Initializes the registry and resets maxIndex.
     * @details Time Complexity: O(1)
     */
    static void init();

    /**
     * @brief Retrieves a variable from the registry by index.
     * @details Time Complexity: O(1)
     */
    inline static Variable* getVariableIndex(int i);

    /**
     * @brief Returns the index of this variable.
     * @details Time Complexity: O(1)
     */
    int getMyIndex() const;

    /**
     * @brief Returns the type of this variable.
     * @details Time Complexity: O(1)
     */
    int getVarType() const;

    /**
     * @brief Returns the current maximum index.
     * @details Time Complexity: O(1)
     */
    static int getMaxIndex();

    /**
     * @brief Returns the registry of all variables.
     * @details Time Complexity: O(1)
     */
    static const std::vector<Variable*>& getRegistry();
};

#endif // VARIABLE_H