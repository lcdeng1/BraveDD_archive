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
UniqueTable::SubTable::SubTable(uint16_t lvl, Forest* f):parent(f),level(lvl)
{
    sizeIndex = 0;
    table = std::vector<NodeHandle>(PRIMES[sizeIndex], 0);
    numEntries = 0;
}
UniqueTable::SubTable::~SubTable()
{
    table.clear();
    std::vector<NodeHandle>().swap(table);
    sizeIndex = 0;
    numEntries = 0;
}

NodeHandle UniqueTable::SubTable::insert(const Node& node)
{
    /* Check if we should enlarge */
    if (numEntries >= PRIMES[sizeIndex+1]) expand();
    /* Determine the hash index for the node */
    uint32_t index = node.hash(parent->nodeSize) % getSize();
    // Special, and hopefully common, case: empty chain. Which means the node is new.
    if (!table[index]) {
        numEntries++;
        table[index] = parent->obtainFreeNodeHandle(level, node);
        return table[index];
    }
    // Non-empty chain. Check the chain for duplicates.
    NodeHandle curr = table[index];
    while (curr) {
        if (!parent->getNode(level, curr).isEqual(node, parent->nodeSize)) {
            curr = parent->getNodeNext(level, curr);
            continue;
        }
        // we found a duplicate. Move it to the front.
        return curr;
    }
    // No duplicates in the chain. 
    // Get a new node handle, and add the new node to the front.
    numEntries++;
    NodeHandle handle = parent->obtainFreeNodeHandle(level, node);
    parent->setNodeNext(level, handle, table[index]);
    table[index] = handle;
    return handle;
}

void UniqueTable::SubTable::sweep()
{
    /* For each chain, traverse and keep only the marked items */
    numEntries = 0;
    NodeHandle curr, prev;
    for (uint32_t i=0; i<PRIMES[sizeIndex]; i++) {
        prev = 0;
        curr = table[i];
        while (curr) {
            if (parent->getNode(level, curr).isMarked()) {
                if (prev) {
                    parent->setNodeNext(level, prev, curr);
                } else {
                    table[i] = curr;
                }
                numEntries++;
                prev = curr;
            }
            curr = parent->getNodeNext(level, curr);
        }
        /* Done traversing; null terminate the new chain */
        if (prev) {
            parent->setNodeNext(level, prev, 0);
        } else {
            table[i] = 0;
        }
    }
    /* Check if we should shrink the table. TBD */
}

void UniqueTable::SubTable::expand()
{
    // Check if we can enlarge
    if (PRIMES[sizeIndex] >= UINT32_MAX) {  // MAX of uint32
        std::cout << "[BRAVE_DD] ERROR!\t Unable to enlarge SubUniqueTable!"
        << "\n\t\tToo many nodes at level: " << level << std::endl;
        exit(0);
    }
    /* Enlarge */
    // table to list, waiting for realloc
    NodeHandle front = 0, chain = 0;
    for (uint32_t i=0; i<PRIMES[sizeIndex]; i++) {
        while (table[i]) {
            chain = table[i];
            table[i] = parent->getNodeNext(level, chain);
            parent->setNodeNext(level, chain, front);
            front = chain;
        }
    }
    numEntries = 0;
    // new size
    sizeIndex++;
    uint32_t newSize = 0;
    if (PRIMES[sizeIndex] > UINT32_MAX) {
        newSize = UINT32_MAX;
    } else {
        newSize = PRIMES[sizeIndex];
    }
    // new table of larger size
    table.resize(newSize);
    for (uint32_t i=0; i<newSize; i++) {
        table[i] = 0;
    }
    // rehash
    NodeHandle next;
    uint32_t newIndex;
    while (front) {
        // save next, before we overwrite it
        next = parent->getNodeNext(level, front);
        // compute new hash and get new index
        newIndex = parent->getNodeHash(level, front) % newSize;
        // add to the front of the new list
        parent->setNodeNext(level, front, table[newIndex]);
        table[newIndex] = front;
        // advance
        front = next;
        numEntries++;
    }
}
// ******************************************************************
// *                                                                *
// *                                                                *
// *                      UniqueTable methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************

UniqueTable::UniqueTable(Forest* f):parent(f)
{
    uint16_t lvls = f->getSetting().getNumVars();
    tables = std::vector<SubTable>(lvls, SubTable(1, f));
    for (uint16_t i=0; i<lvls; i++) {
        tables[i].level = i+1;
    }
}
UniqueTable::~UniqueTable()
{
    tables.clear();
    std::vector<SubTable>().swap(tables);
    parent = 0;
}

