
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
    restrict_op(bdd_forest& bdd, compute_table& ct, unsigned UNK);

    inline void set_levels(unsigned short toplevel, unsigned short botlevel)
    {
        topL = toplevel;
        botL = botlevel;
        sequence = (unsigned(topL) << 16) | botL;
    }

public:
    bdd_edge apply(bdd_edge P);

    const unsigned UNKNOWN;
};

restrict_op::restrict_op(bdd_forest& bdd, compute_table& ct, unsigned UNK)
    : operation(bdd, ct, "restrict_mt"), UNKNOWN(UNK)
{
    set_unary_op();
    set_answer_node();
    set_levels(static_cast <unsigned short>(bdd.get_num_vars()), 0);
}

bdd_edge restrict_op::apply(bdd_edge root)
{
    if (is_terminal(root.node)) return root;

    if (node_level(root.node) < botL) return root;

    // unsigned ans;
    // if (has_sUCT_entry(root.node, sequence, ans)) return ans;

    if (node_level(root.node) <= topL) {
        if (node_child0(root.node) == 0 && node_edge1(root.node) == 0 && node_child1(root.node) != 0)
        {
#ifdef TRACE
            ans = apply(newEdge(node_edge1(root.node), node_child1(root.node)));
            printf("apply(%u) : apply(%u, %u, %u) : %u\n", root,
                node_level(root.node), node_child0(root.node), node_child1(root.node), ans);
            return ans;
#else
            return apply(newEdge(node_edge1(root.node), node_child1(root.node)));
#endif
        }

        if (node_child1(root.node) == 0 && node_edge0(root.node) == 0 && node_child0(root.node) != 0)
        {
#ifdef TRACE
            ans = apply(newEdge(node_edge0(root.node), node_child0(root.node)));
            printf("apply(%u) : apply(%u, %u, %u) : %u\n", root,
                node_level(root.node), node_child0(root.node), node_child1(root.node), ans);
            return ans;
#else
            return apply(newEdge(node_edge0(root.node), node_child0(root.node)));
#endif
        }
    }

    bdd_node tmp;
#ifdef WITH_SWAPS
    tmp.child[0].swap = false;
    tmp.child[1].swap = false;
#endif
    tmp.copy(get_node(root.node));

    tmp.level = node_level(root.node);

    tmp.child[0] = apply(newEdge(node_edge0(root.node), node_child0(root.node)));
    tmp.child[1] = apply(newEdge(node_edge1(root.node), node_child1(root.node)));
    tmp.child[0].edge = node_edge0(root.node);
    tmp.child[1].edge = node_edge1(root.node);
#ifdef TRACE
    ans = newEdge(root.edge, add_node(tmp));
    printf("apply(%u) : apply((%u, %u, %u) : (%u, %u, %u) : %u \n", root,
        node_level(root.node), node_child0(root.node), node_child1(root), tmp.level, tmp.child[0], tmp.child[1], ans);

    return ans;
#else
    return newEdge(root.edge, add_node(tmp));
#endif
}


/*
  New front end
*/

class restrict_resolver : public resolver {
    restrict_op* OP;
public:
    restrict_resolver(bdd_forest& BDD, compute_table& ct);
    virtual ~restrict_resolver();

    virtual unsigned invoke_specific(bdd_edge& root);
};

restrict_resolver::restrict_resolver(bdd_forest& bdd, compute_table& ct)
    : resolver(bdd, ct)
{
    OP = new restrict_op(BDD, ct, 0);
}

restrict_resolver::~restrict_resolver()
{
    delete OP;
}

unsigned restrict_resolver::invoke_specific(bdd_edge& root)
{
    OP->set_levels(topLevel(), botLevel());
    return (OP->apply(root)).node;
}

resolver* restrict_mt(bdd_forest& BDD, compute_table& CT)
{
    return new restrict_resolver(BDD, CT);
}

