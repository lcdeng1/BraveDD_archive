#ifndef BRAVE_DD_COMPUTE_TABLE_H
#define BRAVE_DD_COMPUTE_TABLE_H

#include "../defines.h"
#include "../forest.h"
#include "../hash_stream.h"

namespace BRAVE_DD {
    class CacheEntry;
    class ComputeTable;
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      CacheEntry class                          *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::CacheEntry {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    CacheEntry() {
        lvl = 0;
        key = std::vector<Edge>(2);
        isInUse = 0;
    }
    CacheEntry(const Level level, const Edge& a) {
        lvl = level;
        key = std::vector<Edge>(1);
        key[0] = a;
        isInUse = 0;
    }
    CacheEntry(const Level level, const Edge& a, const Edge& b) {
        lvl = level;
        key = std::vector<Edge>(2);
        key[0] = a;
        key[1] = b;
        isInUse = 0;
    }
    CacheEntry(const Level level, const Edge& a, const Edge& b, const Edge& c, const Edge& d) {
        lvl = level;
        key = std::vector<Edge>(4);
        key[0] = a;
        key[1] = b;
        key[2] = c;
        key[3] = d;
        isInUse = 0;
    }

    inline void setResult(const Edge& r) {
        res = r;
        // only be in use when the result is set
        isInUse = 1;
    }
    inline void setResult(const long v) {
        Value value;
        value.setValue(v, LONG);
        res.setValue(value);
        isInUse = 1;
    }
    inline void setResult(const char v) {
        res.setEdgeHandle((EdgeHandle) v);
        isInUse = 1;
    }
    inline void setResult(const bool v) {
        res.setEdgeHandle((EdgeHandle) v);
        isInUse = 1;
    }

    inline uint64_t hash() const {
        hash_stream hs;
        hs.start();
        // push info
        hs.push(lvl);
        for (char i=0; i<(char)key.size(); i++) {
            // hs.push(key[i].getEdgeHandle());
            hs.push(key[i].getRule());
            hs.push(key[i].getComp());
            hs.push(key[i].getSwap(0));
            hs.push(key[i].getSwap(1));
            hs.push(key[i].getNodeLevel());
            hs.push(key[i].getNodeHandle());
        }
        // for edge valued, TBD
        return (uint64_t)hs.finish64();
    }

    // inline CacheEntry& operator=(const CacheEntry& e) {
    //     if (equals(e)) return *this;
    //     key = e.key;
    //     lvl = e.lvl;
    //     res = e.res;
    //     keySize = e.keySize;
    //     isInUse = e.isInUse;
    // }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    friend class ComputeTable;

    inline bool equals(const CacheEntry& e) const {
        if (lvl != e.lvl) return 0;
        for (char i=0; i<(char)key.size(); i++) {
            if (key[i] != e.key[i]) return 0;
        }
        return 1;
    }

    std::vector<Edge>   key;
    Edge                res;
    Level               lvl;
    bool                isInUse;
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                     ComputeTable class                         *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::ComputeTable {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    ComputeTable();
    ~ComputeTable();

    bool check(const Level lvl, const Edge& a, long& ans);
    bool check(const Level lvl, const Edge& a, Edge& ans);
    bool check(const Level lvl, const Edge& a, const Edge& b, Edge& ans);
    bool check(const Level lvl, const Edge& a, const Edge& b, char& ans);
    bool check(const Level lvl, const Edge& a, const Edge& b, bool& ans);
    bool check(const Level lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, Edge& ans);
    bool check(const Level lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, char& ans);
    bool check(const Level lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, bool& ans);

    void add(const Level lvl, const Edge& a, const long& ans);
    void add(const Level lvl, const Edge& a, const Edge& ans);
    void add(const Level lvl, const Edge& a, const Edge& b, const Edge& ans);
    void add(const Level lvl, const Edge& a, const Edge& b, const char& ans);
    void add(const Level lvl, const Edge& a, const Edge& b, const bool& ans);
    void add(const Level lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, const Edge& ans);
    void add(const Level lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, const char& ans);
    void add(const Level lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, const bool& ans);

    // role: 0 for ans; 1 for key0; 2 for key1
    void sweep(Forest* forest, int role);

    void reportStat(std::ostream& out, int format=0) const;

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /**
     * @brief This will enlarge the table and rehash the old data with the new size
     * 
     */
    void enlarge(uint64_t newSize);
    void sweepAndEnlarge(Forest* forest);

    friend class UnaryOperation;
    friend class BinaryOperation;
    friend class SaturationOperation;

    std::vector<CacheEntry>     table;
    uint64_t                    numEntries;
    uint64_t                    size;

    uint64_t                    countCalls;
    uint64_t                    countHits;
    uint64_t                    countOverwrite;
    size_t                      probingSteps;

};

#endif