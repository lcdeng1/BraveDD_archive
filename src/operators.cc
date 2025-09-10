#include "operators.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                             Operators                          *
// *                                                                *
// *                                                                *
// ******************************************************************
namespace BRAVE_DD {
    Edge operator+(const Edge &e1, const Edge &e2)
    {
        Edge out;
        // implementations TBD
        return out;
    }
    Edge operator-(const Edge &e1, const Edge &e2)
    {
        Edge out;
        // implementations TBD
        return out;
    }
    Edge operator*(const Edge &e1, const Edge &e2)
    {
        Edge out;
        // implementations TBD
        return out;
    }
    Edge operator/(const Edge &e1, const Edge &e2)
    {
        Edge out;
        // implementations TBD
        return out;
    }

    Edge operator&(const Edge &e1, const Edge &e2)
    {
        Edge out;
        // implementations TBD
        return out;
    }
    Edge operator|(const Edge &e1, const Edge &e2)
    {
        Edge out;
        // implementations TBD
        return out;
    }
    Edge operator!(const Edge &e)
    {
        Edge out;
        // implementations TBD
        return out;
    }

    Edge operator+=(Edge &e1, const Edge &e2)
    {
        // implementations TBD
        return e1;
    }
    Edge operator-=(Edge &e1, const Edge &e2)
    {
        // implementations TBD
        return e1;
    }
    Edge operator*=(Edge &e1, const Edge &e2)
    {
        // implementations TBD
        return e1;
    }
    Edge operator/=(Edge &e1, const Edge &e2)
    {
        // implementations TBD
        return e1;
    }

    Edge operator&=(Edge &e1, const Edge &e2)
    {
        // implementations TBD
        return e1;
    }
    Edge operator|=(Edge &e1, const Edge &e2)
    {
        // implementations TBD
        return e1;
    }

    Func operator+(const Func &e1, const Func &e2)
    {
        Func out(e1.getForest());
        apply(PLUS, e1, e2, out);
        return out;
    }
    Func operator-(const Func &e1, const Func &e2)
    {
        Func out;
        // implementations TBD
        return out;
    }
    Func operator*(const Func &e1, const Func &e2){
        Func out;
        // implementations TBD
        return out;
    }
    Func operator/(const Func &e1, const Func &e2)
    {
        Func out;
        // implementations TBD
        return out;
    }

    Func operator&(const Func &e1, const Func &e2)
    {
        Func out(e1.getForest());
        apply(INTERSECTION, e1, e2, out);
        return out;
    }
    Func operator|(const Func &e1, const Func &e2)
    {
        Func out(e1.getForest());
        apply(UNION, e1, e2, out);
        return out;
    }
    Func operator^(const Func &e1, const Func &e2)
    {
        // A xor B = (A or B) and not (A and B)
        Func out1, out2, out;
        out1 = e1 | e2;
        out2 = e1 & e2;
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

    Func operator+=(Func &e1, const Func &e2)
    {
        apply(PLUS, e1, e2, e1);
        return e1;
    }
    Func operator-=(Func &e1, const Func &e2)
    {
        // implementations TBD
        return e1;
    }
    Func operator*=(Func &e1, const Func &e2)
    {
        // implementations TBD
        return e1;
    }
    Func operator/=(Func &e1, const Func &e2)
    {
        // implementations TBD
        return e1;
    }

    Func operator&=(Func &e1, const Func &e2)
    {
        apply(INTERSECTION, e1, e2, e1);
        return e1;
    }
    Func operator|=(Func &e1, const Func &e2)
    {
        apply(UNION, e1, e2, e1);
        return e1;
    }

    bool operator==(const Edge &e1, const Edge &e2)
    {
        return 0;
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