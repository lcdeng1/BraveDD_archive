
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
    bool apply(bdd_edge P, bdd_edge Q);
};

has_common_op::has_common_op(bdd_forest &bdd, compute_table &ct)
 : operation(bdd, ct, "has_common")
{
  set_answer_other();
  set_binary_op();
}

bool has_common_op::apply(bdd_edge P, bdd_edge Q)
{
  if (P.node == Q.node && P.edge == Q.edge) return true;
  if (P.node == 0) return true;
  if (Q.node == 0) return true;
  if (is_terminal(P.node) && is_terminal(Q.node)) return false;

  if (P.node > Q.node) {
    // Deal with 'almost' commutativity
    bdd_edge T = P;
    P = Q;
    Q = T;
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

  answer = (cmp0 && cmp1) ? 1 : 0;

  return answer;
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
    bdd_edge apply(bdd_edge P, bdd_edge Q);
};

common_op::common_op(bdd_forest &bdd, compute_table &ct)
 : operation(bdd, ct, "common")
{
  set_answer_node();
  set_binary_op();
}

bdd_edge common_op::apply(bdd_edge P, bdd_edge Q)
{
  if (P.node == Q.node && P.edge == Q.edge) return P;
  if (P.node == 0) return Q;
  if (Q.node == 0) return P;
  if (is_terminal(P.node) && is_terminal(Q.node)) return newEdge(0,0);   

  if (P.node > Q.node) {
    // Deal with 'almost' commutativity
    bdd_edge T = P;
    P = Q;
    Q = T;
  }

  bdd_node tmp;
#ifdef WITH_SWAPS
  tmp.child[0].swap = false;
  tmp.child[1].swap = false;
#endif
  unsigned p1;
  unsigned q1;
  char a1;
  char b1;

  unsigned p2;
  unsigned q2;
  char a2;
  char b2;

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
      tmp.level = node_level(Q.node);
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
      tmp.level = node_level(P.node);
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
      tmp.level = node_level(P.node);
  }

  
  tmp.child[0] = apply(newEdge(a1, p1), newEdge(b1, q1));
  tmp.child[1] = apply(newEdge(a2, p2), newEdge(b2, q2));
      
  return newEdge(u_edge, add_node(tmp));
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
    bdd_edge apply(bdd_edge P, bdd_edge Q);
};

common_op::common_op(bdd_forest &bdd, compute_table &ct)
 : operation(bdd, ct, "common")
{
  set_answer_other();
  set_binary_op();
}

bdd_edge common_op::apply(bdd_edge P, bdd_edge Q)
{
  if (P.node == Q.node && P.edge == Q.edge) return newEdge(P.edge, P.node+1);
  if (P.node == 0) return newEdge(Q.edge, Q.node+1);
  if (Q.node == 0) return newEdge(P.edge, P.node+1);
  if (is_terminal(P.node) && is_terminal(Q.node)) return newEdge(0,0);   

  if (P.node > Q.node) {
    // Deal with 'almost' commutativity
    bdd_edge T = P;
    P = Q;
    Q = T;
    swap = true;
  } 

  bdd_node tmp;
#ifdef WITH_SWAPS
  tmp.child[0].swap = false;
  tmp.child[1].swap = false;
#endif
  unsigned p1;
  unsigned q1;
  char a1;
  char b1;

  unsigned p2;
  unsigned q2;
  char a2;
  char b2;

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
      tmp.level = node_level(Q.node);
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
      tmp.level = node_level(P.node);
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
      tmp.level = node_level(P.node);
  }

  tmp.child[0] = apply(newEdge(a1, p1), newEdge(b1, q1));
  tmp.child[1] = apply(newEdge(a2, p2), newEdge(b2, q2));
      

  if (0==tmp.child[0].node || 0==tmp.child[1].node) return newEdge(0,0);
  --tmp.child[0].node;
  --tmp.child[1].node;

  return newEdge(u_edge, 1+add_node(tmp));
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

    bdd_edge apply(bdd_edge P);
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

bdd_edge tsm_op::apply(bdd_edge root)
{
  if (is_terminal(root.node)) return root;
  if (node_level(root.node) < botL) return root;


#ifdef OLD
  if (COMP.apply(newEdge(node_edge0(root.node)+root.edge, node_child0(root.node)), newEdge(node_edge1(root.node)+root.edge, node_child1(root.node)))) {
    bdd_edge c_edge = COMMON.apply(newEdge(node_edge0(root.node)+root.edge, node_child0(root.node)), newEdge(node_edge1(root.node)+root.edge, node_child1(root.node)));
    return apply(c_edge);
  }
#else
  bdd_edge c_edge = COMMON.apply(newEdge(node_edge0(root.node)+root.edge, node_child0(root.node)), newEdge(node_edge1(root.node)+root.edge, node_child1(root.node)));
  unsigned C = c_edge.node;
  if (C) return newEdge(c_edge.edge, C-1);
#endif

  bdd_node tmp;
#ifdef WITH_SWAPS
  tmp.child[0].swap = false;
  tmp.child[1].swap = false;
#endif
  unsigned edge = 0;
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

class tsm_resolver : public resolver {
#ifdef OLD
    has_common_op* comp;
#endif
    common_op* common;
    tsm_op* OP;
  public:
    tsm_resolver(bdd_forest &BDD, compute_table &ct);
    virtual ~tsm_resolver();

    virtual unsigned invoke_specific(bdd_edge &root);
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

unsigned tsm_resolver::invoke_specific(bdd_edge &root)
{
  OP->set_levels(topLevel(), botLevel());
  return (OP->apply(root)).node;
}

resolver* two_side_match_mt(bdd_forest &BDD, compute_table &CT)
{
  return new tsm_resolver(BDD, CT);
}


