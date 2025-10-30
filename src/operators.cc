#include "operators.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                             Operators                          *
// *                                                                *
// *                                                                *
// ******************************************************************
namespace BRAVE_DD {
    Func operator+(const Func &f1, const Func &f2)
    {
        Func out(f1.getForest());
        apply(PLUS, f1, f2, out);
        return out;
    }
    Func operator-(const Func &f1, const Func &f2)
    {
        Func out(f1.getForest());
        apply(MINUS, f1, f2, out);
        return out;
    }
    Func operator*(const Func &f1, const Func &f2){
        Func out(f1.getForest());
        apply(MULTIPLY, f1, f2, out);
        return out;
    }
    Func operator/(const Func &f1, const Func &f2)
    {
        Func out(f1.getForest());
        apply(DIVIDE, f1, f2, out);
        return out;
    }

    Func operator&(const Func &f1, const Func &f2)
    {
        Func out(f1.getForest());
        apply(INTERSECTION, f1, f2, out);
        return out;
    }
    Func operator|(const Func &f1, const Func &f2)
    {
        Func out(f1.getForest());
        apply(UNION, f1, f2, out);
        return out;
    }
    Func operator^(const Func &f1, const Func &f2)
    {
        // A xor B = (A or B) and not (A and B)
        Func out1, out2, out;
        out1 = f1 | f2;
        out2 = f1 & f2;
        out2 = !out2;
        out = out1 & out2;
        return out;
    }
    Func operator!(const Func &e)
    {
        Func res(e.getForest());
        apply(COMPLEMENT, e, res);
        return res;
    }

    Func operator+=(Func &f1, const Func &f2)
    {
        apply(PLUS, f1, f2, f1);
        return f1;
    }
    Func operator-=(Func &f1, const Func &f2)
    {
        apply(MINUS, f1, f2, f1);
        return f1;
    }
    Func operator*=(Func &f1, const Func &f2)
    {
        apply(MULTIPLY, f1, f2, f1);
        return f1;
    }
    Func operator/=(Func &f1, const Func &f2)
    {
        apply(DIVIDE, f1, f2, f1);
        return f1;
    }

    Func operator&=(Func &f1, const Func &f2)
    {
        apply(INTERSECTION, f1, f2, f1);
        return f1;
    }
    Func operator|=(Func &f1, const Func &f2)
    {
        apply(UNION, f1, f2, f1);
        return f1;
    }
    Func operator^=(Func &f1, const Func &f2)
    {
        return f1 ^ f2;
    }

    Func operator<(Func &f1, const Func &f2)
    {
        apply(LESS_THAN, f1, f2, f1);
        return f1;
    }
    Func operator>(Func &f1, const Func &f2)
    {
        apply(GREATER_THAN, f1, f2, f1);
        return f1;
    }
    Func operator<=(Func &f1, const Func &f2)
    {
        apply(LESS_THAN_EQUAL, f1, f2, f1);
        return f1;
    }
    Func operator>=(Func &f1, const Func &f2)
    {
        apply(GREATER_THAN_EQUAL, f1, f2, f1);
        return f1;
    }
    /* These will let us do C++ style output, with our output class */
    inline Output& operator<<(Output &s, const ForestSetting &setting)
    {
        //
        return s;
    }

    
    inline Output& operator<<(Output &s, const Edge &e)
    {
        // implementations TBD
        return s;
    }
    inline Output& operator<<(Output &s, char x)
    {
        // implementations TBD
        return s;
    }
    inline Output& operator<<(Output &s, std::string x)
    {
        // implementations TBD
        return s;
    }
    inline Output& operator<<(Output &s, int x)
    {
        // implementations TBD
        return s;
    }
    inline Output& operator<<(Output &s, long x)
    {
        // implementations TBD
        return s;
    }
    inline Output& operator<<(Output &s, unsigned x)
    {
        // implementations TBD
        return s;
    }
    inline Output& operator<<(Output &s, unsigned long x)
    {
        // implementations TBD
        return s;
    }
    inline Output& operator<<(Output &s, double x)
    {
        // implementations TBD
        return s;
    }
}