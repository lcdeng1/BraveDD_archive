
#include "buffer.h"
#include "resolv.h"
#include "bdd.h"

#include <cstring>

minterm_buffer::minterm_buffer(bdd_forest &bdd, bdd_edge &r, unsigned bs, resolver* res) : BDD(bdd), root(r), bufsize(bs)
{
  bufptr = 0;
  minterms = 0;

  R = res;
  last_add = 0;

  statebuf = new char* [bufsize];
  for (unsigned u=0; u<bufsize; u++) {
    statebuf[u] = new char[1+BDD.get_num_vars()];
  }
}

minterm_buffer::~minterm_buffer()
{
  for (unsigned u=0; u<bufsize; u++) {
    delete[] statebuf[u];
  }
  delete[] statebuf;
}

bool minterm_buffer::add(const char* linebuf, char term)
{
  BDD.varray2minterm(linebuf, statebuf[bufptr]);
  statebuf[bufptr][0] = term;

  if (R) {
    if (last_add) {
      unsigned short tc = top_changed(last_add, statebuf[bufptr]);
      if (tc > R->topLevel()) {

        // Flush before bufptr
        if (bufptr) {
          root = BDD.union_minterms(root, statebuf, bufptr);
          minterms += bufptr;
        }

        // Invoke Resolver
        unsigned short toplev = R->topLevel();
        R->set_levels(tc-1, 1);
        root = BDD.newEdge(root.edge, R->invoke(root)); 
        R->set_levels(toplev, 1);

        if (bufptr) {
          // Save state
          memcpy(statebuf[0], statebuf[bufptr], 1+BDD.get_num_vars());
        }
        last_add = statebuf[0];
        bufptr = 1;

        return true;
      } 
    }
    last_add = statebuf[bufptr];
  }

  if (++bufptr < bufsize) return false;

  root = BDD.union_minterms(root, statebuf, bufptr);
  minterms += bufptr;
  bufptr = 0;

  return true;
}

void minterm_buffer::flush()
{
  if (bufptr) {
    root = BDD.union_minterms(root, statebuf, bufptr);
    minterms += bufptr;
    bufptr = 0;
  }
}


unsigned short minterm_buffer::top_changed(const char* state1, const char* state2) const
{
  for (unsigned short L=static_cast <unsigned short>(BDD.get_num_vars()); L; L--) {
    if (state1[L] != state2[L]) return L;
  }
  return 0;
}

