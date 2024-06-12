
#ifndef OPERATION_H
#define OPERATION_H

#include "bdd.h"
#include "ct.h"

class operation {
    const char* name;
    compute_table &CT;
    bdd_forest &BDD;
    bool answer_is_node;
    bool binary_op;
    unsigned char OPCODE;
  public:
    operation(bdd_forest &bdd, compute_table &ct, const char* n);
    ~operation();

  public:

    inline const char* get_name() const { return name; }
    inline unsigned char get_opcode() const { return OPCODE; }

    inline bool entry_is_stale(unsigned short seqno, unsigned P, unsigned Q, 
      unsigned ans) const
    {
      if (BDD.get_node(P).sequence != seqno) return true;
      if (binary_op) {
        if (BDD.get_node(Q).sequence != seqno) return true;
      }
      if (answer_is_node) {
        return BDD.get_node(ans).sequence != seqno;
      } else {
        return false;
      }
    }

  protected:
    void set_answer_node() {
      answer_is_node = true;
    }
    void set_answer_other() {
      answer_is_node = false;
    }
    void set_binary_op() {
      binary_op = true;
    }
    void set_unary_op() {
      binary_op = false;
    }

    inline unsigned add_node(const bdd_node &N) 
    {
      return BDD.add_node(N);
    }

    inline unsigned short node_level(unsigned handle) const
    {
      return BDD.node_level(handle);
    }

    inline unsigned node_child0(unsigned handle) const
    {
      return BDD.node_child0(handle);
    }

    inline unsigned node_child1(unsigned handle) const
    {
      return BDD.node_child1(handle);
    }

    inline bool is_terminal(unsigned P) const
    {
      return P < BDD.get_num_terminals();
    }

    inline bool has_BCT_entry(unsigned P, unsigned Q, unsigned &ans) const {
      ASSERT(binary_op);
      return CT.find_binary(OPCODE, P, Q, ans);
    }

    inline unsigned add_BCT_entry(unsigned P, unsigned Q, unsigned ans) {
      ASSERT(binary_op);
      CT.add_binary(OPCODE, P, Q, ans);
      return ans;
    }

    inline bool has_UCT_entry(unsigned P, unsigned &ans) const {
      ASSERT(!binary_op);
      return CT.find_unary(OPCODE, P, ans);
    }

    inline unsigned add_UCT_entry(unsigned P, unsigned ans) {
      ASSERT(!binary_op);
      CT.add_unary(OPCODE, P, ans);
      return ans;
    }

    inline bool has_sUCT_entry(unsigned P, unsigned seq, unsigned &ans) const {
      ASSERT(!binary_op);
      return CT.find_binary(OPCODE, P, seq, ans);
    }

    inline unsigned add_sUCT_entry(unsigned P, unsigned seq, unsigned ans) const {
      ASSERT(!binary_op);
      CT.add_binary(OPCODE, P, seq, ans);
      return ans;
    }
};



#endif

