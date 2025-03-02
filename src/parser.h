#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>


class file_reader {
public:
   
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


class parser {
public:
    parser(file_reader* fr);
    virtual ~parser();

   
    virtual void read_header(unsigned &ib, unsigned &ob, unsigned &minterms, FILE* debug = 0) = 0;

  .
    virtual bool read_minterm(char* input_bits, char& out_terminal) = 0;

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


parser* new_parser(const char* pathname);

parser* new_parser(char format, char compression);

#endif
