#ifndef BRAVE_DD_TERMINAL_H
#define BRAVE_DD_TERMINAL_H

#include "defines.h"
#include "edge.h"

namespace BRAVE_DD {
    // ******************************************************************
    // *                                                                *
    // *                     Terminal NodeHandle                        *
    // *                                                                *
    // ******************************************************************
    /**
     * @brief Check if the given EdgeHandle is target to terminal.
     * Note: terminal nodes are not stored, they are represented 
     *       by a special NodeHandle value.
     * 
     * @param handle        The given EdgeHandle.
     * @return true         – If it's terminal.
     * @return false        – If not.
     */
    static inline bool isTerminal(const EdgeHandle handle) {
        return unpackLevel(handle) == 0;
    }
    /**
     * @brief Get the terminal value for the given EdgeHandle.
     * Note: it would check if the given handle is terminal, 
     *       and return -1 if not.
     * 
     * @param handle        The given EdgeHandle.
     * @return EdgeValue     – Output termianl value wrapper.
     */
    static inline EdgeValue getTerminalValue(const EdgeHandle handle) {
        // TBD
        EdgeValue val(0);
        return val;
    }
    /**
     * @brief Make the special NodeHandle for terminal Ω. 
     * This is usually used for edge valued BDDs.
     * 
     * @return NodeHandle   – Output terminal node handle.
     */
    static inline NodeHandle makeTerminal() {
        // TBD 
        return 0;
    }
    /**
     * @brief Make the special NodeHandle for termianl with 
     * a given value.
     * 
     * @param val           The given terminal value.
     * @return NodeHandle   – Output terminal node handle.
     */
    static inline NodeHandle makeTerminal(EdgeValue val) {
        // TBD 
        return 0;
    }

}; // end of namespace

#endif