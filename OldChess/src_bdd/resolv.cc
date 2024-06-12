
#include "resolv.h"

resolver::resolver(bdd_forest &bdd, compute_table &ct)
 : BDD(bdd), CT(ct)
{
  toplevel = 0;
  botlevel = 0;
  invoke_count = 0;
}

resolver::~resolver()
{
}

