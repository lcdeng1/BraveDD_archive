#include "parser.h"

using namespace BRAVE_DD;

// ******************************************************************
// *                                                                *
// *                       ParserPla methods                        *
// *                                                                *
// ******************************************************************
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