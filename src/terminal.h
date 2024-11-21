#ifndef BRAVE_DD_TERMINAL_H
#define BRAVE_DD_TERMINAL_H

#include "defines.h"
#include "edge.h"

namespace BRAVE_DD {
    // ******************************************************************
    // *                                                                *
    // *                        Terminal Handle                         *
    // *                                                                *
    // ******************************************************************
    /**
     * EdgeHandle pointing to terminal nodes
     * Assuming terminal value can only be:
     *          INT, FLOAT, -∞, +∞, UNDEF, OMEGA.
     * header (9 bits): bit 63: 1: if terminal is float value;
     *                  bit 62: 1: if terminal is int value;
     *                  bit 61: 1: if terminal is special value;
     * 
     * In Node storage: informatin in target NodeHandle, ForestSetting tells the meaning
     * 
     */
    
    /**
     * @brief Get the terminal value for the given EdgeHandle.
     * Note: it would check if the given handle is terminal, 
     *       and exit if not.
     * 
     * @param handle        The given EdgeHandle.
     * @return Value     – Output termianl value wrapper.
     */
    static inline Value getTerminalValue(const EdgeHandle& handle) {
        if(unpackLevel(handle)>0) {
            std::cout << "[BRAVE_DD] ERROR!\t No value for nonterminal node!"<< std::endl;
            exit(0);
        }
        Value val(0);
        NodeHandle data = unpackNode(handle);
        if (handle & FLOAT_VALUE_FLAG_MASK) {
            // float value
            float value = *reinterpret_cast<float*>(&data);
            val.setValue(&value, FLOAT);
        } else if (handle & INT_VALUE_FLAG_MASK) {
            // int value
            int value = *reinterpret_cast<int*>(&data);
            val.setValue(&value, INT);
        } else if (handle & SPECIAL_VALUE_FLAG_MASK) {
            // special value
            SpecialValue value = *reinterpret_cast<SpecialValue*>(&data);
            val.setValue(&value, VOID);
        } else {
            // unknown value
            std::cout << "[BRAVE_DD] ERROR!\t Unknown value for terminal node!"<< std::endl;
            exit(0);
        }
        return val;
    }
    
    /**
     * @brief Make a plain EdgeHandle target to terminal node, by giving the terminal value and type.
     * 
     * @param type 
     * @param value 
     * @return EdgeHandle 
     */
    static inline EdgeHandle makeTerminal(const ValueType type, const void* value) {
        if (type == DOUBLE || type == LONG) {
            std::cout << "[BRAVE_DD] ERROR!\t Unsupported data type! It can only be INT/FLOAT."<< std::endl;
            exit(0);
        }
        EdgeHandle handle = 0;
        NodeHandle node = 0;
        if (type == INT) {
            handle |= INT_VALUE_FLAG_MASK;
            int target = *((int*) value);
            node = *reinterpret_cast<NodeHandle*>(&target);
        } else if (type == FLOAT) {
            handle |= FLOAT_VALUE_FLAG_MASK;
            float target = *((float*) value);
            node = *reinterpret_cast<NodeHandle*>(&target);
        } else {
            handle |= SPECIAL_VALUE_FLAG_MASK;
            SpecialValue target = *((SpecialValue*) value);
            node = *reinterpret_cast<NodeHandle*>(&target);
        }
        packTarget(handle, node);
        return handle;
    }
    template <typename T>
    static inline EdgeHandle makeTerminal(const ValueType type, const T& value) {
        // return makeTerminal(type, reinterpret_cast<void*>(const_cast<T*>(&value)));
        return makeTerminal(type, static_cast<const void*>(&value));
    }

}; // end of namespace

#endif