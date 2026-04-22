#include "parser.h"

using namespace BRAVE_DD;

// ******************************************************************
// *                                                                *
// *                       ParserPla methods                        *
// *                                                                *
// ******************************************************************
void ParserPla::readHeader()
{
    int next = 0;
    for (;;) {
        // read lines while first character is '.'
        next = get();
        if (next == EOF) {
            std::cout << "[BRAVE_DD] ERROR!\t ParserPla::readHeader(): Unexpected EOF.\n";
            break;
        }
        if (next == '#') {
            skipUntil('\n');
            continue;
        }
        if (next != '.') {
            unget(next);
            break;
        }
        next = get();
        if (next == 'i') {
            inbits = readUnsigned();
        }
        if (next == 'o') {
            outbits = readUnsigned();
        }
        if (next == 'p') {
            numf = readSize();
        }
        skipUntil('\n');
    }
}

bool ParserPla::readAssignment(std::vector<bool>& inputs, char& out)
{
    int c;
    while (true) {
        c = get();
        if (c == EOF) return false;
        if (c == '.') {
            skipUntil('\n');
        } else {
            break;
        }
    }
    // read inputs
    inputs[0] = static_cast<bool>(c-'0');
    unsigned n;
    for (n=1; n<inbits; n++) {
        c = get();
        if (c == EOF) {
            std::cout << "[BRAVE_DD] ERROR!\t ParserPla::readAssignment(): Unexpected EOF.\n";
            return false;
        }
        inputs[n] = static_cast<bool>(c-'0');
    }
    // read out
    // inputs[n] = 0;
    unsigned t = 0;
    while ((c = get()) != '\n' && c != EOF) {
        if (c == '1') t = (t << 1) | 1;
        else if (c == '~') t <<= 1;
    }
    out = '0' + t;
    return true;
}
bool ParserPla::readAssignment(std::vector<bool>& inputs, int& out)
{
    int c;
    while (true) {
        c = get();
        if (c == EOF) return false;
        if (c == '.') {
            skipUntil('\n');
        } else {
            break;
        }
    }
    // read inputs
    inputs[0] = static_cast<bool>(c-'0');
    unsigned n;
    for (n=1; n<inbits; n++) {
        c = get();
        if (c == EOF) {
            std::cout << "[BRAVE_DD] ERROR!\t ParserPla::readAssignment(): Unexpected EOF.\n";
            return false;
        }
        inputs[n] = static_cast<bool>(c-'0');
    }
    // read out
    // inputs[n] = 0;
    int t = 0;
    while ((c = get()) != '\n' && c != EOF) {
        if (c == '1') t = (t << 1) | 1;
        else if (c == '~') t <<= 1;
    }
    out = t;
    return true;
}


// ******************************************************************
// *                                                                *
// *                      ParserBddx methods                        *
// *                                                                *
// ******************************************************************
bool BRAVE_DD::ParserBddx::match(const TokenType tt, const std::string name)
{
    bool isMatch = lexer.match(tt);
    if (!isMatch) {
        std::cerr << "[BRAVE_DD] Error: ParserBddx error." << std::endl;
        std::cerr << "\tWrong Token: " << lexer.getLexeme() << std::endl;
        std::cerr << "\tExpected " << name 
                    << " in file: " << lexer.getFilename() 
                    << " in line: " << lexer.getLinenum() << std::endl;
        return 0;
    }
    lexer.nextToken();
    return 1;
}

bool BRAVE_DD::ParserBddx::parseHeader(Forest* forest)
{
    // "FOREST" is consumed
    if (!match(TokenType::LBRACE, "'{'")) return 1;
    for (;;) {
        if (lexer.match(TokenType::RBRACE)) break;
        // go through
        if (lexer.match(TokenType::TYPE)) {
            lexer.nextToken();
            std::string lexeme = lexer.getLexeme();
            if (match(TokenType::IDENT, "PredefinedForest")) {
                // get the predefined type, TBD
                continue;
            }
            // error message?
            return 1;
        } else if (lexer.match(TokenType::REDUCED)) {
            lexer.nextToken();
            std::string lexeme = lexer.getLexeme();
            if (match(TokenType::BOOL, "'true' or 'false")) {
                // get if the input is reduced
                isReduced = (lexeme == "true") ? 1 : 0;
                continue;
            }
            // error message?
            return 1;
        } else if (lexer.match(TokenType::LVLS)) {
            lexer.nextToken();
            std::string lexeme = lexer.getLexeme();
            if (match(TokenType::INT_LIT, "Integer")) {
                numVars = (Level)std::stoi(lexeme);
                if (match(TokenType::STAR, "'*'") && match(TokenType::INT_LIT, "Integer")) {
                    // get is relation, TBD
                    continue;
                }
            }
            // error message?
            return 1;
        } else if (lexer.match(TokenType::RANGE)) {
            lexer.nextToken();
            std::string lexeme = lexer.getLexeme();
            if (match(TokenType::IDENT, "Range Type")) {
                // get the range type, keywords TBD
                continue;
            }
            // error message?
            return 1;
        } else if (lexer.match(TokenType::NNUM)) {
            lexer.nextToken();
            std::string lexeme = lexer.getLexeme();
            if (match(TokenType::INT_LIT, "Integer")) {
                numNodes = (uint64_t)std::stoul(lexeme);
                continue;
            }
            // error message?
            return 1;
        } else if (lexer.match(TokenType::RNUM)) {
            lexer.nextToken();
            std::string lexeme = lexer.getLexeme();
            if (match(TokenType::INT_LIT, "Integer")) {
                numRoots = (uint64_t)std::stoul(lexeme);
                continue;
            }
            // error message?
            return 1;
        }
        // check if this is compatible with target forest, TBD
        // error message?
        return 1;
    } // end loop
    if (!match(TokenType::RBRACE, "'}'")) return 1;
    return 0;
}
bool BRAVE_DD::ParserBddx::parseEdge(Forest* forest, const Level& beginLvl, Edge& edge)
{
    // lexer at "<"
    if (!match(TokenType::LT, "\'<\'")) return 1;
    // edge info
    ReductionRule rule = RULE_X;
    bool swap = 0, comp = 0;
    uint64_t nodeId;

    std::string token = lexer.getLexeme();
    if (!(match(TokenType::RULE, "Edge Rule") && match(TokenType::COMMA, "\',\'"))) {
        std::cerr << "[BRAVE_DD] Error: ParserBddx::parseEdge(): unknown edge rule: \"" << token
                        << "\" in file: " << lexer.getFilename() 
                        << " in line: " << lexer.getLinenum() << std::endl;
        return 1;
    }
    rule = string2Rule(token);

    token = lexer.getLexeme();
    if (!(match(TokenType::INT_LIT, "Complement Flag: 0/1") && match(TokenType::COMMA, "\',\'"))) {
        std::cerr << "[BRAVE_DD] Error: ParserBddx::parseEdge(): unknown complement flag: \"" << token
                        << "\" in file: " << lexer.getFilename() 
                        << " in line: " << lexer.getLinenum() << std::endl;
        return 1;
    }
    if (token == "1") comp = 1;

    token = lexer.getLexeme();
    if (!(match(TokenType::INT_LIT, "Swap Flag: 0/1") && match(TokenType::COMMA, "\',\'"))) {
        std::cerr << "[BRAVE_DD] Error: ParserBddx::parseEdge(): unknown swap flag: \"" << token
                        << "\" in file: " << lexer.getFilename() 
                        << " in line: " << lexer.getLinenum() << std::endl;
        return 1;
    }
    if (token == "1") swap = 1;

    token = lexer.getLexeme();
    if (lexer.match(TokenType::TERMI)) {
        // terminal node
        int terminalVal = std::stoi(token.substr(1));
        edge.setEdgeHandle(makeTerminal(terminalVal));
        edge.setRule(rule);
        edge.setComp(comp);
        edge.setSwap(swap, (forest->getSetting().getSwapType() == TO || forest->getSetting().getSwapType() == FROM_TO));
        edge = forest->normalizeEdge(beginLvl, edge);
    } else if (lexer.match(TokenType::INT_LIT)) {
        // nonterminal node
        nodeId = std::stoull(token);
        if (isReduced) {
            auto it = reducedNodesMap.find(nodeId);
            if (it == reducedNodesMap.end()) {
                std::cerr << "[BRAVE_DD] Error: ParserBddx::parseEdge(): undefined node handle: \"" << token
                            << "\" in file: " << lexer.getFilename() 
                            << " in line: " << lexer.getLinenum() << std::endl;
                return 1;
            }
            edge.setRule(rule);
            edge.setComp(comp);
            edge.setSwap(swap, (forest->getSetting().getSwapType() == TO || forest->getSetting().getSwapType() == FROM_TO));
            edge.setLevel(reducedNodesMap[nodeId].first);
            edge.setNodeHandle(reducedNodesMap[nodeId].second);
        } else {
            auto it = nodesMap.find(nodeId);
            if (it == nodesMap.end()) {
                std::cerr << "[BRAVE_DD] Error: ParserBddx::parseEdge(): undefined node handle: \"" << token
                            << "\" in file: " << lexer.getFilename() 
                            << " in line: " << lexer.getLinenum() << std::endl;
                return 1;
            }
            // merge edge
            EdgeLabel root = 0;
            packRule(root, rule);
            packComp(root, comp);
            packSwap(root, swap);
            edge = forest->mergeEdge(beginLvl, nodesMap[nodeId].first, root, nodesMap[nodeId].second);
        }
    } else {
        std::cerr << "[BRAVE_DD] Error: ParserBddx::parseEdge(): invalid node handle" << std::endl;
        return 1;
    }
    lexer.nextToken();
    if (!match(TokenType::GT, "\'>\'")) return 1;
    return 0;
}
bool BRAVE_DD::ParserBddx::parseChildEdge(Forest* forest, const Level& beginLvl, std::vector<Edge>& child)
{
    // lexer at "0"
    std::vector<bool> isChildSet;
    if (forest->getSetting().isRelation()) {
        child = std::vector<Edge>(4);
        isChildSet = std::vector<bool>(4, 0);
    } else {
        child = std::vector<Edge>(2);
        isChildSet = std::vector<bool>(2, 0);
    }
    for (;;) {
        if (lexer.match(TokenType::COMMA)) {
            lexer.nextToken();
            continue;
        }
        if (lexer.match(TokenType::NONTERMI) || lexer.match(TokenType::RBRACE)) break;
        std::string childId = lexer.getLexeme();
        if (!(match(TokenType::INT_LIT, "childID: Integer") && match(TokenType::COLON, "\':\'"))) return 1;
        int i = static_cast<int>(std::stoi(childId));
        if (parseEdge(forest, beginLvl, child[i])) return 1;
        if (!isChildSet[i]) isChildSet[i] = 1;
    }
    auto it = std::find(isChildSet.begin(), isChildSet.end(), 0);
    if (it != isChildSet.end()) {
        std::cerr << "[BRAVE_DD] Error: ParserBddx::parseChildEdge(): child edges: " << std::distance(isChildSet.begin(), it)
                    << "\" in file: " << lexer.getFilename() 
                    << " in line: " << lexer.getLinenum()-1 << std::endl;
        return 1;
    }
    return 0;
}
bool BRAVE_DD::ParserBddx::parseNode(Forest* forest)
{
    // lexer at "N123456"
    std::string lexeme = lexer.getLexeme();
    uint64_t nodeId = std::stoull(lexeme.substr(1));
    if (!match(TokenType::NONTERMI, "Nonterminal Identifier")) return 1;
    Level lvl = 0;
    if (!match(TokenType::LVL, "'L'")) return 1;
    lexeme = lexer.getLexeme();
    if (!match(TokenType::INT_LIT, "Integer")) return 1;
    lvl = static_cast<Level>(std::stoi(lexeme));
    if (!match(TokenType::COLON, "\':\'")) return 1;
    std::vector<Edge> child;
    if (parseChildEdge(forest, lvl-1, child)) return 1;
    if (isReduced) {
        Node node(forest->setting);
        for (size_t i=0; i<child.size(); i++) {
            node.setChildEdge(i, child[i].getEdgeHandle(), forest->setting.isRelation(), forest->setting.getReductionSize()>0);
        }
        NodeHandle handle = forest->insertNode(lvl, node);
        reducedNodesMap.insert({nodeId, {lvl, handle}});

    } else {
        // reduce
        EdgeLabel root = 0;
        packRule(root, RULE_X);
        Edge reduced  = forest->reduceEdge(lvl, root, lvl, child);
        nodesMap.insert({nodeId, {lvl, reduced}});
    }
    
    return 0;
}
bool BRAVE_DD::ParserBddx::parseNodes(Forest* forest)
{
    // "FOREST" is consumed
    if (!match(TokenType::LBRACE, "'{'")) return 1;
    for (;;) {
        if (lexer.match(TokenType::RBRACE)) break;
        // go through
        if (lexer.match(TokenType::NONTERMI)) {
            // parse and reduce node
            if (parseNode(forest)) return 1;
            continue;
        }
    } // end loop
    if (!match(TokenType::RBRACE, "'}'")) return 1;
    return 0;
}
bool BRAVE_DD::ParserBddx::parseRoots(Forest* forest)
{
    // "ROOTS" is consumed
    if (!match(TokenType::LBRACE, "'{'")) return 1;
    for (;;) {
        if (lexer.match(TokenType::RBRACE)) break;
        // go through
        std::string lexeme = lexer.getLexeme();
        if (!match(TokenType::ROOTEDGE, "Root Identifier")) return 1;
        uint64_t rootId = std::stoull(lexeme.substr(1));
        // parse and reduce node
        Edge rootEdge;
        if (parseEdge(forest, forest->getSetting().getNumVars(), rootEdge))  return 1;
        rootsMap.insert({rootId, Func(forest, rootEdge)});
        continue;
    } // end loop
    if (!match(TokenType::RBRACE, "'}'")) return 1;
    return 0;
}
bool BRAVE_DD::ParserBddx::parse(Forest* forest)
{
    if (!forest) {
        std::cerr << "[BRAVE_DD] Error: ParserBddx::parse(): invalid target Forest" << std::endl;
        return 1;
    }
    bool isHeaderRead = 0;
    bool isNodesRead = 0;
    bool isRootsRead = 0;
    for (;;) {
        if (lexer.match(TokenType::END)) break;
        if (lexer.match(TokenType::FOREST)) {
            lexer.nextToken();
            if (parseHeader(forest)) {
                std::cerr << "[BRAVE_DD] Error: ParserBddx error for FOREST." << std::endl;
                return 1;
            }
            isHeaderRead = 1;
            continue;
        } else if (lexer.match(TokenType::NODES)) {
            lexer.nextToken();
            if (parseNodes(forest)) {
                std::cerr << "[BRAVE_DD] Error: ParserBddx error for NODES." << std::endl;
                return 1;
            }
            isNodesRead = 1;
            continue;
        } else if (lexer.match(TokenType::ROOTS)) {
            lexer.nextToken();
            if (parseRoots(forest)) {
                std::cerr << "[BRAVE_DD] Error: ParserBddx error for ROOTS." << std::endl;
                return 1;
            }
            isRootsRead = 1;
            continue;
        }
        // unkown
        return 1;
    } // end loop
    if (!isHeaderRead) {
        std::cerr << "[BRAVE_DD] Error: ParserBddx::parse(): Missing FOREST statement." << std::endl;
        return 1;
    } else if (!isNodesRead) {
        std::cerr << "[BRAVE_DD] Error: ParserBddx::parse(): Missing NODES statement." << std::endl;
        return 1;
    } else if (!isRootsRead) {
        std::cerr << "[BRAVE_DD] Error: ParserBddx::parse(): Missing ROOTS statement." << std::endl;
        return 1;
    }
    return 0;
}
