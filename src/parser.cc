#include "parser.h"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <lzma.h>

// ======================= File Reader Implementation =======================

file_reader::file_reader(const char* inpath, char comp) {
    if (comp != 'x' && comp != 'g' && comp != 'b')
        comp = ' '; // no compression

    if (comp == ' ') {
        pclose_F = false;
        if (inpath) {
            F = fopen(inpath, "r");
        } else {
            F = stdin;
        }
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

// Optional: XZ decompression using LZMA (not used in file_reader)
std::vector<uint8_t> decompress_xz(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Error opening file: " + filename);
    }
    std::vector<uint8_t> compressedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    lzma_stream strm = LZMA_STREAM_INIT;
    if (lzma_auto_decoder(&strm, UINT64_MAX, 0) != LZMA_OK) {
        throw std::runtime_error("Failed to initialize LZMA decoder.");
    }
    std::vector<uint8_t> decompressedData(compressedData.size() * 4);
    strm.next_in = compressedData.data();
    strm.avail_in = compressedData.size();
    strm.next_out = decompressedData.data();
    strm.avail_out = decompressedData.size();

    if (lzma_code(&strm, LZMA_FINISH) != LZMA_STREAM_END) {
        throw std::runtime_error("XZ decompression failed.");
    }
    decompressedData.resize(strm.total_out);
    lzma_end(&strm);
    return decompressedData;
}

// ======================= Parser Implementation =======================

parser::parser(file_reader* fr) : FR(fr) {
    if (!fr) {
        throw std::runtime_error("Null file_reader passed to parser constructor");
    }
}

parser::~parser() {
    delete FR;
}

void parser::debug(bool show_header, bool show_minterms, bool show_summary) {
    unsigned ib, ob, nmt;
    read_header(ib, ob, nmt, show_header ? stdout : nullptr);
    fprintf(stdout, "Debug: inbits=%u, outbits=%u, minterms=%u\n", ib, ob, nmt);

    if (show_header) {
        fprintf(stdout, "Header information printed above.\n");
    }
    
    char* minterm = new char[ib + 1];
    char term;
    unsigned count = 0;
    while (read_minterm(minterm, term)) {
        if (show_minterms) {
            fprintf(stdout, "Minterm: %s => %c\n", minterm, term);
        }
        count++;
    }
    if (show_summary) {
        fprintf(stdout, "Actual #minterms: %u\n", count);
    }
    delete[] minterm;
}

// ======================= PLA Parser Implementation =======================

pla_parser::pla_parser(file_reader* fr) : parser(fr), inbits(0), outbits(0) { }
pla_parser::~pla_parser() {}

// Reads the header directives from a PLA file.
void pla_parser::read_header(unsigned &ib, unsigned &ob, unsigned &nmt, FILE* debug) {
    ib = 0;
    ob = 0;
    nmt = 0;
    if (debug) {
        fprintf(debug, "Header Information:\n");
    }
    for (;;) {
        int next = get();
        if (EOF == next)
            throw std::runtime_error("Unexpected EOF while reading header.");
        if (next == '#') {
            skip_until('\n');
            continue;
        }
        if (next != '.') {
            unget(next);
            break;
        }
        next = get(); // Directive character
        if (next == 'i') {
            ib = read_unsigned();
            if (debug) fprintf(debug, "    .i %u\n", ib);
        } else if (next == 'o') {
            ob = read_unsigned();
            if (debug) fprintf(debug, "    .o %u\n", ob);
        } else if (next == 'p') {
            nmt = read_unsigned();
            if (debug) fprintf(debug, "    .p %u\n", nmt);
        }
        skip_until('\n');
    }
    inbits = ib;
    outbits = ob;
}

// Reads one minterm from the PLA file.
bool pla_parser::read_minterm(char* input_bits, char& out_terminal) {
    int c;
    // Skip lines that are directives (starting with '.') or comments.
    while (true) {
        c = get();
        if (EOF == c) return false;
        if (c == '.' || c == '#' || c == '\n') {
            if (c == '.') {
                // Check if it's the end directive.
                char directive[16];
                int pos = 0;
                while ((c = get()) != '\n' && c != EOF && pos < 15) {
                    directive[pos++] = static_cast<char>(c);
                }
                directive[pos] = '\0';
                if (strcmp(directive, "end") == 0)
                    return false;
            }
            continue;
        }
        break;
    }
    // Put back the first character of the minterm.
    unget(c);

    // Read exactly inbits characters for the input pattern.
    for (unsigned i = 0; i < inbits; ++i) {
        c = get();
        if (EOF == c) return false;
        input_bits[i] = static_cast<char>(c);
    }
    input_bits[inbits] = '\0';

    // Skip any whitespace.
    while ((c = get()) == ' ');

    // For this example, take the first non-whitespace character as the output token.
    out_terminal = static_cast<char>(c);
    skip_until('\n');
    return true;
}

// ======================= Parser Factory Functions =======================

// Factory: Create a parser for the given file (auto-detecting format and compression).
// This function examines the file extension to determine the format and compression.
static bool matches(const char* ext, char format, char comp) {
    const char* ext1 = (format == 'b') ? ".bin" : ".pla";
    const char* ext2 = (comp == 'x') ? ".xz" : (comp == 'g') ? ".gz" : (comp == 'b') ? ".bz2" : "";
    return strncmp(ext1, ext, strlen(ext1)) == 0 && strcmp(ext2, ext + strlen(ext1)) == 0;
}

static bool file_type(const char* pathname, char& format, char &comp) {
    const char* formats = "pb"; // 'p' for PLA, 'b' for BIN if needed
    const char* comps = " xgb"; // space for none, 'x' for xz, 'g' for gzip, 'b' for bzip2
    for (size_t i = 0; pathname[i]; ++i) {
        if (pathname[i] == '.') {
            for (size_t f = 0; formats[f]; f++) {
                format = formats[f];
                for (size_t c = 0; comps[c]; c++) {
                    comp = comps[c];
                    if (matches(pathname + i, format, comp))
                        return true;
                }
            }
        }
    }
    return false;
}

// Factory: Create a parser using a given pathname, format, and compression.
static parser* new_parser(const char* pathname, char format, char compression) {
    file_reader* fr = new file_reader(pathname, compression);
    if (!fr->fok()) {
        delete fr;
        return nullptr;
    }
    if (format == 'p')
        return new pla_parser(fr);
    return nullptr;
}

// Factory: Auto-detect the file type from the pathname.
parser* new_parser(const char* pathname) {
    char fmt, comp;
    if (!file_type(pathname, fmt, comp)) {
        std::cerr << "Couldn't determine file type for: " << pathname << std::endl;
        return nullptr;
    }
    return new_parser(pathname, fmt, comp);
}

// Factory: Create a parser for a given format and compression when no file is provided (e.g. reading from stdin).
parser* new_parser(char format, char compression) {
    file_reader* fr = new file_reader(nullptr, compression);
    if (!fr->fok()) {
        delete fr;
        return nullptr;
    }
    if (format == 'p')
        return new pla_parser(fr);
    return nullptr;
}
