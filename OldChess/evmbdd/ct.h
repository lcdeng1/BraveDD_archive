
#ifndef CT_H
#define CT_H

#include "defines.h"

class operation;

class compute_table {
  public:
    compute_table(unsigned size); 
    ~compute_table();

    // Note: op = 0 is reserved for CT use.
    unsigned char register_op(operation* op);
    void unregister_op(operation* op);

    void remove_stales(unsigned short seqno);

    // For unary operations

    bool find_unary(unsigned char op, unsigned P, unsigned &ans) const;
    void add_unary(unsigned char op, unsigned P, unsigned ans);

    // For binary operations,
    // or sequenced unary operations.

    bool find_binary(unsigned char op, unsigned P, unsigned Q, unsigned &ans) const;
    void add_binary(unsigned char op, unsigned P, unsigned Q, unsigned ans);

  private:

    // For unary operations

    inline bool equals(unsigned H, unsigned char op, unsigned P) const
    {
      return (oparray[H] == op) && (Parray[H] == P);
    }

    inline void set(unsigned H, unsigned char op, unsigned P, unsigned R)
    {
      ASSERT(H<size);
      oparray[H] = op;
      Parray[H] = P;
      Rarray[H] = R;
    }

    // For binary operations

    inline bool equals(unsigned H, unsigned char op, unsigned P, unsigned Q) const
    {
      return (oparray[H] == op) && (Parray[H] == P) && (Qarray[H] == Q);
    }

    inline void set(unsigned H, unsigned char op, unsigned P, unsigned Q, unsigned R)
    {
      ASSERT(H<size);
      oparray[H] = op;
      Parray[H] = P;
      Qarray[H] = Q;
      Rarray[H] = R;
    }

  private:
    unsigned size;

    unsigned char* oparray; 
    unsigned* Parray;
    unsigned* Qarray;
    unsigned* Rarray;

    operation* optable[256];
    // Status of each operation slot
    //  0: unused
    //  1: registered and active 
    //  2: unregistered, but not removed yet
    char opstatus[256]; 
    
};

#endif

