#ifndef BRAVE_DD_OUT_DOT_H
#define BRAVE_DD_OUT_DOT_H

#include "../defines.h"

namespace BRAVE_DD {
    class DotMaker;
    class Forest;
    class Func;
    class Edge;
} // end of namespace

// ******************************************************************
// *                                                                *
// *                       DotMaker class                           *
// *                                                                *
// ******************************************************************
class BRAVE_DD::DotMaker {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    DotMaker(const Forest* f, const std::string bn);
    ~DotMaker();

    void buildGraph(const Func& func);
    void buildGraph(const std::vector<Func>& func);
    void runDot(const std::string ext);
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    void buildEdge(const Level lvl, const Edge& edge, const NodeHandle rootHandle=0, const char st = 0);
    /// ============================================================
        const Forest* parent;
        std::string basename;
        std::ofstream outfile;

};

#endif
