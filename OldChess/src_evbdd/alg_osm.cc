
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

  unsigned p1;
  unsigned q1;
  char a1;
  char b1;

  unsigned p2;
  unsigned q2;
  char a2;
  char b2;


  unsigned cmp0, cmp1;

  char u_edge = min(P.edge, Q.edge);

  if (node_level(P.node) < node_level(Q.node)) {
      a1 = P.edge - u_edge;
      b1 = Q.edge - u_edge + node_edge0(Q.node);
      p1 = P.node;
      q1 = node_child0(Q.node);

      a2 = P.edge - u_edge;
      b2 = Q.edge - u_edge + node_edge1(Q.node);
      p2 = P.node;
      q2 = node_child1(Q.node);
  }
  else if (node_level(P.node) > node_level(Q.node)) {
      a1 = P.edge - u_edge + node_edge0(P.node);
      b1 = Q.edge - u_edge;
      p1 = node_child0(P.node);
      q1 = Q.node;

      a2 = P.edge - u_edge + node_edge1(P.node);
      b2 = Q.edge - u_edge;
      p2 = node_child1(P.node);
      q2 = Q.node;
  }
  else {
      a1 = P.edge - u_edge + node_edge0(P.node);
      b1 = Q.edge - u_edge + node_edge0(Q.node);
      p1 = node_child0(P.node);
      q1 = node_child0(Q.node);

      a2 = P.edge - u_edge + node_edge1(P.node);
      b2 = Q.edge - u_edge + node_edge1(Q.node);
      p2 = node_child1(P.node);
      q2 = node_child1(Q.node);
  }

  cmp0 = apply(newEdge(a1, p1), newEdge(b1, q1));
  cmp1 = apply(newEdge(a2, p2), newEdge(b2, q2));

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
      // child 0 -> child 1
      return apply(newEdge(node_edge1(root.node)+root.edge, node_child1(root.node)));
    }
    if ('<' == child_cmp) {
      // child 1 -> child 0
      return apply(newEdge(node_edge0(root.node)+root.edge, node_child0(root.node)));
    }
    ASSERT('=' != child_cmp); // should be impossible
  }

  bdd_node tmp;
#ifdef WITH_SWAPS
  tmp.child[0].swap = false;
  tmp.child[1].swap = false;
#endif
  tmp.copy(get_node(root.node));
  
  tmp.level = node_level(root.node);
  
  tmp.child[0] = apply(tmp.child[0]);
  tmp.child[1] = apply(tmp.child[1]);
  edge = min(tmp.child[0].edge, tmp.child[1].edge);
  tmp.child[0].edge -= edge;
  tmp.child[1].edge -= edge;

  return newEdge(root.edge+edge, add_node(tmp));
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


