#ifndef BRAVE_HELPERS_H
#define BRAVE_HELPERS_H

#include "forest.h"  // Needed for Brave_DD::Edge and Brave_DD::Forest

#ifdef __cplusplus
extern "C" {
#endif

// Use Brave_DD::Edge and Brave_DD::Forest directly
BRAVE_DD::Edge union_minterm(BRAVE_DD::Forest* F, BRAVE_DD::Edge* root, char* minterm, int outcome, uint32_t K);
BRAVE_DD::Edge union_minterms_radixscan(BRAVE_DD::Forest* F, uint32_t K, BRAVE_DD::Edge* root, char** minterms, int outcome, unsigned int N);

#ifdef __cplusplus
}
#endif

#endif  // BRAVE_HELPERS_H
