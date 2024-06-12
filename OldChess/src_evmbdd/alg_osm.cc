
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

    unsigned char apply(bdd_edge P, bdd_edge Q);

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

unsigned char compare_op::apply(bdd_edge P, bdd_edge Q)
{
  if (P.node == Q.node && P.edge == Q.edge) return '=';
  if (P.node == 0) return '>';
  if (Q.node == 0) return '<';
  if (is_terminal(P.node) && is_terminal(Q.node)) return '!';

  bool swap = false;
  if (P.node > Q.node) {
    // Deal with 'almost' commutativity
    bdd_edge T = P;
    P = Q;
    Q = T;
    swap = true;
  } 
  unsigned answer;
  unsigned cmp0, cmp1;

  char a, b;
  bdd_edge ue1, ue2;

  // set u_node level
    unsigned modVal = 5;
  // tmp.level = max(node_level(P.node), node_level(Q.node));

  bool f1 = (node_level(P.node) == node_level(Q.node) || node_level(P.node) > node_level(Q.node));
  bool f2 = (node_level(P.node) == node_level(Q.node) || node_level(P.node) < node_level(Q.node));
  
  // recursion for low edge
  a = (P.edge + (f1? node_edge0(P.node) : 0)) % modVal;
  b = (Q.edge + (f2? node_edge0(Q.node) : 0)) % modVal;
  
  ue1 = newEdge(a, f1? node_child0(P.node) : P.node);
  ue2 = newEdge(b, f2? node_child0(Q.node) : Q.node);

  cmp0 = apply(ue1, ue2);

  // recursion for high edge
  a = (P.edge + (f1? node_edge1(P.node) : 0)) % modVal;
  b = (Q.edge + (f2? node_edge1(Q.node) : 0)) % modVal;
  
  ue1 = newEdge(a, f1? node_child1(P.node) : P.node);
  ue2 = newEdge(b, f2? node_child1(Q.node) : Q.node);
  
  cmp1 = apply(ue1, ue2);

  if (('=' == cmp0) || (cmp0 == cmp1)){
    answer = cmp1;
  } else {
    answer = '!';
  }

  return flip_comparison(swap, static_cast<unsigned char>(answer));;
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

    bdd_edge apply(bdd_edge P);
};

osm_op::osm_op(bdd_forest &bdd, compute_table &ct, compare_op &comp)
 : operation(bdd, ct, "osm_mt"), COMP(comp)
{
  set_unary_op();
  set_answer_node();
}

bdd_edge osm_op::apply(bdd_edge root)
{
  if (is_terminal(root.node)) {
    return root;
  }

  if (node_level(root.node) < botL) return root;
  unsigned edge;

  if (node_level(root.node) <= topL) {
    unsigned char child_cmp = COMP.apply(newEdge(node_edge0(root.node)+root.edge, node_child0(root.node)), newEdge(node_edge1(root.node)+root.edge, node_child1(root.node)));
    if ('>' == child_cmp) {
      root.edge = node_edge1(root.node);
      // child 0 -> child 1
      return apply(newEdge(node_edge1(root.node)+root.edge, node_child1(root.node)));
    }
    if ('<' == child_cmp) {
      // child 1 -> child 0
      root.edge = node_edge0(root.node);
      return apply(newEdge(node_edge0(root.node)+root.edge, node_child0(root.node)));
    }
    ASSERT('=' != child_cmp); // should be impossible
  }

  bdd_node tmp;
  char modVal = 5;
#ifdef WITH_SWAPS
  tmp.child[0].swap = false;
  tmp.child[1].swap = false;
#endif
  tmp.copy(get_node(root.node));
  
  tmp.level = node_level(root.node);
  
  tmp.child[0] = apply(tmp.child[0]);
  tmp.child[1] = apply(tmp.child[1]);
  char pushVal;
  pushVal = tmp.child[0].edge;
  tmp.child[0].edge = 0;
  tmp.child[1].edge = ((modVal + tmp.child[1].edge) - pushVal) % modVal;

  return newEdge((root.edge + pushVal) % modVal, add_node(tmp));
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

    virtual unsigned invoke_specific(bdd_edge &root);
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

unsigned osm_resolver::invoke_specific(bdd_edge &root)
{
  OP->set_levels(topLevel(), botLevel());
  return (OP->apply(root)).node;
}

resolver* one_side_match_mt(bdd_forest &BDD, compute_table &CT)
{
  return new osm_resolver(BDD, CT);
}


