#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

//
// Abstract file reader, for compressed vs. uncompressed input
//
class file_reader {
    FILE* F;
    bool pclose_F;
public:
    /**
     * @param inpath Input file name, or nullptr to read from stdin
     * @param comp   Compression type:
     *               ' ' : none
     *               'x' : xz compression
     *               'g' : gzip compression
     *               'b' : bzip2 compression
     */
    file_reader(const char* inpath, char comp);

    /// Closes the file properly.
    ~file_reader();

    inline bool fok() const { return F; }
    inline bool eof() { return feof(F); }
    inline int get() { return fgetc(F); }
    inline unsigned read_unsigned() {
        unsigned x;
        fscanf(F, "%u", &x);
        return x;
    }
    inline void unget(int c) { ungetc(c, F); }
    inline unsigned read(void* ptr, unsigned bytes) {
        return static_cast<unsigned>(fread(ptr, 1, bytes, F));
    }
};

//
// Abstract parser interface
//
class parser {
public:
    /**
     * Constructs a parser using the provided file_reader.
     */
    parser(file_reader* fr);
    virtual ~parser();

    /**
     * Reads the header from the PLA file.
     * @param inbits   Number of input bits.
     * @param outbits  Number of output bits.
     * @param minterms Number of minterms.
     * @param debug    Optional FILE* for debug output.
     */
    virtual void read_header(unsigned &inbits, unsigned &outbits, 
                             unsigned &minterms, FILE* debug = 0) = 0;

    /**
     * Reads a single minterm.
     * @param input_bits Buffer for the input bits.
     * @param out_terminal Output terminal character.
     * @return true if a minterm was read successfully, false otherwise.
     */
    virtual bool read_minterm(char* input_bits, char& out_terminal) = 0;

    /**
     * Runs a simple debug routine: reads header and minterms.
     */
    void debug(bool show_header, bool show_minterms, bool show_summary);
protected:
    inline bool eof() { return FR->eof(); }
    inline int get() { return FR->get(); }
    inline void unget(int c) { FR->unget(c); }
    inline unsigned read_unsigned() { return FR->read_unsigned(); }
    inline int skip_until(char x) {
        for (;;) {
            int c = get();
            if (x == c) return x;
            if (EOF == c) return EOF;
        }
    }
    inline unsigned read(void* ptr, unsigned bytes) {
        return FR->read(ptr, bytes);
    }
private:
    file_reader* FR;
};

//
// Front-end: parser creation functions
//

/**
 * Creates a parser for the given format and compression.
 * Format choices:
 *   'p' : PLA file
 *   'b' : BIN file (if applicable)
 * Compression choices:
 *   ' ' : none
 *   'x' : xz
 *   'g' : gzip
 *   'b' : bzip2
 */
parser* new_parser(char format, char compression);

/**
 * Creates a parser by auto-detecting format and compression from a pathname.
 */
parser* new_parser(const char* pathname);

/**
 * Creates a parser for a given pathname with specified format and compression.
 */
parser* new_parser(const char* pathname, char format, char compression);

#endif
