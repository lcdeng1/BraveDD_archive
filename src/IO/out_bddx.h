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
    BddxMaker(const Forest* f) {
        parent = f;
    }
    ~BddxMaker() {}

    void buildBddx(const Func& func, const std::string fn="");
    void buildBddx(const std::vector<Func>& func, const std::string fn="");
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    void makeHeader(std::ostream& outfile);
    void buildBddx(const Func& func, std::ostream& outfile);
    void buildBddx(const std::vector<Func>& func, std::ostream& outfile);

    const Forest* parent;
};

#endif