#include "node_manager.h"
#include "forest.h"


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
    nodes = (Node*)malloc(PRIMES[sizeIndex] * sizeof(Node));
    recycled = 0;
    firstUnalloc = 0;
    freeList = 0;
    numFrees = PRIMES[sizeIndex];
}
NodeManager::SubManager::~SubManager()
{
    for (uint32_t i=0; i<firstUnalloc; i++) {
        nodes[i].~Node();
    }
    free(nodes);
}

NodeHandle NodeManager::SubManager::getFreeNodeHandle(const Node& node)
{
    /* Re-use the recycled handle, if we have one */
    if (recycled) {
        const NodeHandle h = recycled - 1;
        recycled = 0;
        nodes[h] = node;
        return h;
    }
    /* Enlarge if there is no free/unused slots */
    if (!numFrees) expand();
    /* There are definitely free slots. Pull form there, if the free list is not empty */
    numFrees--;
    if (freeList) {
        // pull from the free list
        NodeHandle h = freeList - 1;
        freeList = nodes[h].info[2];
        nodes[h] = node;
        return h;
    }
    /* Free list is empty, so pull from the unallocated end portion */
    new (&nodes[firstUnalloc]) Node(parent->getSetting());
    nodes[firstUnalloc] = node;
    return firstUnalloc++;
}

Node& NodeManager::SubManager::getNodeFromHandle(const NodeHandle h)
{
    //
    if (h>=firstUnalloc) {
        std::cout << "[BRAVE_DD] ERROR!\t Invalid handle in node submanager; " 
        << firstUnalloc-1 << "nodes are allocated" << std::endl;
        exit(0);
    }
    return nodes[h];
}

void NodeManager::SubManager::expand()
{
    // Check if we can enlarge
    if (PRIMES[sizeIndex] >= UINT32_MAX) {  // MAX of uint32
        std::cout << "[BRAVE_DD] ERROR!\t Out of nodes in node submanager; " 
        << PRIMES[sizeIndex] << "nodes are full" << std::endl;
        exit(0);
    }
    // Enlarge
    sizeIndex++;
    uint32_t newSize = 0;
    if (PRIMES[sizeIndex] > UINT32_MAX) {
        newSize = UINT32_MAX;
    } else {
        newSize = PRIMES[sizeIndex];
    }
    nodes = (Node*)realloc(nodes, newSize * sizeof(Node));
    numFrees += (newSize - PRIMES[sizeIndex-1]);
    firstUnalloc = PRIMES[sizeIndex-1];
}

void NodeManager::SubManager::shrink()
{
    //
    sizeIndex--;
    uint32_t newSize = PRIMES[sizeIndex];
    nodes = (Node*)realloc(nodes, newSize * sizeof(Node));
    numFrees -= (PRIMES[sizeIndex+1] - newSize);
}

void NodeManager::SubManager::sweep()
{
    if (!nodes) return;
    /* Expand the unallocated portion as much as we  can */
    while (firstUnalloc) {
        if ((nodes+firstUnalloc-1)->isMarked()) {
            break;
        }
        (nodes+firstUnalloc-1)->~Node();
        firstUnalloc--;
    }
    /* Check if we can shrink */
    if (firstUnalloc < PRIMES[sizeIndex-1]) {
        shrink();
    }
    /* Rebuild the free list, by scanning all nodes backwards.
       Unmarked nodes are added to the list. */
    numFrees = ((PRIMES[sizeIndex]>UINT32_MAX)? UINT32_MAX:PRIMES[sizeIndex]) - firstUnalloc;
    freeList = 0;
    for (uint32_t i=firstUnalloc; i; --i) {
        if (nodes[i-1].isMarked()) {
            nodes[i-1].unmark();
        } else {
            nodes[i-1].recycle(freeList);
            freeList = i;
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
    chunks = (SubManager*)malloc(lvls * sizeof(SubManager));
    for (uint32_t i=0; i<lvls; i++) {
        new (&chunks[i]) SubManager(f);
    }
}
NodeManager::~NodeManager()
{
    for (uint16_t i=0; i<parent->getSetting().getNumVars(); i++) {
        chunks[i].~SubManager();
    }
    free(chunks);
    parent = 0;
}