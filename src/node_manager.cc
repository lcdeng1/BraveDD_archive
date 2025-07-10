#include "node_manager.h"
#include "forest.h"

// #define BRAVE_DD_NM_TRACE

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                       SubManager methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************

NodeManager::SubManager::SubManager(Forest *f):parent(f)
{
    sizeIndex = 0;
    nodes = std::vector<Node>((PRIMES[sizeIndex] + 1), Node(parent->nodeSize));
    recycled = 0;
    firstUnalloc = 1;
    freeList = 0;
    numFrees = PRIMES[sizeIndex];
    peak = 0;
}
NodeManager::SubManager::~SubManager()
{
    // for (uint32_t i=1; i<firstUnalloc; i++) {
    //     nodes[i].~Node();
    // }
    nodes.clear();
    std::vector<Node>().swap(nodes);
}

NodeHandle NodeManager::SubManager::getFreeNodeHandle(const Node& node)
{
    /* Re-use the recycled handle, if we have one */
    if (recycled) {
        const NodeHandle h = recycled;
        recycled = 0;
        nodes[h].assign(node, parent->nodeSize);
        return h;
    }
    /* Enlarge if there is no free/unused slots */
    if (!numFrees) expand();
    /* There are definitely free slots. Pull form there, if the free list is not empty */
    numFrees--;
    if (freeList) {
        // pull from the free list
        NodeHandle h = freeList;
        freeList = nodes[h].nextFree();
        nodes[h].assign(node, parent->nodeSize);
        return h;
    }
    /* Free list is empty, so pull from the unallocated end portion */
    new (&nodes[firstUnalloc]) Node(parent->nodeSize);
    nodes[firstUnalloc].assign(node, parent->nodeSize);
    if (firstUnalloc > peak) peak = firstUnalloc;   // update peak
    return firstUnalloc++;
}

Node& NodeManager::SubManager::getNodeFromHandle(const NodeHandle h)
{
    if (h>=firstUnalloc) {
        std::cout << "[BRAVE_DD] ERROR!\t getNodeFromHandle(): Invalid handle in node submanager; " 
        << firstUnalloc-1 << " slots are allocated" << std::endl;
        exit(0);
    }
    return nodes[h];
}

uint32_t NodeManager::SubManager::getNumMarked() const
{
    if (firstUnalloc <= 1) return 0;
    uint32_t num = 0;
    for (uint32_t i=firstUnalloc-1; i>0; i--) {
        if (nodes[i].isMarked()) num++;
    }
    return num;
}

void NodeManager::SubManager::expand()
{
    // Check if we can enlarge
    if (PRIMES[sizeIndex] >= UINT32_MAX) {  // MAX of uint32
        std::cout << "[BRAVE_DD] ERROR!\t expand(): Unable to enlarge node submanager!" << std::endl;
        exit(0);
    }
    // Enlarge
    sizeIndex++;
    uint32_t newSize = 0;
    if (PRIMES[sizeIndex] > UINT32_MAX) {
        newSize = UINT32_MAX + 1;
    } else {
        newSize = PRIMES[sizeIndex] + 1;
    }
    nodes.resize(newSize, Node(parent->nodeSize));
    numFrees += (newSize - PRIMES[sizeIndex-1] - 1);
}

void NodeManager::SubManager::shrink()
{
    uint32_t newSize = 1;
    sizeIndex--;
    if (sizeIndex >= 0) newSize = PRIMES[sizeIndex] + 1;
    nodes.resize(newSize, Node(parent->nodeSize));
    nodes.shrink_to_fit();
    numFrees -= (PRIMES[sizeIndex+1] + 1 - newSize);
}

void NodeManager::SubManager::sweep()
{
    if (firstUnalloc == 1) return;
    /* Expand the unallocated portion as much as we  can */
    while (firstUnalloc > 1) {
        if (nodes[firstUnalloc-1].isMarked()) {
            break;
        }
        nodes[firstUnalloc-1].~Node();
        firstUnalloc--;
    }
    numFrees = ((PRIMES[sizeIndex]>UINT32_MAX)? UINT32_MAX:PRIMES[sizeIndex]) + 1 - firstUnalloc;
    /* Check if we can shrink */
    // TBD
    // if (firstUnalloc < PRIMES[sizeIndex-1]) {
    //     shrink();
    // }
    /* Rebuild the free list, by scanning all nodes backwards.
       Unmarked nodes are added to the list. */
    freeList = 0;
    for (uint32_t i=firstUnalloc; i>1; --i) {
        if (nodes[i-1].isMarked()) {
            nodes[i-1].unmark();
        } else {
            nodes[i-1].recycle(freeList);
            freeList = i-1;
            numFrees++;
        }
    }
}
// ******************************************************************
// *                                                                *
// *                                                                *
// *                      NodeManager methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************

NodeManager::NodeManager(Forest *f):parent(f)
{
    uint16_t lvls = f->getSetting().getNumVars();
    // chunks = std::vector<SubManager>(lvls, SubManager(f));
    chunks.reserve(lvls);
    for (uint16_t i = 0; i < lvls; i++) {
        chunks.emplace_back(f);  // Directly constructs SubManager
    }
    peak = 0;
}
NodeManager::~NodeManager()
{
    // for (uint16_t i=0; i<parent->getSetting().getNumVars(); i++) {
    //     chunks[i].~SubManager();
    // }
    chunks.clear();
    std::vector<SubManager>().swap(chunks);
    parent = 0;
    peak = 0;
}

void NodeManager::sweep(uint16_t lvl)
{
    uint32_t beforeNum = numUsed(lvl);
    chunks[lvl-1].sweep();
    numNodes += (numUsed(lvl) - beforeNum);     // update number of used nodes
}

void NodeManager::sweep()
{
    numNodes = 0;
    for (uint16_t k=1; k<=parent->getSetting().getNumVars(); k++) {
        numNodes += numUsed(k);     // update number of used nodes
        sweep(k);
    }
}

void NodeManager::unmark(uint16_t lvl)
{
#ifdef BRAVE_DD_NM_TRACE
    std::cout << "unmark: unmark lvl = " << lvl << std::endl;
    std::cout << "\tfirstUnalloc = " << chunks[lvl-1].firstUnalloc << "; size = " << PRIMES[chunks[lvl-1].sizeIndex] << std::endl;
#endif
    for (uint32_t i=1; i<chunks[lvl-1].firstUnalloc; i++) {
        if (chunks[lvl-1].nodes[i].isMarked()) {
            chunks[lvl-1].nodes[i].unmark();
        }
    }
}

void NodeManager::unmark()
{
    for (uint16_t k=1; k<=parent->getSetting().getNumVars(); k++) {
        unmark(k);
    }
}