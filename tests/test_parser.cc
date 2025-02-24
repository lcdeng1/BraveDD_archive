#include "src/parser.h"
#include <iostream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

static int usage(const char* exe) {
    std::cerr << "Usage: " << exe << " (options) (infile)\n\n"
              << "Options:\n"
              << "  -h : don't show header\n"
              << "  -H : show header\n"
              << "  -m : don't show minterms\n"
              << "  -M : show minterms\n"
              << "  -s : don't show summary\n"
              << "  -S : show summary\n\n";
    return 1;
}

int main(int argc, const char** argv) {
    bool show_header   = true;
    bool show_minterms = true;
    bool show_summary  = true;
    const char* infile = nullptr;

    // 1) Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        // If it's not a flag, treat it as the input file
        if (argv[i][0] != '-') {
            if (infile) {
                // We already have an infile; too many positional args
                return usage(argv[0]);
            }
            infile = argv[i];
        }
        else {
            // Expect single-letter flags, e.g. "-h" or "-M"
            if (argv[i][1] == 0 || argv[i][2] != 0) {
                // Invalid flag format
                return usage(argv[0]);
            }
            switch (argv[i][1]) {
                case 'h': show_header   = false; break;
                case 'H': show_header   = true;  break;
                case 'm': show_minterms = false; break;
                case 'M': show_minterms = true;  break;
                case 's': show_summary  = false; break;
                case 'S': show_summary  = true;  break;
                default : return usage(argv[0]);
            }
        }
    }

    // 2) Open and parse the file (or stdin if none provided)
    parser* P = nullptr;
    try {
        if (infile) {
            fs::path filePath = fs::absolute(infile);

            // Check if file exists
            if (!fs::exists(filePath)) {
                throw std::runtime_error(
                    "Error: File not found -> " + filePath.string()
                );
            }

            // Ignore ".bin" files
            if (filePath.extension() == ".bin") {
                throw std::runtime_error(
                    "Ignoring '.bin' files -> " + filePath.string()
                );
            }

            // Attempt to create the parser
            P = new_parser(filePath.string().c_str());
            if (!P) {
                throw std::runtime_error(
                    "Couldn't open input file: " + filePath.string()
                );
            }
            std::cerr << "Input from: " << filePath << "\n";
        }
        else {
            // No file given; default to reading from stdin as a PLA
            P = new_parser('p', ' ');
            if (!P) {
                throw std::runtime_error("Couldn't initialize parser for stdin.");
            }
            std::cerr << "Input from stdin\n";
        }

        // 3) Run the parser’s debug logic
        P->debug(show_header, show_minterms, show_summary);

        // Clean up
        delete P;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        delete P;  // Make sure we clean up if parser was partially constructed
        return 1;
    }

    return 0;
}
