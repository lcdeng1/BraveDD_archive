#include "unique_table.h"
#include "forest.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                        SubTable methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************
/** If table contains key, move it to the front of the list.
    Otherwise, do nothing.
    Returns the item if found, 0 otherwise.

    Class T must have the following methods:
        unsigned hash():    return the hash value for this item.
        bool equals(int p): return true iff this item equals node p.
*/
template <typename T> 
NodeHandle UniqueTable::SubTable::find(const T &key) const
{
    unsigned h = key.hash() % PRIMES[sizeIndex];
    // CHECK_RANGE(__FILE__, __LINE__, 0u, h, PRIMES[sizeIndex]);
    NodeHandle prev = 0;
    for (NodeHandle ptr = table[h];
            ptr != 0;
            ptr = parent->getNodeNext(level, ptr))
    {
        if (parent->areDuplicates(level, ptr, key)) {
            // MATCH
            if (ptr != table[h]) {
                // Move to front
                BRAVE_DD_DCASSERT(prev);
                parent->setNodeNext(level, prev, parent->getNodeNext(level, ptr));
                parent->setNodeNext(level, ptr, table[h]);
                table[h] = ptr;
            }
            BRAVE_DD_DCASSERT(table[h] == ptr);
            return ptr;
        }
        prev = ptr;
    }

    return 0;
}
// ******************************************************************
// *                                                                *
// *                                                                *
// *                      UniqueTable methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************

UniqueTable::UniqueTable()
{
    //
}
UniqueTable::UniqueTable(Forest* f)
{
    //
}
UniqueTable::~UniqueTable()
{
    //
}

