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

    Root operator+(const Root &e1, const Root &e2)
    {
        Root out;
        // implementations TBD
        return out;
    }
    Root operator-(const Root &e1, const Root &e2)
    {
        Root out;
        // implementations TBD
        return out;
    }
    Root operator*(const Root &e1, const Root &e2){
        Root out;
        // implementations TBD
        return out;
    }
    Root operator/(const Root &e1, const Root &e2)
    {
        Root out;
        // implementations TBD
        return out;
    }

    Root operator&(const Root &e1, const Root &e2)
    {
        Root out;
        // implementations TBD
        return out;
    }
    Root operator|(const Root &e1, const Root &e2)
    {
        Root out;
        // implementations TBD
        return out;
    }
    Root operator!(const Root &e)
    {
        // implementations TBD
        return e;
    }

    Root operator+=(Root &e1, const Root &e2)
    {
        // implementations TBD
        return e1;
    }
    Root operator-=(Root &e1, const Root &e2)
    {
        // implementations TBD
        return e1;
    }
    Root operator*=(Root &e1, const Root &e2)
    {
        // implementations TBD
        return e1;
    }
    Root operator/=(Root &e1, const Root &e2)
    {
        // implementations TBD
        return e1;
    }

    Root operator&=(Root &e1, const Root &e2)
    {
        // implementations TBD
        return e1;
    }
    Root operator|=(Root &e1, const Root &e2)
    {
        // implementations TBD
        return e1;
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