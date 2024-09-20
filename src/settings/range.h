#ifndef BRAVE_DD_RANGE_H
#define BRAVE_DD_RANGE_H

namespace BRAVE_DD {
    ///  Range type
    enum RangeType {
        BOOLEAN,            // {0, 1}
        FINITE,             // {0, 1, 2, ..., N-1}, N is determined in maxRange
        NNINTEGER,          // {0, 1, 2, ...}
        INTEGER,            // {..., -2, -1, 0, 1, 2, ...}
        NNREAL,             // [0, +∞)
        REAL                // (-∞, +∞)
    };
    /// Value type
    //      Note: used only for EVBDDs different value range and precision
    enum ValueType{
        VOID,               //  Locally for ForestSetting, without values on edges; for EdgeValue, special value
        INT,                //  Integer
        // ^ For finite range, only logN bits needed.
        LONG,               //  Long
        FLOAT,              //  Float
        DOUBLE              //  Double
    };
    /// Range type and range size if FINITE*
    class Range;
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                           Range class                          *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::Range {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
        Range(const RangeType range,
              const ValueType val);
        Range(unsigned long size);
        ~Range();
        //******************************************
        //  Getters
        //******************************************
        inline RangeType getRangeType() const {return rangeType;}
        inline ValueType getValType() const {return valueType;}
        inline unsigned long getMaxRange() const {return maxRange;}
        inline bool hasNegInf() const {return negInf;}
        inline bool hasPosInf() const {return posInf;}
        inline bool hasUnDef() const {return unDef;}
        //******************************************
        //  Setters
        //******************************************
        inline void setRangeType(const RangeType type) {rangeType = type;}
        inline void setValType(const ValueType type) {valueType = type;}
        inline void setMaxRange(const unsigned long max) {maxRange = max;}
        inline void setNedInf(const bool nif) {negInf = nif;}
        inline void setPosInf(const bool pif) {posInf = pif;}
        inline void setUnDef(const bool udf) {unDef = udf;}

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
        RangeType       rangeType;
        ValueType       valueType;
        unsigned long   maxRange;       // the max range limit N if type is FINITE
        bool            negInf;         // 1 if -∞ is allowed
        bool            posInf;         // 1 if +∞ is allowed
        bool            unDef;          // 1 if UnDef is allowed
};


#endif