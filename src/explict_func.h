#ifndef BRAVE_DD_EXPLICT_FUNC_H
#define BRAVE_DD_EXPLICT_FUNC_H

#include "defines.h"
#include <vector>

namespace BRAVE_DD {

/**
 * ExplictFunc class for handling explicit function representations 
 * with assignments (minterms) and their corresponding outcomes.
 */
class ExplictFunc {
public:
    // Constructor and destructor
    ExplictFunc() {}
    ~ExplictFunc() {}

    // Add an assignment (minterm) with its outcome value
    void addAssignment(const std::vector<bool>& minterm, const Value& outcome) {
        assignments.push_back(minterm);
        outcomes.push_back(outcome);
    }

    // Get the number of assignments
    int getNumAssignments() const {
        return static_cast<int>(assignments.size());
    }

    // Get the bit length of assignments
    int getNumBits() const {
        return assignments.empty() ? 0 : static_cast<int>(assignments[0].size());
    }

    // Get outcome for a specific assignment
    const Value& getOutcome(int index) const {
        return outcomes[index];
    }

    // Get the assignment (minterm) at the given index
    const std::vector<bool>& getAssignment(int idx) const {
        return assignments[idx];
    }

    // Access the full assignment and outcome vectors
    const std::vector<std::vector<bool>>& getAssignments() const {
        return assignments;
    }
    
    const std::vector<Value>& getOutcomes() const {
        return outcomes;
    }

    // Converts assignments to char** pattern strings for radix scan
    char** getAllAssignmentsAsCharArray() const {
        int num = getNumAssignments();
        int bits = getNumBits();

        if (num == 0 || bits == 0) return nullptr;

        char** result = new char*[num];
        for (int i = 0; i < num; ++i) {
            result[i] = new char[bits + 1];
            for (int j = 0; j < bits; ++j) {
                // Skip index 0 if using 1-indexed assignments
                int idx = (assignments[i].size() > bits) ? j+1 : j;
                result[i][j] = assignments[i][idx] ? '1' : '0';
            }
            result[i][bits] = '\0';
        }
        return result;
    }

    // Helper to free memory from getAllAssignmentsAsCharArray
    static void freeCharArray(char** array, int size) {
        if (!array) return;
        for (int i = 0; i < size; ++i) {
            delete[] array[i];
        }
        delete[] array;
    }

private:
    std::vector<std::vector<bool>> assignments;
    std::vector<Value> outcomes;
};

} // namespace BRAVE_DD

#endif // BRAVE_DD_EXPLICT_FUNC_H 