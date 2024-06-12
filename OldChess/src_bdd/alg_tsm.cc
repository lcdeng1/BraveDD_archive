
#include "bdd.h"
#include "ct.h"
#include "operation.h"
#include "resolv.h"

#define OLD
#ifdef OLD

//
// Checks if two nodes can be concretized to each other.
// That means they are equal except possibly for don't cares.
//

class has_common_op : public operation {
  public:
    has_common_op(bdd_forest &bdd, compute_table &ct);

  public:
    bool apply(unsigned P, unsigned Q);
};

has_common_op::has_common_op(bdd_forest &bdd, compute_table &ct)
 : operation(bdd, ct, "has_common")
{
  set_answer_other();
  set_binary_op();
}

bool has_common_op::apply(unsigned P, unsigned Q)
{
  if (P == Q) return true;
  if (P == 0) return true;
  if (Q == 0) return true;
  if (is_terminal(P) && is_terminal(Q)) return false;

  if (P > Q) {
    unsigned T = P;
    P = Q;
    Q = T;
  } 
  unsigned answer;

  if (has_BCT_entry(P, Q, answer)) {
    return answer;
  }

  bool cmp0, cmp1;
  if (node_level(P) < node_level(Q)) {
    cmp0 = apply(P, node_child0(Q));
    cmp1 = apply(P, node_child1(Q));
  } else if (node_level(P) > node_level(Q)) {
    cmp0 = apply(node_child0(P), Q);
    cmp1 = apply(node_child1(P), Q);
  } else {
    cmp0 = apply(node_child0(P), node_child0(Q));
    cmp1 = apply(node_child1(P), node_child1(Q));
  }

  answer = (cmp0 && cmp1) ? 1 : 0;

  return add_BCT_entry(P, Q, answer);
}


//
// Build common concretization for two nodes.
// Undefined if the comparison op returns false.
//
// Note: if we had an "illegal" value, we could use this
// in place of the comparison.
//

class common_op : public operation {
  public:
    common_op(bdd_forest &bdd, compute_table &ct);

  public:
    unsigned apply(unsigned P, unsigned Q);
};

common_op::common_op(bdd_forest &bdd, compute_table &ct)
 : operation(bdd, ct, "common")
{
  set_answer_node();
  set_binary_op();
}

unsigned common_op::apply(unsigned P, unsigned Q)
{
  if (P == Q) return P;
  if (P == 0) return Q;
  if (Q == 0) return P;
  if (is_terminal(P) && is_terminal(Q)) return 0;   

  if (P > Q) {
    unsigned T = P;
    P = Q;
    Q = T;
  } 
  unsigned answer;

  if (has_BCT_entry(P, Q, answer)) {
    return answer;
  }

  bdd_node tmp;
#ifdef WITH_SWAPS
  tmp.swap[0] = false;
  tmp.swap[1] = false;
#endif
  if (node_level(P) < node_level(Q)) {
    tmp.level = node_level(Q);
    tmp.child[0] = apply(P, node_child0(Q));
    tmp.child[1] = apply(P, node_child1(Q));
  } else if (node_level(P) > node_level(Q)) {
    tmp.level = node_level(P);
    tmp.child[0] = apply(node_child0(P), Q);
    tmp.child[1] = apply(node_child1(P), Q);
  } else {
    tmp.level = node_level(P);
    tmp.child[0] = apply(node_child0(P), node_child0(Q));
    tmp.child[1] = apply(node_child1(P), node_child1(Q));
  }

  return add_BCT_entry(P, Q, add_node(tmp));
}

#else


/*
   Build common concretization for two nodes.
   If there is no common concretization, return 0.
   Otherwise, return 1 + the node encoding the concretization.
*/
class common_op : public operation {
  public:
    common_op(bdd_forest &bdd, compute_table &ct);

  public:
    unsigned apply(unsigned P, unsigned Q);
};

common_op::common_op(bdd_forest &bdd, compute_table &ct)
 : operation(bdd, ct, "common")
{
  set_answer_other();
  set_binary_op();
}

unsigned common_op::apply(unsigned P, unsigned Q)
{
  if (P == Q) return P+1;
  if (P == 0) return Q+1;
  if (Q == 0) return P+1;
  if (is_terminal(P) && is_terminal(Q)) return 0;   

  if (P > Q) {
    unsigned T = P;
    P = Q;
    Q = T;
  } 
  unsigned answer;

  if (has_BCT_entry(P, Q, answer)) {
    return answer;
  }

  bdd_node tmp;
#ifdef WITH_SWAPS
  tmp.swap[0] = false;
  tmp.swap[1] = false;
#endif
  if (node_level(P) < node_level(Q)) {
    tmp.level = node_level(Q);
    tmp.child[0] = apply(P, node_child0(Q));
    tmp.child[1] = apply(P, node_child1(Q));
  } else if (node_level(P) > node_level(Q)) {
    tmp.level = node_level(P);
    tmp.child[0] = apply(node_child0(P), Q);
    tmp.child[1] = apply(node_child1(P), Q);
  } else {
    tmp.level = node_level(P);
    tmp.child[0] = apply(node_child0(P), node_child0(Q));
    tmp.child[1] = apply(node_child1(P), node_child1(Q));
  }

  if (0==tmp.child[0] || 0==tmp.child[1]) return 0;
  --tmp.child[0];
  --tmp.child[1];

  return add_BCT_entry(P, Q, 1+add_node(tmp));
}


#endif

//
// Top down, make redundant nodes based on "compare" operation
//

class tsm_op : public operation {
    unsigned sequence;
    unsigned short topL;
    unsigned short botL;
#ifdef OLD
    has_common_op &COMP;
#endif
    common_op &COMMON;
  public:
#ifdef OLD
    tsm_op(bdd_forest &bdd, compute_table &ct, has_common_op &comp, common_op &common);
#else
    tsm_op(bdd_forest &bdd, compute_table &ct, common_op &common);
#endif
    virtual ~tsm_op() { }

    inline void set_levels(unsigned short toplevel, unsigned short botlevel)
    {
      topL = toplevel;
      botL = botlevel;
      sequence = (unsigned(topL) << 16) | botL;
    }

    unsigned apply(unsigned P);
};

#ifdef OLD
tsm_op::tsm_op(bdd_forest &bdd, compute_table &ct, has_common_op &comp,
  common_op &common) : operation(bdd, ct, "tsm_mt"), COMP(comp), COMMON(common)
#else
tsm_op::tsm_op(bdd_forest &bdd, compute_table &ct, common_op &common)
 : operation(bdd, ct, "tsm_mt"), COMMON(common)
#endif
{
  set_unary_op();
  set_answer_node();
}

unsigned tsm_op::apply(unsigned root)
{
  if (is_terminal(root)) return root;
  if (node_level(root) < botL) return root;

  unsigned ans;
  if (has_sUCT_entry(root, sequence, ans)) return ans;

#ifdef OLD
  if (COMP.apply(node_child0(root), node_child1(root))) {
    return add_sUCT_entry(root, sequence,
      apply(COMMON.apply(node_child0(root), node_child1(root))));
  }
#else
  unsigned C = COMMON.apply(node_child0(root), node_child1(root));
  if (C) return add_sUCT_entry(root, sequence, C-1);
#endif

  bdd_node tmp;
#ifdef WITH_SWAPS
  tmp.swap[0] = false;
  tmp.swap[1] = false;
#endif
  tmp.level = node_level(root);
  tmp.child[0] = apply(node_child0(root));
  tmp.child[1] = apply(node_child1(root));

  return add_sUCT_entry(root, sequence, add_node(tmp));
}


/*
  New front end
*/

class tsm_resolver : public resolver {
#ifdef OLD
    has_common_op* comp;
#endif
    common_op* common;
    tsm_op* OP;
  public:
    tsm_resolver(bdd_forest &BDD, compute_table &ct);
    virtual ~tsm_resolver();

    virtual unsigned invoke_specific(unsigned root);
};

tsm_resolver::tsm_resolver(bdd_forest &bdd, compute_table &ct)
 : resolver(bdd, ct)
{
#ifdef OLD
  comp = new has_common_op(BDD, ct);
  common = new common_op(BDD, ct);
  OP = new tsm_op(BDD, ct, *comp, *common);
#else
  common = new common_op(BDD, ct);
  OP = new tsm_op(BDD, ct, *common);
#endif
}

tsm_resolver::~tsm_resolver()
{
  delete OP; 
  delete common;
#ifdef OLD
  delete comp;
#endif
}

unsigned tsm_resolver::invoke_specific(unsigned root)
{
  OP->set_levels(topLevel(), botLevel());
  return OP->apply(root);
}

resolver* two_side_match_mt(bdd_forest &BDD, compute_table &CT)
{
  return new tsm_resolver(BDD, CT);
}


