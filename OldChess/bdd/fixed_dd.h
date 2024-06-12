#ifndef FIXED_DD
#define FIXED_DD
#define WITH_SWAPS
#define DEBUG

/**
  Class for reading and querying BDDs that won't change.
*/

class fixed_dd {
  public:
    fixed_dd();
    ~fixed_dd();

    /**
      Read a BDD from a file.
        @param  pathname    File to read from.  Input files may be
                            compressed with the following:
                              .gz, .bz2, .xz

        @return   A handle to the root node in the file.
    */
    void add_nodes_from_file(const char* pathname, unsigned &root, bool &swap, short &shift);

    /**
      Indicates that we are done building, and
      all remaining operations will be read-only queries.
      This allows us to free some data structures
      used during building (like the unique table).
    */
    void done_building();

    /**
      Evaluate a function for given variable assignments.
        @param  f_root  Handle to the root node of the function
        @param  vars    variable assignments, where vars[i]
                        indicates the assignment to variable i.
                        vars should be dimension 1+levels or higher.
    */
    unsigned evaluate(unsigned f_root, bool fswap, short fshift, const bool* vars) const;

    inline unsigned num_levels() const { return levels; }
    inline unsigned num_terminals() const { return terminals; }
    inline unsigned num_nonterminals() const { return next_node - terminals; }
    inline unsigned num_file_nodes() const { return file_nodes; }

		inline unsigned long bytes_required() const {
			unsigned long nb = num_nonterminals();
			nb *= sizeof(unsigned char) + 2*sizeof(unsigned);
			return nb;
		}

  private:
    unsigned add_unique(unsigned char level, unsigned child0, bool swap0, short shift0, unsigned child1, bool swap1, short shift1);
    void enlarge_ut(unsigned req_max);
  private:
    unsigned char levels;
    unsigned terminals;
    unsigned next_node;
    unsigned max_nodes;

    // nodes in last file read
    unsigned file_nodes;

    // Node storage
    unsigned char* node_level;
    unsigned* node_child0;
    unsigned* node_child1;

    short* node_shift0;
    short* node_shift1;

    // Unique table
    unsigned* ut;
    unsigned* ut_next;
};

#endif
