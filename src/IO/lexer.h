#ifndef BRAVE_DD_LEXER_H
#define BRAVE_DD_LEXER_H

#include "../defines.h"
#include "tokens.h"

namespace BRAVE_DD{
    class FileReader;
    class BddxLexer;
} // end of namespace
// ******************************************************************
// *                                                                *
// *                      FileReader class                          *
// *                                                                *
// ******************************************************************
class BRAVE_DD::FileReader {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    // FileReader(const char* inpath);
    FileReader(const std::string& inpath);
    ~FileReader();

    inline bool end() { return !infile; }
    inline bool eof() { return feof(infile); }
    inline int get() { return fgetc(infile); }
    inline void unget(int c) { ungetc(c, infile); }
    inline size_t readInBuffer(std::string& buffer, const size_t size) {
        return fread(&buffer[0], sizeof(char), size, infile);
    }
    inline unsigned readUnsigned() {
        unsigned x;
        fscanf(infile, "%u", &x);
        return x;
    }
    inline size_t readSize() {
        size_t x;
        fscanf(infile, "%zu", &x);
        return x;
    }
    inline char getFormat() { return format; }
    inline std::string getFileName() { return filename; }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    // helper functions to determine format and compress type
    bool matches(const char* ext, char fmt, char comp);
    bool fileType(const char* path);

    FILE*       infile;         // for popen
    std::string filename;       // input file name
    char        format;         // 'p' for PLA, 'b' for BIN, 'x' for BDDX
    char        compress;       // ' ' for none, 'x' for xz, 'g' for gzip, 'b' for bzip2
    bool        closeNeed;      // infile needs pclose if true, or fclose if false
};

// ******************************************************************
// *                                                                *
// *                      BddxLexer class                           *
// *                                                                *
// ******************************************************************
class BRAVE_DD::BddxLexer {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    BddxLexer(const std::string& filename):reader(filename) {
        if (reader.getFormat() != 'x') {
            std::cout << "[BRAVE_DD] ERROR!\t BddxLexer(): Unexpected file format.\n";
            exit(1);
        }
        lineNumber = 1;
        tokenType = TokenType::END;
        buffer = std::string(MAX_LEX, '\0');
        lexeme = std::string(MAX_STR, '\0');
        nextToken();
    }
    ~BddxLexer() {
        reader.~FileReader();
    }

    bool nextToken();
    inline std::string getLexeme() {
        return lexeme.substr(0, lexeme.find('\0'));
    }
    inline TokenType getLexemeType() {
        return tokenType;
    }
    inline bool match(const TokenType tt) {
        if (tt != tokenType) {
            return 0;
        }
        return 1;
    }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    // helper functions for lexing
    // load characters into a buffer
    inline void readInBuffer() {
        size_t numEntries = reader.readInBuffer(buffer, MAX_LEX);
        bufferIndex = 0;
        if (numEntries < MAX_LEX) {
            // add 0 to buffer's last as end of buffer flag
            buffer[numEntries] = '\0';
        }
    }
    // check if need to reload characters, and reload if needed
    inline void checkInBuffer() {
        if ((bufferIndex == buffer.size()) || (buffer[bufferIndex] == '\0')) {
            reader.readInBuffer(buffer, MAX_LEX);
        }
    }
    // asuming we've identified /*, then continue until identifing */ or EOF
    bool consumCommentsML();
    // asuming we've identified //, then continue until identifing a new line \n or EOF
    void consumCommentsSL();
    // consum idedntifiers
    bool consumIdent();
    // consum numbers
    bool consumNum();
    // consum symbol
    bool consumSymbol();
    

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    FileReader      reader;         // input file reader
    unsigned        lineNumber;     // current line number of lexing file
    TokenType       tokenType;      // token type defined in token.h
    std::string     lexeme;         // current lexeme
    std::string     buffer;         // buffer to store the ASCII character from the input file
    unsigned        bufferIndex;    // char index in buffer

    static const size_t MAX_LEX = 2048;                 // The limitation of buffer size
    static const size_t MAX_STR = 1024;                 // The longest size of strings
    static const size_t MAX_IDENT = 48;                 // The longest size of identifiers
    static const size_t MAX_INT = 48;                   // The longest size of integers
    static const size_t MAX_REAL = 48;                  // The longest size of real numbers
    static std::vector<std::string> KEY_WORDS;          // List of keywords
    static std::vector<TokenType> KEY_TYPE;             // list of keywords' type
};



#endif