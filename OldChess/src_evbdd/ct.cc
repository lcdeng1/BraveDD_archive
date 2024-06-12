
#include "ct.h"
#include "operation.h"
#include <cstring>

inline void hash(unsigned &h, unsigned char b)
{
  h = (h*31) ^ b;
}

inline void hash(unsigned &h, unsigned b)
{
  h = (h*31) ^ ((b & 0xff000000) >> 24);
  h = (h*31) ^ ((b & 0x00ff0000) >> 16);
  h = (h*31) ^ ((b & 0x0000ff00) >> 8);
  h = (h*31) ^ (b & 0x000000ff);
}

inline unsigned hash(unsigned char b, unsigned P)
{
  unsigned h = 0xdeadbeef;
  hash(h, b);
  hash(h, P);
  return h;
}

inline unsigned hash(unsigned char b, unsigned P, unsigned Q)
{
  unsigned h = 0xdeadbeef;
  hash(h, b);
  hash(h, P);
  hash(h, Q);
  return h;
}

compute_table::compute_table(unsigned sz)
 : size(sz)
{
  oparray = new unsigned char[size];
  Parray = new unsigned[size];
  Qarray = new unsigned[size];
  Rarray = new unsigned[size];

  bzero(oparray, size);

  for (unsigned i=0; i<256; i++) {
    optable[i] = 0;
    opstatus[i] = 0;
  }
}

compute_table::~compute_table()
{
  delete[] oparray;
  delete[] Parray;
  delete[] Qarray;
  delete[] Rarray;

  for (unsigned u=0; u<256; u++) delete optable[u];
}

unsigned char compute_table::register_op(operation* op)
{
  if (0==op) return 0;  // sanity check

  for (unsigned u=1; u<256; u++) {
    if (optable[u] == op) {
      opstatus[u] = 1;
      return static_cast <unsigned char>(u);
    }
  }
  for (unsigned u=1; u<256; u++) {
    if (opstatus[u]) continue;
    optable[u] = op;
    opstatus[u] = 1;
    return static_cast <unsigned char>(u);
  }
  throw "Operation table full";
  // return 0;
}

void compute_table::unregister_op(operation* op)
{
  if (0==op) return;  // sanity check

  for (unsigned u=1; u<256; u++) {
    if (optable[u] == op) {
      opstatus[u] = 2;
      optable[u] = 0;
      return;
    }
  }
}

void compute_table::remove_stales(unsigned short seqno)
{
  for (unsigned u=0; u<size; u++) {
    unsigned char opind = oparray[u];
    if (0==opind) continue;
    if (opstatus[opind]!=1) {
      oparray[u] = 0;
      continue;
    }
    const operation* op = optable[oparray[u]];
    ASSERT(op);
    if (op->entry_is_stale(seqno, Parray[u], Qarray[u], Rarray[u])) {
      oparray[u] = 0;
    }
  }
  // Any unregistered ops can be cleared
  for (unsigned u=0; u<256; u++) {
    if (opstatus[u]==2) {
      opstatus[u] = 0;
    }
  }
}


//
// Unary
//

bool compute_table::find_unary(unsigned char op, unsigned P, unsigned &ans) const
{
  unsigned H;
  // check H, H+1, H+2 

  if (equals (H=hash(op, P) % size, op, P)) 
  {
    ans = Rarray[H];
    return true;
  }

  if (equals (H = (H+1) % size, op, P)) 
  {
    ans = Rarray[H];
    return true;
  }

  if (equals (H = (H+1) % size, op, P)) 
  {
    ans = Rarray[H];
    return true;
  }

  return false;
}

void compute_table::add_unary(unsigned char op, unsigned P, unsigned ans)
{
  const unsigned H0 = hash(op, P) % size;

  unsigned H = H0;
  // check H, H+1, H+2 for empty.  If none, use H.

  if (0==oparray[H]) {
    return set(H, op, P, ans);
  }

  H = (H+1) % size;

  if (0==oparray[H]) {
    return set(H, op, P, ans);
  }

  H = (H+1) % size;

  if (0==oparray[H]) {
    return set(H, op, P, ans);
  }

  set(H0, op, P, ans);
}


// 
// Binary
//

bool compute_table::find_binary(unsigned char op, unsigned P, unsigned Q, unsigned &ans) const
{
  unsigned H;
  // check H, H+1, H+2 

  if (equals (H=hash(op, P, Q) % size, op, P, Q)) 
  {
    ans = Rarray[H];
    return true;
  }

  if (equals (H = (H+1) % size, op, P, Q)) 
  {
    ans = Rarray[H];
    return true;
  }

  if (equals (H = (H+1) % size, op, P, Q)) 
  {
    ans = Rarray[H];
    return true;
  }

  return false;
}

void compute_table::add_binary(unsigned char op, unsigned P, unsigned Q, unsigned ans)
{
  const unsigned H0 = hash(op, P, Q) % size;

  unsigned H = H0;
  // check H, H+1, H+2 for empty.  If none, use H.

  if (0==oparray[H]) {
    return set(H, op, P, Q, ans);
  }

  H = (H+1) % size;

  if (0==oparray[H]) {
    return set(H, op, P, Q, ans);
  }

  H = (H+1) % size;

  if (0==oparray[H]) {
    return set(H, op, P, Q, ans);
  }

  set(H0, op, P, Q, ans);
}

