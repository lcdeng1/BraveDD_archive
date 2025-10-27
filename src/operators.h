#ifndef BRAVE_DD_OPERATORS_H
#define BRAVE_DD_OPERATORS_H

#include "edge.h"
#include "function.h"
#include "io.h"
#include "operations/apply.h"

namespace BRAVE_DD {
    /* Operators for Funcs */
    
    Func operator+(const Func &f1, const Func &f2);
    Func operator-(const Func &f1, const Func &f2);
    Func operator*(const Func &f1, const Func &f2);
    Func operator/(const Func &f1, const Func &f2);

    Func operator&(const Func &f1, const Func &f2);
    Func operator|(const Func &f1, const Func &f2);
    Func operator^(const Func &f1, const Func &f2);
    Func operator!(const Func &e);

    Func operator+=(Func &f1, const Func &f2);
    Func operator-=(Func &f1, const Func &f2);
    Func operator*=(Func &f1, const Func &f2);
    Func operator/=(Func &f1, const Func &f2);

    Func operator&=(Func &f1, const Func &f2);
    Func operator|=(Func &f1, const Func &f2);
    Func operator^=(Func &f1, const Func &f2);

    Func operator<(Func &f1, const Func &f2);
    Func operator>(Func &f1, const Func &f2);
    Func operator<=(Func &f1, const Func &f2);
    Func operator>=(Func &f1, const Func &f2);

    // Func operator!=(Func &f1, const Func &f2);
    // Func operator==(Func &f1, const Func &f2);



    /* These will let us do C++ style output, with our output class */
    
    inline Output& operator<<(Output &s, const ForestSetting &setting);
    inline Output& operator<<(Output &s, const Edge &e);
    inline Output& operator<<(Output &s, char x);
    inline Output& operator<<(Output &s, std::string x);
    inline Output& operator<<(Output &s, int x);
    inline Output& operator<<(Output &s, long x);
    inline Output& operator<<(Output &s, unsigned x);
    inline Output& operator<<(Output &s, unsigned long x);
    inline Output& operator<<(Output &s, double x);
    
} // namespace BRAVE_DD




#endif