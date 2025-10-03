#include "compute_table.h"

// #define BRAVE_DD_CACHE_TRACE

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                     ComputeTable methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************

ComputeTable::ComputeTable()
{
    numEntries = 0;
    size = 0x01<<18;
    table = std::vector<CacheEntry>(size, CacheEntry());
    probingSteps = 2;
    countCalls = 0;
    countHits = 0;
    countOverwrite = 0;
}
ComputeTable::~ComputeTable()
{
    table.clear();
    std::vector<CacheEntry>().swap(table);
}

bool ComputeTable::check(const uint16_t lvl, const Edge& a, long& ans)
{
    countCalls++;
    CacheEntry entry(lvl, a);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
    /* probing the entries*/
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (table[probId].isInUse) {
            /* Valid entry, then check if match */
            if ((table[probId].lvl == lvl) && (table[probId].key.size() == 1) && (table[probId].key[0] == a)) {
                countHits++;
                table[probId].res.getValue().getValueTo(&ans, LONG);
                return 1;
            }
        }
    }
    
    /* Not cached */
    return 0;
}

bool ComputeTable::check(const uint16_t lvl, const Edge& a, Edge& ans)
{
    countCalls++;
    CacheEntry entry(lvl, a);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
    /* probing the entries*/
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (table[probId].isInUse) {
            /* Valid entry, then check if match */
            if ((table[probId].lvl == lvl) && (table[probId].key.size() == 1) && (table[probId].key[0] == a)) {
                countHits++;
                ans = table[probId].res;
                return 1;
            }
        }
    }
    /* Not cached */
    return 0;
}

bool ComputeTable::check(const uint16_t lvl, const Edge& a, const Edge& b, Edge& ans)
{
    countCalls++;
    CacheEntry entry(lvl, a, b);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "checking in cache, id = " << id << "; size = " << size << std::endl;
#endif
    /* probing the entries*/
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (table[probId].isInUse) {
            /* Valid entry, then check if match */
            if ((table[probId].lvl == lvl) && (table[probId].key.size() == 2)
                && (table[probId].key[0] == a) && (table[probId].key[1] == b)) {
                countHits++;
                ans = table[probId].res;
                return 1;
            }
        }
    }
    /* Not cached */
    return 0;
}

bool ComputeTable::check(const uint16_t lvl, const Edge& a, const Edge& b, char& ans)
{
    countCalls++;
    CacheEntry entry(lvl, a, b);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "checking in cache, id = " << id << "; size = " << size << std::endl;
#endif
    /* probing the entries*/
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (table[probId].isInUse) {
            /* Valid entry, then check if match */
            if ((table[probId].lvl == lvl) && (table[probId].key.size() == 2)
                && (table[probId].key[0] == a) && (table[probId].key[1] == b)) {
                countHits++;
                ans = (char)table[probId].res.getEdgeHandle();
                return 1;
            }
        }
    }
    /* Not cached */
    return 0;
}

bool ComputeTable::check(const uint16_t lvl, const Edge& a, const Edge& b, bool& ans)
{
    countCalls++;
    CacheEntry entry(lvl, a, b);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "checking in cache, id = " << id << "; size = " << size << std::endl;
#endif
    /* probing the entries*/
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (table[probId].isInUse) {
            /* Valid entry, then check if match */
            if ((table[probId].lvl == lvl) && (table[probId].key.size() == 2)
                && (table[probId].key[0] == a) && (table[probId].key[1] == b)) {
                countHits++;
                ans = (bool)table[probId].res.getEdgeHandle();
                return 1;
            }
        }
    }
    /* Not cached */
    return 0;
}

bool ComputeTable::check(const uint16_t lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, Edge& ans)
{
    countCalls++;
    CacheEntry entry(lvl, a, b, c, d);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "checking in cache, id = " << id << "; size = " << size << std::endl;
#endif
    /* probing the entries*/
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (table[probId].isInUse) {
            /* Valid entry, then check if match */
            if ((table[probId].lvl == lvl) && (table[probId].key.size() == 4)
                && (table[probId].key[0] == a) && (table[probId].key[1] == b) && (table[probId].key[2] == c) && (table[probId].key[3] == d)) {
                countHits++;
                ans = table[probId].res;
                return 1;
            }
        }
    }
    /* Not cached */
    return 0;
}

bool ComputeTable::check(const uint16_t lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, char& ans)
{
    countCalls++;
    CacheEntry entry(lvl, a, b, c, d);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "checking in cache, id = " << id << "; size = " << size << std::endl;
#endif
    /* probing the entries*/
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (table[probId].isInUse) {
            /* Valid entry, then check if match */
            if ((table[probId].lvl == lvl) && (table[probId].key.size() == 4)
                && (table[probId].key[0] == a) && (table[probId].key[1] == b) && (table[probId].key[2] == c) && (table[probId].key[3] == d)) {
                countHits++;
                ans = (char)table[probId].res.getEdgeHandle();
                return 1;
            }
        }
    }
    /* Not cached */
    return 0;
}

bool ComputeTable::check(const uint16_t lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, bool& ans)
{
    countCalls++;
    CacheEntry entry(lvl, a, b, c, d);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "checking in cache, id = " << id << "; size = " << size << std::endl;
#endif
    /* probing the entries*/
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (table[probId].isInUse) {
            /* Valid entry, then check if match */
            if ((table[probId].lvl == lvl) && (table[probId].key.size() == 4)
                && (table[probId].key[0] == a) && (table[probId].key[1] == b) && (table[probId].key[2] == c) && (table[probId].key[3] == d)) {
                countHits++;
                ans = (bool)table[probId].res.getEdgeHandle();
                return 1;
            }
        }
    }
    /* Not cached */
    return 0;
}

void ComputeTable::add(const uint16_t lvl, const Edge& a, const long& ans)
{
    CacheEntry entry(lvl, a);
    entry.setResult(ans);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
    /* insert new entry */
    // while (table[id].isInUse) {
    //     id = (id + 1) % size;
    // }
    // numEntries++;
    // table[id] = entry;
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (!table[probId].isInUse) {
            numEntries++;
            table[probId] = entry;
            break;
        } else if (s == probingSteps - 1) {
            countOverwrite++;
            table[probId] = entry;
        } else {
            continue;
        }
    }
}

void ComputeTable::add(const uint16_t lvl, const Edge& a, const Edge& ans)
{
    CacheEntry entry(lvl, a);
    entry.setResult(ans);
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
    /* insert new entry */
    // while (table[id].isInUse) {
    //     id = (id + 1) % size;
    // }
    // numEntries++;
    // table[id] = entry;
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (!table[probId].isInUse) {
            numEntries++;
            table[probId] = entry;
            break;
        } else if (s == probingSteps - 1) {
            countOverwrite++;
            table[probId] = entry;
        } else {
            continue;
        }
    }
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "entry ID = " << id << ": a: ";
    entry.key[0].print(std::cout);
    std::cout << " ans: ";
    entry.res.print(std::cout);
    std::cout << std::endl;
    std::cout << "table entry: a: ";
    table[id].key[0].print(std::cout);
    std::cout << " ans: ";
    table[id].res.print(std::cout);
    std::cout << std::endl;
#endif
}
void ComputeTable::add(const uint16_t lvl, const Edge& a, const Edge& b, const Edge& ans)
{
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "add entry lvl = " << lvl << ": a: ";
    a.print(std::cout);
    std::cout << " b: ";
    b.print(std::cout);
    std::cout << " ans: ";
    ans.print(std::cout);
    std::cout << std::endl;
#endif
    CacheEntry entry(lvl, a, b);
    entry.setResult(ans);
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "compute hash\n";
#endif
    uint64_t id = entry.hash() % size;
    // uint64_t id = entry.hash() >> (64 - sizeBits);
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "compute hash done\n";
#endif
    /* insert new entry */
    // while (table[id].isInUse) {
    //     id = (id + 1) % size;
    // }
    // numEntries++;
    // table[id] = entry;
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (!table[probId].isInUse) {
            numEntries++;
            table[probId] = entry;
            break;
        } else if (s == probingSteps - 1) {
            countOverwrite++;
            table[probId] = entry;
        } else {
            continue;
        }
    }
#ifdef BRAVE_DD_CACHE_TRACE
    std::cout << "entry ID = " << id << ": a: ";
    entry.key[0].print(std::cout);
    std::cout << " b: ";
    entry.key[1].print(std::cout);
    std::cout << " ans: ";
    entry.res.print(std::cout);
    std::cout << std::endl;
    std::cout << "table entry: a: ";
    table[id].key[0].print(std::cout);
    std::cout << " b: ";
    table[id].key[1].print(std::cout);
    std::cout << " ans: ";
    table[id].res.print(std::cout);
    std::cout << std::endl;
#endif
}

void ComputeTable::add(const uint16_t lvl, const Edge& a, const Edge& b, const char& ans)
{
    CacheEntry entry(lvl, a, b);
    entry.setResult(ans);
    uint64_t id = entry.hash() % size;
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (!table[probId].isInUse) {
            numEntries++;
            table[probId] = entry;
            break;
        } else if (s == probingSteps - 1) {
            countOverwrite++;
            table[probId] = entry;
        } else {
            continue;
        }
    }
}
void ComputeTable::add(const uint16_t lvl, const Edge& a, const Edge& b, const bool& ans)
{
    CacheEntry entry(lvl, a, b);
    entry.setResult(ans);
    uint64_t id = entry.hash() % size;
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (!table[probId].isInUse) {
            numEntries++;
            table[probId] = entry;
            break;
        } else if (s == probingSteps - 1) {
            countOverwrite++;
            table[probId] = entry;
        } else {
            continue;
        }
    }
}
void ComputeTable::add(const uint16_t lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, const Edge& ans)
{
    CacheEntry entry(lvl, a, b, c, d);
    entry.setResult(ans);
    uint64_t id = entry.hash() % size;
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (!table[probId].isInUse) {
            numEntries++;
            table[probId] = entry;
            break;
        } else if (s == probingSteps - 1) {
            countOverwrite++;
            table[probId] = entry;
        } else {
            continue;
        }
    }
}
void ComputeTable::add(const uint16_t lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, const char& ans)
{
    CacheEntry entry(lvl, a, b, c, d);
    entry.setResult(ans);
    uint64_t id = entry.hash() % size;
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (!table[probId].isInUse) {
            numEntries++;
            table[probId] = entry;
            break;
        } else if (s == probingSteps - 1) {
            countOverwrite++;
            table[probId] = entry;
        } else {
            continue;
        }
    }
}
void ComputeTable::add(const uint16_t lvl, const Edge& a, const Edge& b, const Edge& c, const Edge& d, const bool& ans)
{
    CacheEntry entry(lvl, a, b, c, d);
    entry.setResult(ans);
    uint64_t id = entry.hash() % size;
    for (size_t s=0; s<probingSteps; s++) {
        size_t probId = (id + s) % size;
        if (!table[probId].isInUse) {
            numEntries++;
            table[probId] = entry;
            break;
        } else if (s == probingSteps - 1) {
            countOverwrite++;
            table[probId] = entry;
        } else {
            continue;
        }
    }
}

void ComputeTable::sweep(Forest* forest, int role)
{
    // role: 0 for ans, 1 for key0, 2 for key1
    uint16_t lvl = 0;
    NodeHandle target = 0;
    for (size_t i=0; i<table.size(); i++) {
        if (table[i].isInUse) {
            if (role == 0) {
                lvl = table[i].res.getNodeLevel();
                target = table[i].res.getNodeHandle();
            } else if (role == 1) {
                lvl = table[i].key[0].getNodeLevel();
                target = table[i].key[0].getNodeHandle();
            } else if (role == 2) {
                lvl = table[i].key[1].getNodeLevel();
                target = table[i].key[1].getNodeHandle();
            } else {
                std::cout << "[BRAVE_DD] ERROR!\t ComputeTable::sweep(): Unknown role value!" << std::endl;
                exit(0);
            }
            // check target node: if marked continue; otherwise, flip Inuse flag
            if ((lvl > 0) && !forest->getNode(lvl, target).isMarked()) {
                table[i].isInUse = 0;
                numEntries--;
            } else {
                continue;
            }
        } else {
            continue;
        }
    }
}

void ComputeTable::reportStat(std::ostream& out, int format) const
{
    if (format == 0) {
        out << "Computing Table Statistics: \n";
        out << std::left << std::setw(10) << "Size:" << size << "\n";
        out << std::left << std::setw(10) << "Ents:" << numEntries << "\n";
        out << std::left << std::setw(10) << "Calls:" << countCalls << "\n";
        out << std::left << std::setw(10) << "Hits:" << countHits << "\n";
        out << std::left << std::setw(10) << "OWs:" << countOverwrite << "\n";
    }
}

void ComputeTable::enlarge(uint64_t newSize)
{
    // resize the table
    std::vector<CacheEntry> newTable(newSize);
    // rehash
    for (size_t i=0; i<table.size(); i++) {
        if (table[i].isInUse) {
            uint64_t newId = table[i].hash() % newSize;
            // uint64_t newId = table[i].hash() >> (64 - sizeBits);
            // while (newTable[newId].isInUse) {
            //     newId = (newId + 1) % newSize;
            // }
            // newTable[newId] = table[i];
            for (size_t s=0; s<probingSteps; s++) {
                size_t probId = (newId + s) % newSize;
                if (!newTable[probId].isInUse) {
                    newTable[probId] = table[i];
                } else if (s == probingSteps - 1) {
                    newTable[probId] = table[i];
                } else {
                    continue;
                }
            }
        }
    }
    // update num in use
    numEntries = 0;
    for (size_t i=0; i<newTable.size(); i++) {
        if (newTable[i].isInUse) numEntries++;
    }
    // probingSteps = newSize;
    table = std::move(newTable);
}