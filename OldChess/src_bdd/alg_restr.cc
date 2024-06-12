
#include "bdd.h"
#include "ct.h"
#include "operation.h"
#include "resolv.h"

// #define TRACE

class restrict_op : public operation {
    unsigned sequence;
    unsigned short topL;
    unsigned short botL;
  public:
    restrict_op(bdd_forest &bdd, compute_table &ct, unsigned UNK);

    inline void set_levels(unsigned short toplevel, unsigned short botlevel)
    {
      topL = toplevel;
      botL = botlevel;
      sequence = (unsigned(topL) << 16) | botL;
    }

  public:
    unsigned apply(unsigned P);

    const unsigned UNKNOWN;
};

restrict_op::restrict_op(bdd_forest &bdd, compute_table &ct, unsigned UNK)
 : operation(bdd, ct, "restrict_mt"), UNKNOWN(UNK)
{
  set_unary_op();
  set_answer_node();
  set_levels(static_cast <unsigned short>(bdd.get_num_vars()), 0);
}

unsigned restrict_op::apply(unsigned root)
{
  if (is_terminal(root)) return root;

  if (node_level(root) < botL) return root;

  unsigned ans;
  if (has_sUCT_entry(root, sequence, ans)) return ans;

  if (node_level(root) <= topL) {
    if (node_child0(root) == UNKNOWN)
    {
#ifdef TRACE
      ans = apply(node_child1(root));
      printf("apply(%u) : apply(%u, %u, %u) : %u\n", root, 
        node_level(root), node_child0(root), node_child1(root), ans);
      return ans;
#else
      return add_sUCT_entry(root, sequence, apply(node_child1(root)));
#endif
    }

    if (node_child1(root) == UNKNOWN)
    {
#ifdef TRACE
      ans = apply(node_child0(root));
      printf("apply(%u) : apply(%u, %u, %u) : %u\n", root, 
        node_level(root), node_child0(root), node_child1(root), ans);
      return ans; 
#else
      return add_sUCT_entry(root, sequence, apply(node_child0(root)));
#endif
    }
  }

  bdd_node tmp;
  tmp.level = node_level(root);
  tmp.child[0] = apply(node_child0(root));
  tmp.child[1] = apply(node_child1(root));
#ifdef WITH_SWAPS
  tmp.swap[0] = false;
  tmp.swap[1] = false;
#endif

#ifdef TRACE
  ans = add_node(tmp);
  printf("apply(%u) : apply((%u, %u, %u) : (%u, %u, %u) : %u \n", root, 
    node_level(root), node_child0(root), node_child1(root), tmp.level, tmp.child[0], tmp.child[1], ans);

  return add_sUCT_entry(root, sequence, ans);
#else
  return add_sUCT_entry(root, sequence, add_node(tmp));
#endif
}


/*
  New front end
*/

class restrict_resolver : public resolver {
    restrict_op* OP;
  public:
    restrict_resolver(bdd_forest &BDD, compute_table &ct);
    virtual ~restrict_resolver();

    virtual unsigned invoke_specific(unsigned root);
};

restrict_resolver::restrict_resolver(bdd_forest &bdd, compute_table &ct)
 : resolver(bdd, ct)
{
  OP = new restrict_op(BDD, ct, 0);
}

restrict_resolver::~restrict_resolver()
{
  delete OP; 
}

unsigned restrict_resolver::invoke_specific(unsigned root)
{
  OP->set_levels(topLevel(), botLevel());
  return OP->apply(root);
}

resolver* restrict_mt(bdd_forest &BDD, compute_table &CT)
{
  return new restrict_resolver(BDD, CT);
}

