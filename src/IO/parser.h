#ifndef BRAVE_DD_PARSER_H
#define BRAVE_DD_PARSER_H

#include"../defines.h"

namespace BRAVE_DD{
    class FileReader;
    class Parser;
    class ParserPla;
    class ParserBin;        // TBD
    class ParserBddx;       // TBD
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
    FileReader(const char* inpath);
    ~FileReader();

    inline bool fok() const { return infile != nullptr; }
    inline bool eof() { return feof(infile); }
    inline int get() { return fgetc(infile); }
    inline unsigned readUnsigned() {
        unsigned x;
        fscanf(infile, "%u", &x);
        return x;
    }
    inline void unget(int c) { ungetc(c, infile); }
    inline char getFormat() { return format; }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    // helper functions to determine format and compress type
    bool matches(const char* ext, char fmt, char comp);
    bool fileType(const char* path);

    FILE*       infile;         // for popen
    char        format;         // 'p' for PLA, 'b' for BIN, 'x' for BDDX
    char        compress;       // ' ' for none, 'x' for xz, 'g' for gzip, 'b' for bzip2
    bool        closeNeed;      // infile needs pclose if true, or fclose if false
};

// ******************************************************************
// *                                                                *
// *                         Parser class                           *
// *                                                                *
// ******************************************************************
class BRAVE_DD::Parser {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    Parser(FileReader* FR);
    virtual ~Parser();

    virtual void readHeader() = 0;

    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    // helper functions for parsing
    inline bool eof() { return reader->eof(); }
    inline int get() { return reader->get(); }
    inline void unget(int c) { reader->unget(c); }
    inline unsigned readUnsigned() { return reader->readUnsigned(); }
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
    FileReader*     reader;
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
    ParserPla(FileReader* FR);
    ~ParserPla();

    // main functions
    virtual void readHeader() override;
    bool readAssignment(std::vector<char>& inputs, char& out);
    // get bits info
    inline unsigned getInBits() { return inbits; }
    inline unsigned getOutBits() { return outbits; }
    inline unsigned getNum() { return numf; }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    unsigned            inbits;     // number of inbits of assignments
    unsigned            outbits;    // number of outbits of assignments
    unsigned long       numf;       // number of assignments
};

// more parsers for BIN, BDDX

#endif