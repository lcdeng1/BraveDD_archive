
#ifndef BDD_H
#define BDD_H
#define WITH_SWAPS
#define WITH_SHIFTS

#include "defines.h"

#include <iostream>

class compute_table;

struct bdd_node {
    unsigned child[2];
    unsigned next;  // next free, or next in hash table
    unsigned short level;			// 0 for terminals and freed nodes
    unsigned short sequence;	// for mark and sweep
    #ifdef WITH_SWAPS
        bool swap[2];            // swap flags  
    #endif
    #ifdef WITH_SHIFTS
        unsigned short shift[2];
    #endif
  
    inline void copy(const bdd_node &N)
    {
      child[0] = N.child[0];
      child[1] = N.child[1];
      #ifdef WITH_SWAPS
        swap[0] = N.swap[0];
        swap[1] = N.swap[1];
      #endif
#ifdef WITH_SHIFTS
        shift[0] = N.shift[0];
        shift[1] = N.shift[1];
#endif
      level = N.level;
    }

    inline bool is_duplicate(const bdd_node &N)
    {
      if (level != N.level) return false;
      if (child[0] != N.child[0]) return false;
        #ifdef WITH_SWAPS
          if (swap[0] != N.swap[0]) return false;
          if (swap[1] != N.swap[1]) return false;
        #endif
#ifdef WITH_SHIFTS
          if (shift[0] != N.shift[0]) return false;
          if (shift[1] != N.shift[1]) return false;
#endif
      return child[1] == N.child[1];
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
      hash(h, child[0]);
      hash(h, child[1]);
#ifdef WITH_SHIFTS
      hash(h, shift[0]);
      hash(h, shift[1]);
#endif
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

    inline unsigned short get_num_vars() const {
      return num_vars;
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
    inline char evaluate(short shift, bool swap, unsigned root, const char* minterm) const
    {
        using namespace std;
        unsigned dir; 
#ifdef WITH_SHIFTS
        unsigned short curLev = num_vars;
#endif

        for (;;) {
            if (root < get_num_terminals()) {
                return terminal2char(root);
            }
            const bdd_node& node = node_array[root];
            char var;
#ifndef WITH_SHIFTS
            var = minterm[node.level];
#else
            if (shifted){
                curLev -= shift;
                var = minterm[curLev];
            }
            else {
                var = minterm[node.level];
            }
#endif

            ASSERT('0' == var || '1' == var);
#ifdef WITH_SWAPS
            dir = (var - '0') ^ (swap);
#else
            dir = (var - '0');
#endif
            root = node.child[dir];
#ifdef WITH_SWAPS
            swap = node.swap[dir];
#endif
#ifdef WITH_SHIFTS
            shift = node.shift[dir];
#endif
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
      return node_array[handle].child[0];
    }

    /**
      Given a node handle, return the node's high child.
    */
    inline unsigned node_child1(unsigned handle) const
    {
      ASSERT(handle < next_node);
      return node_array[handle].child[1];
    }

#ifdef WITH_SWAPS
    /**
        Given a node handle, return the node's low child.
    */
    inline bool swap_child0(unsigned handle) const
    {
        ASSERT(handle < next_node);
        return node_array[handle].swap[0];
    }

    /**
        Given a node handle, return the node's high child.
    */
    inline bool swap_child1(unsigned handle) const
    {
        ASSERT(handle < next_node);
        return node_array[handle].swap[1];
    }
#endif

#ifdef WITH_SHIFTS
    /**
        Given a node handle, return the node's low child.
    */
    inline short shift_child0(unsigned handle) const
    {
        ASSERT(handle < next_node);
        return node_array[handle].shift[0];
    }

    /**
        Given a node handle, return the node's high child.
    */
    inline short shift_child1(unsigned handle) const
    {
        ASSERT(handle < next_node);
        return node_array[handle].shift[1];
    }
#endif
    /**
      Garbage collection.  Keeps the root node and all terminals;
      anything unreachable is recycled.
      Returns the number of reachable non-terminal nodes, including roots.
    */
    unsigned mark_and_sweep(unsigned* roots, unsigned len);

    inline unsigned mark_and_sweep(unsigned root)
    {
      return mark_and_sweep(&root, 1);
    }

#ifdef WITH_SWAPS
    inline void apply_swap(unsigned root, bool swap) {
        apply_swap(&root, &swap, 1);
    }

    void apply_swap(unsigned* roots, bool* swaps, unsigned len);
#endif

#ifdef WITH_SHIFTS
    inline void apply_shift(unsigned root, short shift) {
        apply_shift(&root, &shift, 1);
    }

    void apply_shift(unsigned* roots, short* shifts, unsigned len);
#endif

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
    inline unsigned union_minterms(unsigned root, char** minterm, unsigned N)
    {
      return union_rec(num_vars, root, minterm, N);
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
    void write_exch(std::ostream& out, unsigned* roots, bool* swaps, short* shifts, unsigned len) const;

    inline void write_exch(std::ostream &out, unsigned root, bool swaps, short shifts) const
    {
      write_exch(out, &root, &swaps, &shifts, 1);
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

    unsigned union_rec(unsigned short level, unsigned root, char** minterm, unsigned N);

    void minterms_rec(std::ostream &out, unsigned root, bool show_zeroes) const;
#ifdef WITH_SWAPS
    unsigned apply_swap_rec(unsigned &root);
#endif

#ifdef WITH_SHIFTS
    unsigned apply_shift_rec(unsigned& root, unsigned inLevel);
#endif
//    unsigned redundant_rec(unsigned root);

  private:
    unsigned short num_vars;
    unsigned char num_terminals;
    unsigned short msround;
    bool swapped;
    bool shifted;

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
