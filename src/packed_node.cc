#include "packed_node.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                     PackedNode methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

PackedNode::PackedNode()
{
    //
}
PackedNode::PackedNode(Node& node)
{
    info = (uint32_t*)malloc(5 * sizeof(uint32_t));
    for (int i=0; i<5; i++) {
        info[i] = 0;
    }
    values = 0;
}
PackedNode::PackedNode(Mxnode& node)
{
    info = (uint32_t*)malloc(8 * sizeof(uint32_t));
    for (int i=0; i<8; i++) {
        info[i] = 0;
    }
    info[1] |= NODE_TYPE_MASK;
    values = (uint64_t*)malloc(3 * sizeof(uint64_t));
    for (int i=0; i<3; i++) {
        values[i] = 0;
    }
}
PackedNode::~PackedNode()
{
    for (int i=0; info[i]; i++) info[i] = 0;
    free(info);
    for (int i=0; values[i]; i++) values[i] = 0;
    free(values);
}