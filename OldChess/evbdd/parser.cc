
#include "parser.h"
#include <string.h>

// #define DEBUG_INPUT_TYPE
// #define DEBUG_HEADER
// #define DEBUG_MINTERM
// #define NO_CURSED

//
// file_reader methods
//

file_reader::file_reader(const char* inpath, char comp)
{
  if ((comp != 'x') && (comp != 'g') && (comp != 'b')) {
    // unknown compression, so don't
    comp = ' ';
  }

  if (' ' == comp) {
    pclose_F = false;
    if (inpath) {
      F = fopen(inpath, "r");
    } else {
      F = stdin;
    }
    return;
  }

  pclose_F = true;

  const char* zcat = 0;
  switch (comp) {
      case 'x': zcat = "xzcat";   break;
      case 'g': zcat = "gzcat";   break;
      case 'b': zcat = "bzcat";   break;
      default:  zcat = "cat";
  };

  if (0==inpath) {
    F = popen(zcat, "r");
    return;
  }

  char buffer[256];
  snprintf(buffer, 256, "%s %s", zcat, inpath);
  F = popen(buffer, "r");
}

file_reader::~file_reader()
{
  if (F) {
    if (pclose_F) {
      pclose(F);
    } else {
      fclose(F);
    }
  }
}

//
// parser methods
//

parser::parser(file_reader* fr) 
{
  FR = fr;
  if (0==FR) throw "null file reader";
}

parser::~parser()
{
  delete FR;
}

void parser::debug(bool show_header, bool show_minterms, bool show_summary)
{
		unsigned ib, ob, nmt;
		read_header(ib, ob, nmt, show_header ? stdout : 0);

		if (show_summary) {
			printf("input  bits: %u\n", ib);
			printf("output bits: %u\n", ob);
			printf("expected #minterms: %u\n", nmt);
			fflush(stdout);
		}

		char* minterm = new char[ib+2];
		char term;
		unsigned i;
		for (i=0; read_minterm(minterm, term); ++i) {
				int traw = term - '0';
				if (show_minterms)
						printf("Minterm: %s => %c%c%c (terminal %c)\n",
								minterm,
								(traw & 4) ? '1' : '0',	
								(traw & 2) ? '1' : '0',	
								(traw & 1) ? '1' : '0',	
								term
						);
		}
		if (show_summary) {
				printf("actual   #minterms: %u\n", i);
		}
		delete[] minterm;
}


//
// pla parser class and methods
//

class pla_parser : public parser {
    unsigned inbits;
    unsigned outbits;
  public:
    pla_parser(file_reader* fr);
    virtual ~pla_parser();

    virtual void read_header(unsigned &inbits, unsigned &outbits, 
				unsigned &nmt, FILE* debug);
    virtual bool read_minterm(char* input_bits, char& out_terminal);
};

pla_parser::pla_parser(file_reader* fr) : parser(fr)
{
  inbits = 0;
  outbits = 0;
}

pla_parser::~pla_parser()
{
}

void pla_parser::read_header(unsigned &ib, unsigned &ob, 
		unsigned &nmt, FILE* debug)
{
  ib = 0;
  ob = 0;
  nmt = 0;

	if (debug) {
		fprintf(debug, "Header information: \n");
	}

  // Read lines while first character is .
  //
  for (;;) {
    int next = get();
    if (EOF == next) throw "Unexpected EOF.";
    if ('#' == next) {
      skip_until('\n');
      continue;
    }
    if ('.' != next) {
      unget(next);
      break; 
    }

    // we got .

    next = get();
    if ('i' == next) {
      ib = read_unsigned();
			if (debug) {
				fprintf(debug, "    .i %u\n", ib);
			}
    }
    if ('o' == next) {
      ob = read_unsigned();
			if (debug) {
				fprintf(debug, "    .o %u\n", ob);
			}
    }
    if ('p' == next) {
      nmt = read_unsigned();
			if (debug) {
				fprintf(debug, "    .p %u\n", nmt);
			}
    }
    skip_until('\n');
  }

  inbits = ib;
  outbits = ob;
}

inline bool is_ws(int x)
{
  return (' ' == x) || ('\t' == x) || ('\r' == x);
}

inline bool is_dash(int x)
{
  return ('-' == x) || ('2' == x);
}

inline unsigned outbit2value(int x)
{
  return ('1' == x) ? 1 : 0;
}


bool pla_parser::read_minterm(char* input_bits, char& out_terminal)
{
  int c;
  for (;;) {
    c = get();
    if (EOF == c) return false;
    if ('.' == c) {
      skip_until('\n');
#ifdef DEBUG_MINTERM
      fprintf(stderr, "skipped . line\n");
#endif
    }
    break;
  }
  // non . line, must be a minterm

  //
  // Read input bits
  //
  input_bits[0] = static_cast <char>(c);
  unsigned u;
  for (u=1; u<inbits; ++u) {
    c = get();
    if (EOF == c) return false;
    input_bits[u] = static_cast <char>(c);
  }
  input_bits[u] = 0;

  //
  // Read output bits and build terminal value
  //

  bool all_dash = true;
  unsigned t = 0;
#ifdef DEBUG_MINTERM
  char out[1+outbits];
  u = 0;
#endif
  for (;;) {
    c = get();
    if (EOF == c) return false;
    if ('\n' == c) break;
    if (is_ws(c)) continue;
#ifdef DEBUG_MINTERM
    out[u++] = c;
#endif
    t *= 2;
    t += outbit2value(c);
    if (!is_dash(c)) all_dash = false;
  }
  if (all_dash) {
    out_terminal = '0';
  } else {
    out_terminal = static_cast <char>('0' + t);
  }
  if (out_terminal - '0' == 6) {
    out_terminal = 5 + '0';
  }

#ifdef DEBUG_MINTERM
  out[u] = 0;
  fprintf(stderr, "Got line: %s => %s\n", input_bits, out);
#endif
  return true;
}


//
// bin parser class and methods
//

class bin_parser : public parser {
    unsigned inbits;
    unsigned long numf;

    unsigned long bits;
    unsigned bits_avail;
  public:
    bin_parser(file_reader* fr);
    virtual ~bin_parser();

    virtual void read_header(unsigned &inbits, unsigned &outbits, 
				unsigned &nmt, FILE* debug);
    virtual bool read_minterm(char* input_bits, char& out_terminal);

    inline bool read_le(unsigned long &u) {
      /*
      static unsigned char raw[8];
      unsigned actual = read(raw, 8);
      if (actual < 8) return false;
      const unsigned long u0 = raw[0];
      const unsigned long u1 = raw[1];
      const unsigned long u2 = raw[2];
      const unsigned long u3 = raw[3];
      const unsigned long u4 = raw[4];
      const unsigned long u5 = raw[5];
      const unsigned long u6 = raw[6];
      const unsigned long u7 = raw[7];

      u =    (u0      ) | (u1 <<  8) | (u2 << 16) | (u3 << 24) | 
             (u4 << 32) | (u5 << 40) | (u6 << 48) | (u7 << 56);

      return true;
      */
      return (8 == read(&u, 8));
    }

    inline void clear_bitstream() {
      bits_avail = 0;
    }

    inline void check_bitstream() {
      if (0==bits_avail) {
        if (!read_le(bits)) throw "unexpected eof";
        bits_avail = sizeof(unsigned long) * 8;
      }
    }

    inline bool read_bit2() {
      check_bitstream();
      unsigned long bp = bits & 0x03;
      bits >>= 2;
      bits_avail -= 2;
      switch (bp) {
        case 1:   return false;
        case 2:   return true;
        default:  throw "unexpected bit value";
      }
    }

    inline bool read_bit1() {
      check_bitstream();
      unsigned long bp = bits & 0x01;
      bits >>= 1;
      bits_avail -= 1;
      return bp;
    }
};

bin_parser::bin_parser(file_reader* fr) : parser(fr)
{
  inbits = 0;
  numf = 0;
}

bin_parser::~bin_parser()
{
}

void bin_parser::read_header(unsigned &ib, unsigned &ob, 
		unsigned &nmt, FILE* debug)
{
  ob = 3;   // Hard coded

  unsigned long numbits, numr, numdc;
  read_le(numbits);
  read_le(numf);  // member
  read_le(numr);
  read_le(numdc);
  nmt = static_cast <unsigned>(numf); 

  inbits = ib = static_cast <unsigned>((numbits-ob) / 2);

	if (debug) {
		fprintf(debug, "numbits: %lu\n", numbits);
		fprintf(debug, "numf   : %lu\n", numf);
		fprintf(debug, "numr   : %lu\n", numr);
		fprintf(debug, "numdc  : %lu\n", numdc);
	}
}

bool bin_parser::read_minterm(char* input_bits, char& out_terminal)
{
#ifndef DEBUG_MINTERM
  if (0==numf) return false;
#endif

  clear_bitstream();

  for (unsigned i=0; i<inbits; ++i) {
    input_bits[i] = '0' + ( read_bit2() ? 1 : 0 );
  }
  input_bits[inbits] = 0;

  const unsigned char w1 = read_bit1() ? 1 : 0;
  unsigned char w2 = read_bit1() ? 1 : 0;
  const unsigned char w3 = read_bit1() ? 1 : 0;
  if (w1 && w3) {
    out_terminal = '0';
  } else {
#ifdef NO_CURSED
    if ( w2 && (w1 || w3) ) {
      w2 = 0;
    }
#endif
    out_terminal = static_cast <char>('0' + w1*4 + w2*2 + w3);
    if (out_terminal - '0' == 6) {
        out_terminal = 5 + '0';
    }
  }

#ifdef DEBUG_MINTERM
	if (0==numf) {
		fprintf(stderr, "Ignored line: %s => %c%c%c (terminal %c)\n", input_bits, 
			w3 ? '1' : '0',
			w2 ? '1' : '0',
			w1 ? '1' : '0',
			out_terminal
		);
		return false;
	  }
  fprintf(stderr, "Got line: %s => %c%c%c (terminal %c)\n", input_bits, 
    w3 ? '1' : '0',
    w2 ? '1' : '0',
    w1 ? '1' : '0',
    out_terminal
  );
#endif
  numf--;
  return true; 
}

//
// Determine file type 
//

static bool matches(const char* ext, char format, char comp)
{
  const char* ext1;
  const char* ext2;
  unsigned ext1len;
  switch (format) {
    case 'b':   ext1 = ".bin";  ext1len = 4;  break;
    default:    ext1 = ".pla";  ext1len = 4;  break;
  }
  switch (comp) {
    case 'x':   ext2 = ".xz";   break;
    case 'g':   ext2 = ".gz";   break;
    case 'b':   ext2 = ".bz2";  break;
    default:    ext2 = "";      break;
  }

  if (strncmp(ext1, ext, ext1len)) return false;
  return 0 == strcmp(ext2, ext+ext1len);
}

static bool file_type(const char* pathname, char& format, char &comp)
{
  if (0==pathname) return false;
  const char* formats = "pb";
  const char* comps   = " xgb";
  unsigned i, f, c;
  for (i=0; pathname[i]; ++i) {
    if ('.' != pathname[i]) continue;
    
    for (f=0; formats[f]; f++) {
      format = formats[f];
      for (c=0; comps[c]; c++) {
        comp = comps[c];
        if (matches(pathname+i, format, comp)) return true;
      }
    }
  }
  return false;
}

static parser* new_parser(const char* pathname, char format, char compression)
{
#ifdef DEBUG_INPUT_TYPE
  fprintf(stderr, "Building parser %s '%c' '%c'\n", pathname, format, compression);
#endif
  file_reader* fr = new file_reader(pathname, compression);
  if (!fr->fok()) {
    delete fr;
    return 0;
  }
  
  switch (format) {
    case 'p':
    case 'P':
                return new pla_parser(fr);

    case 'b':
    case 'B':
                return new bin_parser(fr);

    default:
        fprintf(stderr, "No parser for format %c\n", format);
        return 0;
  }

}

//
// Front end
//


parser* new_parser(char format, char compression)
{
  return new_parser(0, format, compression);
}

parser* new_parser(const char* pathname)
{
  char fmt;
  char comp;
  if (!file_type(pathname, fmt, comp)) {
    fprintf(stderr, "Couldn't determine file type for input pathname %s\n", 
      pathname);
    return 0;
  }
  return new_parser(pathname, fmt, comp);
}

