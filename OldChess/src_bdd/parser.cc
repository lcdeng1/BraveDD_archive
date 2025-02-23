#include "parser.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <lzma.h>

// ======================= File Reader Implementation =======================

file_reader::file_reader(const char* inpath, char comp) {
    if ((comp != 'x') && (comp != 'g') && (comp != 'b')) {
        comp = ' '; // Default: no compression
    }

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
    const char* zcat = (comp == 'x') ? "xzcat" : (comp == 'g') ? "gzcat" : (comp == 'b') ? "bzcat" : "cat";

    if (!inpath) {
        F = popen(zcat, "r");
        return;
    }

    char buffer[256];
    snprintf(buffer, 256, "%s %s", zcat, inpath);
    F = popen(buffer, "r");
}

file_reader::~file_reader() {
    if (F) {
        if (pclose_F) {
            pclose(F);
        } else {
            fclose(F);
        }
    }
}

// ======================= Decompression for .xz files =======================

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

parser::parser(file_reader* fr) {
    FR = fr;
    if (!FR) throw "Null file reader";
}

parser::~parser() {
    delete FR;
}

void parser::debug(bool show_header, bool show_minterms, bool show_summary) {
    unsigned ib, ob, nmt;
    read_header(ib, ob, nmt, show_header ? stdout : nullptr);

    if (show_summary) {
        std::cout << "Input bits: " << ib << "\nOutput bits: " << ob << "\nMinterms: " << nmt << std::endl;
    }

    char* minterm = new char[ib + 2];
    char term;
    unsigned i;
    for (i = 0; read_minterm(minterm, term); ++i) {
        int traw = term - '0';
        if (show_minterms) {
            printf("Minterm: %s => %c%c%c (terminal %c)\n",
                   minterm,
                   (traw & 4) ? '1' : '0',
                   (traw & 2) ? '1' : '0',
                   (traw & 1) ? '1' : '0',
                   term);
        }
    }
    if (show_summary) {
        printf("Actual #minterms: %u\n", i);
    }
    delete[] minterm;
}

// ======================= PLA Parser Implementation =======================

pla_parser::pla_parser(file_reader* fr) : parser(fr) {
    inbits = 0;
    outbits = 0;
}

pla_parser::~pla_parser() {}

void pla_parser::read_header(unsigned &ib, unsigned &ob, unsigned &nmt, FILE* debug) {
    ib = 0;
    ob = 0;
    nmt = 0;

    if (debug) {
        fprintf(debug, "Header Information:\n");
    }

    for (;;) {
        int next = get();
        if (EOF == next) throw "Unexpected EOF.";
        if (next == '#') {
            skip_until('\n');
            continue;
        }
        if (next != '.') {
            unget(next);
            break;
        }

        next = get();
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

bool pla_parser::read_minterm(char* input_bits, char& out_terminal) {
    int c;
    while (true) {
        c = get();
        if (EOF == c) return false;
        if (c == '.') {
            skip_until('\n');
        } else {
            break;
        }
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
    while ((c = get()) != '\n' && c != EOF) {
        if (c == '1') t = (t << 1) | 1;
        else if (c == '0') t <<= 1;
    }
    out_terminal = '0' + t;
    return true;
}

// ======================= Parser Factory Functions =======================

static bool matches(const char* ext, char format, char comp) {
    const char* ext1 = (format == 'b') ? ".bin" : ".pla";
    const char* ext2 = (comp == 'x') ? ".xz" : (comp == 'g') ? ".gz" : (comp == 'b') ? ".bz2" : "";

    return strncmp(ext1, ext, 4) == 0 && strcmp(ext2, ext + 4) == 0;
}

static bool file_type(const char* pathname, char& format, char &comp) {
    const char* formats = "pb";
    const char* comps = " xgb";
    for (size_t i = 0; pathname[i]; ++i) {
        if (pathname[i] == '.') {
            for (size_t f = 0; formats[f]; f++) {
                format = formats[f];
                for (size_t c = 0; comps[c]; c++) {
                    comp = comps[c];
                    if (matches(pathname + i, format, comp)) return true;
                }
            }
        }
    }
    return false;
}

static parser* new_parser(const char* pathname, char format, char compression) {
    file_reader* fr = new file_reader(pathname, compression);
    if (!fr->fok()) {
        delete fr;
        return nullptr;
    }

    if (format == 'p') return new pla_parser(fr);
    return nullptr;
}

parser* new_parser(const char* pathname) {
    char fmt, comp;
    if (!file_type(pathname, fmt, comp)) {
        std::cerr << "Couldn't determine file type for: " << pathname << std::endl;
        return nullptr;
    }
    return new_parser(pathname, fmt, comp);
}
