
#ifndef BDD_H
#define BDD_H

#define WITH_SWAPS

#include "defines.h"

#include <iostream>

class compute_table;

struct bdd_edge {
    unsigned node;
    char edge;
#ifdef WITH_SWAPS
    bool swap;
#endif

};

struct bdd_node {
    unsigned short level;			// 0 for terminals and freed nodes
    unsigned next;  // next free, or next in hash table
    unsigned short sequence;	// for mark and sweep
    bdd_edge child[2];

    inline void copy(const bdd_node &N)
    {
        child[0].edge = N.child[0].edge;
        child[0].node = N.child[0].node;
        child[1].edge = N.child[1].edge;
        child[1].node = N.child[1].node;
#ifdef WITH_SWAPS
        child[0].swap = N.child[0].swap;
        child[1].swap = N.child[1].swap;
#endif
        level = N.level;

    }

    inline bool is_duplicate(const bdd_node &N)
    {
      if (level != N.level) return false;
      if (child[0].node != N.child[0].node) return false;
      if (child[1].node != N.child[1].node) return false;
      if (child[0].edge != N.child[0].edge) return false;

#ifdef WITH_SWAPS
      if (child[1].swap != N.child[1].swap) return false;
      if (child[0].swap != N.child[0].swap) return false;
#endif

      return (child[1].edge == N.child[1].edge);

    }

    inline void recycle(unsigned free_list) 
    {
      level = 0;
      next = free_list; 
    }

    static inline void hash(unsigned &H, unsigned short x) 
    {
      H = 37 * H ^ (x >> 8);
      H = 37 * H ^ (x & 0x00ff);
    }
    static inline void hash(unsigned &H, unsigned x)
    {
      H = 37 * H ^ (x >> 24);
      H = 37 * H ^ ((x & 0x00ff0000) >> 16);
      H = 37 * H ^ ((x & 0x0000ff00) >> 8);
      H = 37 * H ^  (x & 0x000000ff);
    }
    inline unsigned hash() const
    {
      unsigned h = 0;
      hash(h, level);
      hash(h, child[0].node + child[0].edge);
      hash(h, child[1].node + child[1].edge);
      return h;
    }
};

class bdd_forest {
  public:
    /**
      Initialize a forest with the given number of variables and terminals.
      Terminal values are 0, 1, ..., terminals-1.
      The map array specifies the level for each input variable.
      I.e., map[1] gives the level for variable 1, map[2] gives
      the level for variable 2, etc.  map[0] will be set to 0.
    */
    bdd_forest(unsigned short nvars, unsigned* map, unsigned char terminals=2);

    /**
      Destroy the forest
    */
    ~bdd_forest();

    inline unsigned peak_nodes() const {
      return max_next_node - num_terminals;
    }

    inline unsigned skip_nodes() const {
        return skipped_nodes;
    }

    inline unsigned short get_num_vars() const {
      return num_vars;
    }

    inline unsigned short get_mod_val() const {
        return modVal;
    }

    inline unsigned get_num_terminals() const {
      return num_terminals;
    }

    static inline unsigned char2terminal(char t)
    {
      return unsigned(t - '0');
    }

    static inline char terminal2char(unsigned v)
    {
      return char('0' + v);
    }

    /**
      Convert from variable array to level array.
        @param  input   input[k] is the value for variable k+1.
        @param  output  output[k] is the value for level k.
    */
    inline void varray2minterm(const char* input, char* output) const
    {
      for (short b=0; b<num_vars; b++) {
        output[map[b+1]] = input[b];
      }
    }

    /**
      Convert from level array to variable array.
        @param  input   input[k] is the value for level k.
        @param  output  output[k] is the value for variable k+1.
    */
    inline void minterm2varray(const char* input, char* output) const
    {
      for (short b=0; b<num_vars; b++) {
        output[b] = input[map[b+1]];
      }
    }

     
    /**
      Evaluate a function for given variable assignments, as a minterm.
    */

#ifdef WITH_SWAPS
    inline void apply_swap(bdd_edge root) {
        apply_swap(&root, 1);
    }

    void apply_swap(bdd_edge* roots, unsigned len);
     bdd_edge apply_swap_rec(bdd_edge& root);

#endif
    

    inline char evaluate(bdd_edge root, const char* minterm) const
    {
        using namespace std;
        unsigned val = 0;
        unsigned dir;
        bdd_node& node = node_array[0];
        char minterm_var;

        for (;;){
            val += root.edge;

              if (root.node == 0) {
                // This means infty, since 6 will not happen for chess endgame cases
                return '6';
              } else if (root.node == 1) {
                return val % modVal + 1 + '0';
              }


          const bdd_node& node = node_array[root.node];
          minterm_var = minterm[node.level];
          ASSERT('0' == minterm_var || '1' == minterm_var);

          // what if skipping levels? still work!
#ifndef WITH_SWAPS
          dir = node.child[minterm_var - '0'];
#else
          dir = (minterm_var - '0') ^ (root.swap);
#endif
          root = node.child[dir];
        }

    }

    /**
      Find or add a node in the BDD.
    */
    unsigned add_node(const bdd_node &N);

    /**
      Given a node handle, return the node struct.
    */
    inline const bdd_node& get_node(unsigned handle) const 
    {
      ASSERT(handle < next_node);
      return node_array[handle];
    }

    /**
      Given a node handle, return the node's level.
    */
    inline unsigned short node_level(unsigned handle) const
    {
      ASSERT(handle < next_node);
      return node_array[handle].level;
    }

    /**
      Given a node handle, return the node's low child.
    */
    inline unsigned node_child0(unsigned handle) const
    {
      ASSERT(handle < next_node);
      return node_array[handle].child[0].node;
    }

    /**
      Given a node handle, return the node's high child.
    */
    inline unsigned node_child1(unsigned handle) const
    {
      ASSERT(handle < next_node);
      return node_array[handle].child[1].node;
    }

    /* 
    * Given a node handle, return the node's high edge.
    */
    inline char node_edge0(unsigned handle) const
    {
        ASSERT(handle < next_node);
        return node_array[handle].child[0].edge;
    }

    /**
      Given a node handle, return the node's high edge.
    */
    inline char node_edge1(unsigned handle) const
    {
        ASSERT(handle < next_node);
        return node_array[handle].child[1].edge;
    }

#ifdef WITH_SWAPS
    /*
    * Given a node handle, return the node's high edge.
    */
    inline bool node_swap0(unsigned handle) const
    {
        ASSERT(handle < next_node);
        return node_array[handle].child[0].swap;
    }

    /**
      Given a node handle, return the node's high edge.
    */
    inline char node_swap1(unsigned handle) const
    {
        ASSERT(handle < next_node);
        return node_array[handle].child[1].swap;
    }
#endif

    /*
       reduces an unreduced EV+BDD into a quasi-reduced EV+BDD
    */
    /*unsigned reduce(unsigned root);*/

    /**
      Garbage collection.  Keeps the root node and all terminals;
      anything unreachable is recycled.
      Returns the number of reachable non-terminal nodes, including roots.
    */
    unsigned mark_and_sweep(bdd_edge* roots, unsigned len);

    inline unsigned mark_and_sweep(bdd_edge root)
    {
      return mark_and_sweep(&root, 1);
    }


    /**
      Build a new BDD handle as the union of the given root,
      and a list of minterms.  Each minterm is a string of length
      number of levels + 1, with minterm[0] as the terminal value,
      and minterm[i] the value at level i, as characters
      (use char2terminal, terminal2char methods for terminals).

      TBD: allow '?' for don't care variables.

      The minterm array WILL be modified: the minterms will be
      reordered to optimize the union operation.
    */
    inline bdd_edge union_minterms(bdd_edge root, char** minterm, unsigned N)
    {
      //TEMP
      //REPLACING ROOT WITH 0 (BASE CASE) RETURNS BASE CASE
      //this means that this is where the union of trees happens . . . 
      //return union_rec(num_vars, root, minterm, N);

        /*unsigned max = 0;
        unsigned m;
        for (unsigned i = 0; i < N; i++) {
            m = char2terminal(minterm[i][0]) - 1;
            if (m > max) max = m;
        }*/

        bdd_edge new_edge = union_rec(num_vars, 0, minterm, N);
        bdd_edge union_edge = UnionMin(new_edge, root);

        return union_edge;
    }

    bdd_edge newEdge(char edge, unsigned node) {
        return newEdge(edge, node, false);
    }

    bdd_edge newEdge(char edge, unsigned node, bool swap) {
        bdd_edge new_edge;
        new_edge.node = node;
        new_edge.edge = edge;
#ifdef WITH_SWAPS
        new_edge.swap = swap;
#endif
        return new_edge;
    }

    unsigned min(unsigned a, unsigned b) {
        if (a < b) return a;
        return b;
    }

    unsigned max(unsigned a, unsigned b) {
        if (a > b) return a;
        return b;
    }

		/**
			Count nodes with a low or high edge directly to a terminal node.
			returns the total number of nodes.
		*/
		unsigned count_pattern(unsigned *low, unsigned *high) const;

    /**
      Display the entire forest, by levels.
    */
    void display(std::ostream &out) const;

    /**
      Write the given BDDs to a file.
    */
    void write_exch(std::ostream &out, bdd_edge* roots, unsigned len) const;

    inline void write_exch(std::ostream &out, bdd_edge root) const
    {
      write_exch(out, &root, 1);
    }

    /**
      Display minterms for the given function (root node).
        @param  out   Stream to send output to
        @param  root  Root node of the function
        @param  show_zeroes		Show minterms that lead to value 0
    */
    inline void show_minterms(std::ostream &out, unsigned root, bool show_zeroes=false) 
    {
      _minterm = new char[num_vars+2];
      _varray = new char[num_vars+1];
      _varray[num_vars] = 0;
      for (short i=0; i<=num_vars; i++) _minterm[i] = '-';
      minterms_rec(out, root, show_zeroes);
      delete[] _varray;
      delete[] _minterm;
      _varray = 0;
      _minterm = 0;
    }

    
    // void build_parent_lists(bool show);

    inline void set_CT(compute_table* ct) {
      CT = ct;
    }

    bdd_edge Normalize (bdd_node u);

  private:
    inline void recycle(unsigned handle)
    {
      ASSERT(handle >= num_terminals); 
      if (next_node - 1 == handle) {
        --next_node;
      } else {
        node_array[handle].recycle(free_nodes);
        free_nodes = handle;
      }
    }
    
    void mark(unsigned root); 
    unsigned get_unused_handle();
    void enlarge_node_array();
    void hash_remove(unsigned handle);

    bdd_edge union_rec(unsigned short level, unsigned root, char** minterm, unsigned N);

    

    bdd_edge UnionMin(bdd_edge edge_1, bdd_edge edge_2);

    void minterms_rec(std::ostream &out, unsigned root, bool show_zeroes) const;

//    unsigned redundant_rec(unsigned root);

  private:
    unsigned short modVal;
    unsigned short num_vars;
    unsigned char num_terminals;
    unsigned short msround;
    unsigned skipped_nodes;
    bool swapped;

    bdd_node* node_array;
    unsigned next_node;
    unsigned max_next_node;
    unsigned max_nodes;
    unsigned free_nodes;

    unsigned* UT;
    unsigned ut_size;
    unsigned ut_elements;

    unsigned* unary_cache;

    compute_table* CT;  // used to synch garbage collection

    unsigned* map;  // variable to level mapping.

    // For minterms_rec
    char* _minterm;
    char* _varray;
};

#endif
