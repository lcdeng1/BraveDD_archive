#include "parser.h"

using namespace BRAVE_DD;


// ******************************************************************
// *                                                                *
// *                                                                *
// *                     FileReader  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************
FileReader::FileReader(const char* inpath)
{
    if (!fileType(inpath)) {
        std::cout << "[BRAVE_DD] ERROR!\t FileReader(): Unknown file type or compress\n";
        exit(1);
    }
    if(compress == ' ') {
        closeNeed = false;
        if (!inpath) {
            infile = fopen(inpath, "r");
            if (!infile) {
                std::cout << "[BRAVE_DD] ERROR!\t FileReader(): Could not fopen file: " << inpath << std::endl;
                exit(1);
            }
        } else {
            infile = stdin;
        }
        return;
    }
    // now we will use popen
    closeNeed = true;
    
    std::string zcat = "";
    switch (compress)
    {
        case 'x': zcat += "xzcat";   break;
        case 'g': zcat += "gzcat";   break;
        case 'b': zcat += "bzcat";   break;
        default:    zcat += "cat";
    }

    if (!inpath) {
        infile = popen(zcat.c_str(), "r");
        return;
    }

    zcat += " ";
    zcat += inpath;
    infile = popen(zcat.c_str(), "r");
}

FileReader::~FileReader()
{
    if (infile) {
        if (closeNeed) {
            pclose(infile);
        } else {
            fclose(infile);
        }
    }
}

bool FileReader::matches(const char* ext, char fmt, char comp)
{
    int ext1length = 4;
    const char* ext1 = (fmt == 'p') ? ".pla" : (fmt == 'b') ? ".bin" : ".bddx";
    if (fmt == 'x') ext1length = 5;
    const char* ext2 = (comp == 'x') ? ".xz" : (comp == 'g') ? ".gz" : (comp == 'b') ? ".bz2" : "";
    return strncmp(ext1, ext, ext1length) == 0 && strcmp(ext2, ext + ext1length) == 0;
}

bool FileReader::fileType(const char* path)
{
    const char* formats = "pbx";
    const char* comps = " xgb";
    for (size_t i=0; path[i]; i++) {
        if (path[i] == '.') {
            for (size_t f=0; formats[f]; f++) {
                format = formats[f];
                for (size_t c=0; comps[c]; c++) {
                    compress = comps[c];
                    if (matches(path + i, format, compress)) return true;
                }
            }
        }
    }
    return false;
}

// ******************************************************************
// *                                                                *
// *                         Parser methods                         *
// *                                                                *
// ******************************************************************
Parser::Parser(FileReader* FR) : reader(FR) {
    //
}

Parser::~Parser() {
    // 
}

// ******************************************************************
// *                                                                *
// *                       ParserPla methods                        *
// *                                                                *
// ******************************************************************
ParserPla::ParserPla(FileReader* FR) : Parser(FR) {
    if (!FR->fok() || (FR->getFormat() != 'p')) {
        std::cout << "[BRAVE_DD] ERROR!\t ParserPla(): Unexpected file format.\n";
        exit(1);
    }
    inbits = 0;
    outbits = 0;
    numf = 0;
}
ParserPla::~ParserPla()
{
    //
}

void ParserPla::readHeader()
{
    int next = 0;
    for (;;) {
        // read lines while first character is '.'
        next = get();
        if (next == EOF) {
            std::cout << "[BRAVE_DD] ERROR!\t ParserPla::readHeader(): Unexpected EOF.\n";
            break;
        }
        if (next == '#') {
            skipUntil('\n');
            continue;
        }
        if (next != '.') {
            unget(next);
            break;
        }
        next = get();
        if (next == 'i') {
            inbits = readUnsigned();
        }
        if (next == 'o') {
            outbits = readUnsigned();
        }
        if (next == 'p') {
            numf = readSize();
        }
        skipUntil('\n');
    }
}

bool ParserPla::readAssignment(std::vector<bool>& inputs, char& out)
{
    int c;
    while (true) {
        c = get();
        if (c == EOF) return false;
        if (c == '.') {
            skipUntil('\n');
        } else {
            break;
        }
    }
    // read inputs
    inputs[0] = static_cast<bool>(c-'0');
    unsigned n;
    for (n=1; n<inbits; n++) {
        c = get();
        if (c == EOF) {
            std::cout << "[BRAVE_DD] ERROR!\t ParserPla::readAssignment(): Unexpected EOF.\n";
            return false;
        }
        inputs[n] = static_cast<bool>(c-'0');
    }
    // read out
    // inputs[n] = 0;
    unsigned t = 0;
    while ((c = get()) != '\n' && c != EOF) {
        if (c == '1') t = (t << 1) | 1;
        else if (c == '~') t <<= 1;
    }
    out = '0' + t;
    return true;
}
bool ParserPla::readAssignment(std::vector<bool>& inputs, int& out)
{
    int c;
    while (true) {
        c = get();
        if (c == EOF) return false;
        if (c == '.') {
            skipUntil('\n');
        } else {
            break;
        }
    }
    // read inputs
    inputs[0] = static_cast<bool>(c-'0');
    unsigned n;
    for (n=1; n<inbits; n++) {
        c = get();
        if (c == EOF) {
            std::cout << "[BRAVE_DD] ERROR!\t ParserPla::readAssignment(): Unexpected EOF.\n";
            return false;
        }
        inputs[n] = static_cast<bool>(c-'0');
    }
    // read out
    // inputs[n] = 0;
    int t = 0;
    while ((c = get()) != '\n' && c != EOF) {
        if (c == '1') t = (t << 1) | 1;
        else if (c == '~') t <<= 1;
    }
    out = t;
    return true;
}