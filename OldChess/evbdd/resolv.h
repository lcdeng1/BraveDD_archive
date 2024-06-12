
#ifndef RESOLV_H
#define RESOLV_H

class bdd_forest;
class compute_table;
class bdd_edge;

class resolver {
  public:
    resolver(bdd_forest &BDD, compute_table &ct);
    virtual ~resolver();

    inline void set_levels(unsigned short top, unsigned short bot)
    {
      toplevel = top;
      botlevel = bot;
    }

    inline unsigned short topLevel() const { return toplevel; }
    inline unsigned short botLevel() const { return botlevel; }

    inline unsigned invoke(bdd_edge &root) {
      ++invoke_count;
      return invoke_specific(root);
    }

    inline unsigned num_calls() const {
      return invoke_count;
    }

  protected:
    virtual unsigned invoke_specific(bdd_edge &root) = 0;

  protected:
    // bdd_edge &root;
    bdd_forest &BDD;
    compute_table &CT;
  private:
    unsigned short toplevel;
    unsigned short botlevel;
    unsigned invoke_count;
};



// Implemented in alg_restr.cc
resolver* restrict_mt(bdd_forest &BDD, compute_table &CT);


// Implemented in alg_osm.cc
resolver* one_side_match_mt(bdd_forest &BDD, compute_table &CT);


// Implemeted in alg_tsm.cc
resolver* two_side_match_mt(bdd_forest &BDD, compute_table &CT);




#endif

