
#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

//
// Abstract file reader, for compressed vs uncompressed input
//
class file_reader {
    FILE* F;
    bool pclose_F;
  public:
    /*
      @param  inpath    Input file name, or null to read from stdin
      @param  comp      Type of compression to use:
                          ' ': none
                          'x': xz  compression
                          'g': gz  compression
                          'b': bz2 compression
    */
    file_reader(const char* inpath, char comp);

    /*
      Close file properly
    */
    ~file_reader();

    inline bool fok() const {
      return F;
    }

    inline bool eof() {
      return feof(F);
    }

    inline int get() {
      return fgetc(F);
    }

    inline unsigned read_unsigned() {
      unsigned x;
      fscanf(F, "%u", &x);
      return x;
    }

    inline void unget(int c) {
      ungetc(c, F);
    }

    inline unsigned read(void* ptr, unsigned bytes) {
      return static_cast <unsigned>(fread(ptr, 1, bytes, F));
    }
};

//
// Abstract parser interface
//

class parser {
  public:
    parser(file_reader* fr);

    virtual ~parser();

    virtual void read_header(unsigned &inbits, unsigned &outbits, 
				unsigned &minterms, FILE* debug=0) = 0;
    // returns true on successful read, false otherwise
    virtual bool read_minterm(char* input_bits, char& out_terminal) = 0;

		void debug(bool show_header, bool show_minterms, bool show_summary);

  protected:
    inline bool eof() {
      return FR->eof();
    }

    inline int get() {
      return FR->get();
    }

    inline void unget(int c) {
      FR->unget(c);
    }

    inline unsigned read_unsigned() {
      return FR->read_unsigned();
    }

    inline int skip_until(char x) {
      for (;;) {
        int c = get();
        if (x == c)   return x;
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
// Front-end
//


/*
  Build a parser, reading from stdin, with specified
    file compression and file format.
  Format choices:
    'p': pla file
    'b': bin file
  Compression choices:
    ' ': none
    'x': xz
    'g': gz
    'b': bz2

  Returns null on error.
*/
parser* new_parser(char format, char compression);

/*
  Build a parser, reading from a pathname,

  Returns null on error.
*/
parser* new_parser(const char* pathname);

#endif

