#ifndef BRAVE_DD_NODE_STORAGE_H
#define BRAVE_DD_NODE_STORAGE_H

#include "defines.h"
#include "node.h"
#include "setting.h"

namespace BRAVE_DD {
    class NodeStorage;
}

// ******************************************************************
// *                                                                *
// *                   Node Storage  class                          *
// *                                                                *
// ******************************************************************
/** Abstract class for packed node storage
 *  
 */
class BRAVE_DD::NodeStorage {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    /// Default construction for Rex
    NodeStorage();
    /// Construction based on settings
    NodeStorage(ForestSetting *s);
    ~NodeStorage();
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    char bytesPerEdge;
};


#endif