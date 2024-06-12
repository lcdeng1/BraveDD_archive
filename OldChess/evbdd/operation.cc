
#include "operation.h"

operation::operation(bdd_forest &bdd, compute_table &ct, const char* n)
 : name(n), CT(ct), BDD(bdd)
{
  answer_is_node = true;
  OPCODE = CT.register_op(this);
}

operation::~operation()
{
  CT.unregister_op(this);
}
