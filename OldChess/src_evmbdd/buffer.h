
#ifndef BUFFER_H
#define BUFFER_H

class bdd_forest;
class bdd_edge;
class resolver;

class minterm_buffer {
    bdd_forest &BDD;
    bdd_edge &root;

    char** statebuf;
    const unsigned bufsize;
    unsigned bufptr;
    unsigned long minterms;

    resolver* R;
    const char* last_add;

  public:
    minterm_buffer(bdd_forest &bdd, bdd_edge &root, unsigned bufsize,
      resolver* R);

    ~minterm_buffer();

    /// Returns true if we had to flush the buffer.
    bool add(const char* linebuf, char term);
    void flush();

    inline unsigned long get_num_minterms() const { return minterms; }

  private:
    unsigned short top_changed(const char* state1, const char* state2) const;

};

#endif

