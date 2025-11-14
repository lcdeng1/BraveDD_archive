#ifndef BRAVE_DD_PARSER_H
#define BRAVE_DD_PARSER_H

#include "../defines.h"
#include "lexer.h"
#include "../forest.h"

namespace BRAVE_DD{
    class Parser;
    class ParserPla;
    class ParserBddx;
    class ParserBin;        // TBD
} // end of namespace

// ******************************************************************
// *                                                                *
// *                         Parser class                           *
// *                                                                *
// ******************************************************************
class BRAVE_DD::Parser {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    // Parser(FileReader* FR);
    Parser(const std::string inpath):reader(inpath) {};
    // virtual ~Parser();

    virtual void readHeader() = 0;

    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    // helper functions for parsing
    inline char getFormat() { return reader.getFormat(); }
    inline bool eof() { return reader.eof(); }
    inline int get() { return reader.get(); }
    inline void unget(int c) { reader.unget(c); }
    inline unsigned readUnsigned() { return reader.readUnsigned(); }
    inline size_t readSize() { return reader.readSize(); }
    inline int skipUntil(char x) {
        for (;;) {
            int c = get();
            if (c == x) return x;
            if (c == EOF) return EOF;
        }
    }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    FileReader      reader;
};

// ******************************************************************
// *                                                                *
// *                       ParserPla class                          *
// *                                                                *
// ******************************************************************
class BRAVE_DD::ParserPla : public Parser {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    // ParserPla(FileReader* FR);
    ParserPla(const std::string inpath) : Parser(inpath) {
        if (getFormat() != 'p') {
            std::cout << "[BRAVE_DD] ERROR!\t ParserPla(): Unexpected file format.\n";
            exit(1);
        }
        inbits = 0;
        outbits = 0;
        numf = 0;
    }
    // ~ParserPla();

    // main functions
    virtual void readHeader() override;
    bool readAssignment(std::vector<bool>& inputs, char& out);
    bool readAssignment(std::vector<bool>& inputs, int& out);
    // get bits info
    inline unsigned getInBits() { return inbits; }
    inline unsigned getOutBits() { return outbits; }
    inline size_t getNum() { return numf; }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    unsigned            inbits;     // number of inbits of assignments
    unsigned            outbits;    // number of outbits of assignments
    size_t              numf;       // number of assignments
};

// ******************************************************************
// *                                                                *
// *                      ParserBddx class                          *
// *                                                                *
// ******************************************************************
class BRAVE_DD::ParserBddx {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    ParserBddx(const std::string& inpath) : lexer(inpath) {
        numNodes = 0;
        numRoots = 0;
        numVars = 0;
        isReduced = 0;
    }
    // ~ParserBddx() {}

    // main functions

    /* Parse and build all nodes in the given forest */
    bool parse(Forest* forest);
    /* return the first root built */
    bool buildFunc(Func& root);
    /* return all the roots */
    bool buildFunc(std::vector<Func>& roots);
    
    // get bdd info
    inline uint64_t getNumNodes() { return numNodes; }
    inline uint64_t getNumRoots() { return numRoots; }
    inline Level getNumVars() { return numVars; }
    inline Func getRoot(size_t id=1) { return rootsMap[id]; }
    inline std::vector<Func> getAllRoots() { 
        std::vector<Func> roots;
        for (const auto& r : rootsMap) {
            roots.push_back(r.second);
        }
        return roots;
    }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /* Helper functions */
    bool match(const TokenType tt, const std::string name);
    bool parseHeader(Forest* forest);
    bool parseEdge(Forest* forest, const Level& beginLvl, Edge& edge);
    bool parseChildEdge(Forest* forest, const Level& beginLvl, std::vector<Edge>& child);
    bool parseNode(Forest* forest);
    bool parseNodes(Forest* forest);
    bool parseRoots(Forest* forest);

    BddxLexer           lexer;          // lexer to read bddx tokens
    // map of nodes index to new reduced edge, this is used if input BDD is unreduced
    std::unordered_map<uint64_t, std::pair<Level, Edge>>            nodesMap;
    // map of nodes index to inserted node, this is used if input BDD is reduced
    std::unordered_map<uint64_t, std::pair<Level, NodeHandle>>      reducedNodesMap;
    // built roots
    std::unordered_map<uint64_t, Func>   rootsMap;
    uint64_t            numNodes;       // number of nonterminal nodes
    uint64_t            numRoots;       // number of root edges
    Level               numVars;        // number of variables
    bool                isReduced;      // flag of BDD reduction

};

// more parsers for BIN

#endif