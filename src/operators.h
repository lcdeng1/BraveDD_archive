#ifndef BRAVE_DD_OPERATORS_H
#define BRAVE_DD_OPERATORS_H

#include "edge.h"
#include "io.h"

namespace BRAVE_DD {
    Edge operator+(const Edge &e1, const Edge &e2);
    Edge operator-(const Edge &e1, const Edge &e2);
    Edge operator*(const Edge &e1, const Edge &e2);
    Edge operator/(const Edge &e1, const Edge &e2);

    Edge operator&(const Edge &e1, const Edge &e2);
    Edge operator|(const Edge &e1, const Edge &e2);
    Edge operator!(const Edge &e);

    Edge operator+=(Edge &e1, const Edge &e2);
    Edge operator-=(Edge &e1, const Edge &e2);
    Edge operator*=(Edge &e1, const Edge &e2);
    Edge operator/=(Edge &e1, const Edge &e2);

    Edge operator&=(Edge &e1, const Edge &e2);
    Edge operator|=(Edge &e1, const Edge &e2);

    /* These will let us do C++ style output, with our output class */
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