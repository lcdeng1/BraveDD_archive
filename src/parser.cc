#include "parser.h"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <cstring>

// ---------------------------------------------------------------------
// file_reader implementation
// ---------------------------------------------------------------------
file_reader::file_reader(const char* inpath, char comp) {
    if (comp != 'x' && comp != 'g' && comp != 'b')
        comp = ' '; // no compression

    if (comp == ' ') {
        pclose_F = false;
        F = inpath ? fopen(inpath, "r") : stdin;
        return;
    }

    pclose_F = true;
    const char* decompressor = nullptr;
    switch (comp) {
        case 'x': decompressor = "xz -dc";  break;
        case 'g': decompressor = "gunzip -c"; break;
        case 'b': decompressor = "bzip2 -dc"; break;
        default:  decompressor = "cat"; break;
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s %s", decompressor, inpath);
    F = popen(buffer, "r");
}

file_reader::~file_reader() {
    if (F) {
        if (pclose_F)
            pclose(F);
        else
            fclose(F);
    }
}

// ---------------------------------------------------------------------
// parser implementation
// ---------------------------------------------------------------------
parser::parser(file_reader* fr) : FR(fr) {
    if (!fr) {
        throw std::runtime_error("Null file_reader passed to parser constructor");
    }
}

parser::~parser() {
    delete FR;
}

void parser::debug(bool show_header, bool show_minterms, bool show_summary) {
    unsigned inbits, outbits, minterms;
    read_header(inbits, outbits, minterms, stdout);
    fprintf(stdout, "Debug: inbits=%u, outbits=%u, minterms=%u\n", inbits, outbits, minterms);

    if (show_header) {
        fprintf(stdout, "Header information printed above.\n");
    }
    
    char* input_bits = new char[inbits + 1];
    char out_terminal;
    while (read_minterm(input_bits, out_terminal)) {
        if (show_minterms)
            fprintf(stdout, "Minterm: %s => %c\n", input_bits, out_terminal);
    }
    
    if (show_summary)
        fprintf(stdout, "Finished reading PLA file.\n");

    delete[] input_bits;
}

// ---------------------------------------------------------------------
// Dummy implementation of a PLA parser (pla_parser)
// ---------------------------------------------------------------------
#include <cstdio>

class pla_parser : public parser {
    unsigned inbits;
    unsigned outbits;
public:
    pla_parser(file_reader* fr) : parser(fr), inbits(0), outbits(0) { }
    virtual ~pla_parser() {}
    
    virtual void read_header(unsigned &inbits, unsigned &outbits, unsigned &minterms, FILE* debug = 0) override {
        // Dummy implementation: set fixed values
        inbits = 2;
        outbits = 1;
        minterms = 2;
        if (debug)
            fprintf(debug, "Dummy PLA Header: inbits=%u, outbits=%u, minterms=%u\n", inbits, outbits, minterms);
        this->inbits = inbits;
        this->outbits = outbits;
    }
    
    virtual bool read_minterm(char* input_bits, char& out_terminal) override {
        // Dummy implementation: return false to indicate no more minterms.
        return false;
    }
};

// ---------------------------------------------------------------------
// new_parser front-end functions
// ---------------------------------------------------------------------
parser* new_parser(char format, char compression) {
    // Create a parser that reads from stdin.
    file_reader* fr = new file_reader(nullptr, compression);
    if (!fr->fok()) {
        delete fr;
        return nullptr;
    }
    switch (format) {
        case 'p':
            return new pla_parser(fr);
        // Add other formats if needed.
        default:
            delete fr;
            return nullptr;
    }
}

parser* new_parser(const char* pathname) {
    // Auto-detect format and compression from the file extension.
    char format = 'p';  // Default to PLA format.
    char compression = ' '; // Assume no compression by default.
    const char* ext = strrchr(pathname, '.');
    if (ext != nullptr) {
        if (strcmp(ext, ".xz") == 0)
            compression = 'x';
        else if (strcmp(ext, ".gz") == 0)
            compression = 'g';
        else if (strcmp(ext, ".bz2") == 0)
            compression = 'b';
        else if (strcmp(ext, ".pla") == 0)
            compression = ' ';
    }
    return new_parser(pathname, format, compression);
}

parser* new_parser(const char* pathname, char format, char compression) {
    file_reader* fr = new file_reader(pathname, compression);
    if (!fr->fok()) {
        delete fr;
        return nullptr;
    }
    switch (format) {
        case 'p':
            return new pla_parser(fr);
        // Add additional cases here if other formats are supported.
        default:
            delete fr;
            return nullptr;
    }
}
