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
        Func out;
        // implementations TBD
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
        Func out;
        // implementations TBD
        return out;
    }
    Func operator|(const Func &e1, const Func &e2)
    {
        Func out;
        // implementations TBD
        return out;
    }
    Func operator!(const Func &e)
    {
        // implementations TBD
        return e;
    }

    Func operator+=(Func &e1, const Func &e2)
    {
        // implementations TBD
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
        // implementations TBD
        return e1;
    }
    Func operator|=(Func &e1, const Func &e2)
    {
        // implementations TBD
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