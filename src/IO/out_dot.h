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
    DotMaker(const Forest* f) {
        parent = f;
        isHideTerminalZero = 0;
    }
    ~DotMaker() {}

    inline void hideTerminalZero() { isHideTerminalZero = 1; }
    inline void showTerminalZero() { isHideTerminalZero = 0; }
    void buildGraph(const Func& func, std::string fn="");
    void buildGraph(const std::vector<Func>& funcs, std::string fn="");
    void runDot(const std::string baseName, const std::string ext);
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    void buildEdge(std::ostream& outfile, const Level lvl, const Edge& edge, const NodeHandle rootHandle=0, const char st = 0);
    void buildGraph(const Func& func, std::ostream& outfile);
    void buildGraph(const std::vector<Func>& funcs, std::ostream& outfile);
    /// ============================================================
    const Forest* parent;
    bool isHideTerminalZero;
};

#endif
