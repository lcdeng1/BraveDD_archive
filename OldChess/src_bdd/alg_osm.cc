
#include "bdd.h"
#include "ct.h"
#include "operation.h"
#include "resolv.h"

//
// Compares two nodes P and Q.
//    Returns one of:
//      = : if P == Q
//      ! : if P != Q and neither can be replaced with the other
//      > : if P -> Q, i.e., we can replace undefs in P so that it equals Q
//      < : if P <- Q, i.e., we can replace undefs in Q so that it equals P
// 

class compare_op : public operation {
  public:
    compare_op(bdd_forest &bdd, compute_table &ct);

  public:

    unsigned char apply(unsigned P, unsigned Q);

    static inline unsigned char flip_comparison(bool flip, unsigned char cmp)
    {
      if (cmp == '<') return flip ? '>' : '<';
      if (cmp == '>') return flip ? '<' : '>';
      return cmp;
    }
};

compare_op::compare_op(bdd_forest &bdd, compute_table &ct)
 : operation(bdd, ct, "compare")
{
  set_answer_other();
  set_binary_op();
}

unsigned char compare_op::apply(unsigned P, unsigned Q)
{
  if (P == Q) return '=';
  if (P == 0) return '>';
  if (Q == 0) return '<';
  if (is_terminal(P) && is_terminal(Q)) return '!';

  bool swap = false;
  if (P > Q) {
    // Deal with 'almost' commutativity
    unsigned T = P;
    P = Q;
    Q = T;
    swap = true;
  } 
  unsigned answer;

  if (has_BCT_entry(P, Q, answer)) {
    return flip_comparison(swap, static_cast<unsigned char>(answer));
  }

  unsigned char cmp0, cmp1;
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

  if ( ('=' == cmp0) || (cmp0 == cmp1) ) {
    answer = cmp1;
  } else {
    answer = '!';
  }

  return flip_comparison(swap, static_cast<unsigned char>(add_BCT_entry(P, Q, answer)));
}


//
// Top down, make redundant nodes based on "compare" operation
//

class osm_op : public operation {
    unsigned sequence;
    unsigned short topL;
    unsigned short botL;
    compare_op &COMP;
  public:
    osm_op(bdd_forest &bdd, compute_table &ct, compare_op &comp);

    inline void set_levels(unsigned short toplevel, unsigned short botlevel)
    {
      topL = toplevel;
      botL = botlevel;
      sequence = (unsigned(topL) << 16) | botL;
    }

  public:

    unsigned apply(unsigned P);
};

osm_op::osm_op(bdd_forest &bdd, compute_table &ct, compare_op &comp)
 : operation(bdd, ct, "osm_mt"), COMP(comp)
{
  set_unary_op();
  set_answer_node();
}

unsigned osm_op::apply(unsigned root)
{
  if (is_terminal(root)) return root;

  if (node_level(root) < botL) return root;

  unsigned ans;
  if (has_sUCT_entry(root, sequence, ans)) return ans;

  if (node_level(root) <= topL) {
    unsigned char child_cmp = COMP.apply(node_child0(root), node_child1(root));
    if ('>' == child_cmp) {
      // child 0 -> child 1
      return add_sUCT_entry(root, sequence, apply(node_child1(root)));
    }
    if ('<' == child_cmp) {
      // child 1 -> child 0
      return add_sUCT_entry(root, sequence, apply(node_child0(root)));
    }
    ASSERT('=' != child_cmp); // should be impossible
  }

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

class osm_resolver : public resolver {
    compare_op* comp;
    osm_op* OP;
  public:
    osm_resolver(bdd_forest &BDD, compute_table &ct);
    virtual ~osm_resolver();

    virtual unsigned invoke_specific(unsigned root);
};

osm_resolver::osm_resolver(bdd_forest &bdd, compute_table &ct)
 : resolver(bdd, ct)
{
  comp = new compare_op(BDD, ct);
  OP = new osm_op(BDD, ct, *comp);
}

osm_resolver::~osm_resolver()
{
  delete OP; 
  delete comp;
}

unsigned osm_resolver::invoke_specific(unsigned root)
{
  OP->set_levels(topLevel(), botLevel());
  return OP->apply(root);
}

resolver* one_side_match_mt(bdd_forest &BDD, compute_table &CT)
{
  return new osm_resolver(BDD, CT);
}


