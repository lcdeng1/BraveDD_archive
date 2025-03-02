#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>

// ======================= File Reader =======================

class file_reader {
public:
    // Constructor: opens the file specified by 'inpath'. 'comp' indicates compression:
    //   ' ' : no compression, 'x' : xz, 'g' : gzip, 'b' : bzip2.
    file_reader(const char* inpath, char comp);
    ~file_reader();

    inline bool fok() const { return F != nullptr; }
    inline bool eof() { return feof(F); }
    inline int get() { return fgetc(F); }
    inline unsigned read_unsigned() {
        unsigned x;
        fscanf(F, "%u", &x);
        return x;
    }
    inline void unget(int c) { ungetc(c, F); }
    inline unsigned read(void* ptr, unsigned bytes) { return static_cast<unsigned>(fread(ptr, 1, bytes, F)); }
protected:
    FILE* F;
    bool pclose_F;
};

// ======================= Parser Base Class =======================

class parser {
public:
    parser(file_reader* fr);
    virtual ~parser();

    // Read header information from the PLA file.
    // 'ib', 'ob', and 'minterms' are filled with the number of input bits, output bits, and minterm count.
    // If 'debug' is non-null, debug output is printed.
    virtual void read_header(unsigned &ib, unsigned &ob, unsigned &minterms, FILE* debug = 0) = 0;

    // Read one minterm. 'input_bits' is filled with the input bit string.
    // 'out_terminal' is set to a character representing the output.
    // Returns true if a minterm is successfully read; false otherwise.
    virtual bool read_minterm(char* input_bits, char& out_terminal) = 0;

    // Debug routine: prints header info and then all minterms.
    void debug(bool show_header, bool show_minterms, bool show_summary);
protected:
    inline bool eof() { return FR->eof(); }
    inline int get() { return FR->get(); }
    inline void unget(int c) { FR->unget(c); }
    inline unsigned read_unsigned() { return FR->read_unsigned(); }
    inline int skip_until(char x) {
        for (;;) {
            int c = get();
            if (c == x) return x;
            if (c == EOF) return EOF;
        }
    }
private:
    file_reader* FR;
};

// ======================= PLA Parser =======================

class pla_parser : public parser {
public:
    pla_parser(file_reader* fr);
    virtual ~pla_parser();
    virtual void read_header(unsigned &inbits, unsigned &outbits, unsigned &minterms, FILE* debug = 0) override;
    virtual bool read_minterm(char* input_bits, char& out_terminal) override;
private:
    unsigned inbits;
    unsigned outbits;
};

// ======================= Parser Factory Functions =======================

// Create a new parser for the given file (auto-detecting format and compression).
parser* new_parser(const char* pathname);

// Create a new parser for the given format and compression (when no filename is provided).
parser* new_parser(char format, char compression);

#endif
