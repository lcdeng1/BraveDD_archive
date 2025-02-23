#include "parser.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
bool file_type(const char* pathname, char& format, char &comp);

// **File Reader Constructor**: Handles Decompression Automatically
file_reader::file_reader(const char* inpath, char comp) {
    if ((comp != 'x') && (comp != 'g') && (comp != 'b')) {
        comp = ' '; // No compression
    }

    if (' ' == comp) {
        pclose_F = false;
        F = inpath ? fopen(inpath, "r") : stdin;
        return;
    }

    pclose_F = true;
    const char* decompressor = nullptr;
    switch (comp) {
        case 'x': decompressor = "xz -dc";  break;  // Correct command for `.xz`
        case 'g': decompressor = "gunzip -c"; break;
        case 'b': decompressor = "bzip2 -dc"; break;
        default:  decompressor = "cat";  // No compression
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s %s", decompressor, inpath);
    F = popen(buffer, "r");
}

// Destructor: Closes File Properly
file_reader::~file_reader() {
    if (F) {
        if (pclose_F) pclose(F);
        else fclose(F);
    }
}

// **Parser Constructor**
parser::parser(file_reader* fr) {
    FR = fr;
    if (!FR) throw "Null file reader!";
}

parser::~parser() {
    delete FR;
}

// **PLA Parser Implementation**
class pla_parser : public parser {
private:
    unsigned inbits;
    unsigned outbits;

public:
    pla_parser(file_reader* fr) : parser(fr) {
        inbits = 0;
        outbits = 0;
    }

    ~pla_parser() {}

    // Read PLA File Header
    void read_header(unsigned &ib, unsigned &ob, unsigned &nmt, FILE* debug) override {
        ib = 0;
        ob = 0;
        nmt = 0;

        if (debug) fprintf(debug, "Reading PLA Header...\n");

        for (;;) {
            int next = get();
            if (EOF == next) throw "Unexpected EOF in header!";
            if ('#' == next) {
                skip_until('\n');
                continue;
            }
            if ('.' != next) {
                unget(next);
                break;
            }

            next = get();
            if ('i' == next) ib = read_unsigned();
            if ('o' == next) ob = read_unsigned();
            if ('p' == next) nmt = read_unsigned();
            skip_until('\n');
        }

        inbits = ib;
        outbits = ob;
    }

    // Read PLA Minterm
    bool read_minterm(char* input_bits, char& out_terminal) override {
        int c;
        for (;;) {
            c = get();
            if (EOF == c) return false;
            if ('.' == c) {
                skip_until('\n');
                continue;
            }
            break;
        }

        input_bits[0] = static_cast<char>(c);
        unsigned u;
        for (u = 1; u < inbits; ++u) {
            c = get();
            if (EOF == c) return false;
            input_bits[u] = static_cast<char>(c);
        }
        input_bits[u] = 0;

        unsigned t = 0;
        for (;;) {
            c = get();
            if (EOF == c || c == '\n') break;
            if (c == ' ') continue;
            t = (t * 2) + ((c == '1') ? 1 : 0);
        }

        out_terminal = static_cast<char>('0' + t);
        return true;
    }
};

// **Create New Parser**
parser* new_parser(const char* pathname, char format, char compression) {
    file_reader* fr = new file_reader(pathname, compression);
    if (!fr->fok()) {
        delete fr;
        return nullptr;
    }

    switch (format) {
        case 'p': return new pla_parser(fr);  // PLA file parser
        default: delete fr; return nullptr;
    }
}

// Detect File Type
bool file_type(const char* pathname, char& format, char &comp) {
    if (!pathname) return false;
    if (strstr(pathname, ".pla.xz")) { format = 'p'; comp = 'x'; return true; }
    if (strstr(pathname, ".pla.gz")) { format = 'p'; comp = 'g'; return true; }
    if (strstr(pathname, ".pla.bz2")) { format = 'p'; comp = 'b'; return true; }
    if (strstr(pathname, ".pla")) { format = 'p'; comp = ' '; return true; }
    return false;
}
