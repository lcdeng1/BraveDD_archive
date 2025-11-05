#include "lexer.h"

using namespace BRAVE_DD;

// ******************************************************************
// *                                                                *
// *                                                                *
// *                     FileReader  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************
FileReader::FileReader(const std::string& inpath)
{
    if (!fileType(inpath.c_str())) {
        std::cout << "[BRAVE_DD] ERROR!\t FileReader(): Unknown file type or compress\n";
        exit(1);
    }
    if(compress == ' ') {
        closeNeed = false;
        if (inpath != "") {
            infile = fopen(inpath.c_str(), "rb");
            filename = inpath;
            if (!infile) {
                std::cout << "[BRAVE_DD] ERROR!\t FileReader(): Could not fopen file: " << inpath << std::endl;
                exit(1);
            }
            // std::string buffer = std::string(1024, '\0');
            // fread(&buffer[0], sizeof(char), 1024, infile);
            // std::cerr << "buffer: " << buffer << std::endl;

        } else {
            infile = stdin;
        }
        return;
    }
    // now we will use popen
    closeNeed = true;
    
    std::string zcat = "";
    switch (compress)
    {
        case 'x': zcat += "xzcat";   break;
        case 'g': zcat += "gzcat";   break;
        case 'b': zcat += "bzcat";   break;
        default:    zcat += "cat";
    }

    if (inpath == "") {
        infile = popen(zcat.c_str(), "r");
        return;
    }

    zcat += " ";
    zcat += inpath;
    infile = popen(zcat.c_str(), "r");
    filename = inpath;
}

FileReader::~FileReader()
{
    if (infile) {
        if (closeNeed) {
            pclose(infile);
        } else {
            fclose(infile);
        }
    }
}

bool FileReader::matches(const char* ext, char fmt, char comp)
{
    int ext1length = 4;
    const char* ext1 = (fmt == 'p') ? ".pla" : (fmt == 'b') ? ".bin" : ".bddx";
    if (fmt == 'x') ext1length = 5;
    const char* ext2 = (comp == 'x') ? ".xz" : (comp == 'g') ? ".gz" : (comp == 'b') ? ".bz2" : "";
    return strncmp(ext1, ext, ext1length) == 0 && strcmp(ext2, ext + ext1length) == 0;
}

bool FileReader::fileType(const char* path)
{
    const char* formats = "pbx";
    const char* comps = " xgb";
    for (size_t i=0; path[i]; i++) {
        if (path[i] == '.') {
            for (size_t f=0; formats[f]; f++) {
                format = formats[f];
                for (size_t c=0; comps[c]; c++) {
                    compress = comps[c];
                    if (matches(path + i, format, compress)) return true;
                }
            }
        }
    }
    return false;
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      BddxLexer  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************
std::vector<std::string> BRAVE_DD::BddxLexer::KEY_WORDS = {
    // forest header
    "FOREST",
    "TYPE",
    "REDUCED",
    "LVLS",
    "DIM",
    "RANGE",
    "NNUM",
    "RNUM",
    // nonterminal nodes list
    "NODES",
    "L",
    // root edges list
    "ROOTS",
    // reduction rules
    "X",
    "EL0",
    "EL1",
    "EH0",
    "EH1",
    "AL0",
    "AL1",
    "AH0",
    "AH1",
    "I0",
    "I1",
    // true, flase
    "true",
    "flase"
};

std::vector<TokenType> BRAVE_DD::BddxLexer::KEY_TYPE = {
    //
    TokenType::FOREST,
    TokenType::TYPE,
    TokenType::REDUCED,
    TokenType::LVLS,
    TokenType::DIM,
    TokenType::RANGE,
    TokenType::NNUM,
    TokenType::RNUM,
    TokenType::NODES,
    TokenType::LVL,
    TokenType::ROOTS,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::RULE,
    TokenType::BOOL,
    TokenType::BOOL
};

bool BRAVE_DD::BddxLexer::consumCommentsML()
{
    const unsigned commentLine = lineNumber;
    char c = ' ', last = ' ';
    for (;;) {
        // when reach to the end of the buffer, reloading buffer
        if (buffer[bufferIndex] == 0) {
            readInBuffer();
        }
        c = buffer[bufferIndex];
        bufferIndex++;
        // still nothing? that means it's the EOF
        if (0 == c) {
            std::cerr << "[BraveDD] Error: BddxLexer error in file " << reader.getFileName() << " line " << commentLine << std::endl;
            std::cerr << "\tUnclosed comment." << std::endl;
            return 1;
        }
        if (c == '\n') {
            lineNumber++;
            continue;
        }
        if (last == '*' && c == '/') {
            return 0;
        }
        last = c;
    }
    return 0;
}
void BRAVE_DD::BddxLexer::consumCommentsSL()
{
    char c = ' ';
    for (;;) {
        // when reach to the end of the buffer, reloading buffer
        if (buffer[bufferIndex] == 0) {
            readInBuffer();
        }
        c = buffer[bufferIndex];
        bufferIndex++;
        // the EOF
        if (0 == c) {
            return;
        }
        // a new line
        if (c == '\n') {
            lineNumber++;
            return;
        }
    }
}
bool BRAVE_DD::BddxLexer::consumIdent()
{
    // the first character is already in lexeme[0], then continue
    char c = ' ';
    size_t numChars = 1;
    for (;;) {
        checkInBuffer();
        c = buffer[bufferIndex];
        // letter, digit or underscores?
        if ((('a' <= c)&&(c <= 'z')) || 
            (('A' <= c)&&(c <= 'Z')) || 
            (('0' <= c)&&(c <= '9')) || (c == '_')) {
            // length limitation?
            if (numChars >= MAX_IDENT) {
                lexeme[numChars] = 0;
                std::cerr << "[BraveDD] Error: BddxLexer error in file " << reader.getFileName() << " line " << lineNumber << std::endl;
                std::cerr << "\tIdentifier " << lexeme.substr(0, MAX_IDENT-1) << "... is too long." << std::endl;
                return 1;
            }
            lexeme[numChars] = c;
            numChars++;
            bufferIndex++;
            continue;
        }
        // definitely not an identifier
        break;
    }
    lexeme[numChars] = '\0';
    tokenType = TokenType::IDENT;
    return 0;
}

bool BRAVE_DD::BddxLexer::consumNum()
{
    // the first digit is already in lexeme[0], asuming it's integer then further check
    tokenType = TokenType::INT_LIT;
    char c = ' ';
    char hasDot = 0, isExp = 0;
    size_t numDigits = 1;
    for (;;) {
        checkInBuffer();
        c = buffer[bufferIndex];
        if (c=='.') {
            if (hasDot || isExp) {
                std::cerr << "[BraveDD] Error: BddxLexer error in file " << reader.getFileName() << " line " << lineNumber << std::endl;
                std::cerr << "\tMaybe not valid real number " << std::endl;
                return 1;
            } else {
                hasDot = 1;
                tokenType = TokenType::REAL_LIT;
            }
        } else if ((c == 'e') || (c == 'E')) {
            if (isExp) {
                break;
            } else {
                isExp = 1;
                tokenType = TokenType::REAL_LIT;
            }
        } else if ((c < '0') || (c > '9')) {
            if (((isExp) && (c != '-') && (c != '+')) || (!isExp)) {
                break;
            }
        }
        if (((!hasDot) && (numDigits >= MAX_INT)) || 
            ((hasDot) && (numDigits >= MAX_REAL)) || 
            ((isExp) && (numDigits >= MAX_REAL))) {
            lexeme[numDigits] = '\0';
            std::cerr << "[BraveDD] Error: BddxLexer error in file " << reader.getFileName() << " line " << lineNumber << std::endl;
            std::cerr << "\tNumber " << lexeme.substr(0, numDigits) << "... is too long" << std::endl;
            return 1;
        }
        lexeme[numDigits] = c;
        numDigits++;
        bufferIndex++;
    }
    lexeme[numDigits] = '\0';
    return 0;
}

bool BRAVE_DD::BddxLexer::consumSymbol()
{
    // The first character is in lexeme[0]
    if ((lexeme[0] == '*') ||
        (lexeme[0] == '{') || (lexeme[0] == '}') ||
        (lexeme[0] == ':') ||
        (lexeme[0] == ',') ||
        (lexeme[0] == '<') || (lexeme[0] == '>')){
        tokenType = (TokenType)lexeme[0];
        lexeme[1] = '\0';
    } else {
        std::cerr << "[BraveDD] Error: BddxLexer error in file " << reader.getFileName() << " line " << lineNumber << std::endl;
        std::cerr << "\tUnknown symbol: " << lexeme.substr(0, 1) << std::endl;
        return 1;
    }
    return 0;
}

bool BRAVE_DD::BddxLexer::nextToken()
{
    if (reader.end()) {
        tokenType = TokenType::END;
        return 1;
    }
    char c = ' ';
    for (;;) {
        // check in buffer
        checkInBuffer();
        c = buffer[bufferIndex];
        bufferIndex++;
        // the EOF
        if (c == 0) {
            tokenType = TokenType::END;
            return 0;
        }
        // skip the whitespace
        if ((c == ' ') || (c == '\t') || (c == '\r')) {
            continue;
        }
        if (c == '\n') {
            lineNumber++;
            continue;
        }

        lexeme[0] = c;        
        // Identifiers or keywords
        if ((('a'<=c) && (c<='z')) || (('A'<=c) && (c<='Z'))) {
            if (consumIdent()) return 1;
            // check if this is Nonterminal node, terminal node, or root edge
            if ((c=='N') || (c=='n')) {
                bool isNode = 1;
                for (size_t i=1; i<lexeme.find('\0'); i++) {
                    if ((lexeme[i] < '0') || (lexeme[i] > '9')) {
                        isNode = 0;
                        break;
                    }
                }
                if (isNode) {
                    tokenType = TokenType::NONTERMI;
                    return 0;
                }
            } else if ((c=='T') || (c=='t')) {
                bool isTerminal = 1;
                for (size_t i=1; i<lexeme.find('\0'); i++) {
                    if ((lexeme[i] < '0') || (lexeme[i] > '9')) {
                        isTerminal = 0;
                        break;
                    }
                }
                if (isTerminal) {
                    tokenType = TokenType::TERMI;
                    return 0;
                }
            } else if ((c=='R') || (c=='r')) {
                bool isRoot = 1;
                for (size_t i=1; i<lexeme.find('\0'); i++) {
                    if ((lexeme[i] < '0') || (lexeme[i] > '9')) {
                        isRoot = 0;
                        break;
                    }
                }
                if (isRoot) {
                    tokenType = TokenType::ROOTEDGE;
                    return 0;
                }
            }
            // check if this lexeme is keyword
            for (size_t i=0; i<KEY_WORDS.size(); i++) {
                if (lexeme.substr(0, lexeme.find('\0')) == KEY_WORDS[i]) {
                    tokenType = KEY_TYPE[i];
                    return 0;
                }
            }
            return 0;
        }
        // Numbers: integer and real
        if (('0'<=c) && (c<='9')) {
            return consumNum();
        }
        // skip the comments, which needs to read one more character
        checkInBuffer();
        const char next = buffer[bufferIndex];
        if (c == '/') {
            // Multi-line comments
            if (next == '*') {
                bufferIndex++;
                if (consumCommentsML()) return 1;
                continue;
            }
            // Single-line comments
            if (next == '/') {
                bufferIndex++;
                consumCommentsSL();
                continue;
            }
        }
        // here must be symbles
        if (consumSymbol()) return 1;
        if (lexeme.find('\0') > 0) return 0;
    }   // end for loop
    return 0;
}