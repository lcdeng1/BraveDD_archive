#ifndef BRAVE_DD_OUT_BDDX_H
#define BRAVE_DD_OUT_BDDX_H

#include "../defines.h"

namespace BRAVE_DD {
    class BddxMaker;
    class Forest;
    class Func;
    class Edge;
} // end of namespace

// ******************************************************************
// *                                                                *
// *                       BddxMaker class                          *
// *                                                                *
// ******************************************************************
class BRAVE_DD::BddxMaker {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    BddxMaker(const Forest* f, const std::string bn);
    ~BddxMaker();

    void buildBddx(const Func& func);
    void buildBddx(const std::vector<Func>& func);
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
        const Forest* parent;
        std::string basename;
        std::ofstream outfile;
};

#endif