#include "parser.h"
#include <iostream>
#include <stdexcept>
#include <filesystem>  // Include C++ filesystem library for path handling

using namespace std;
namespace fs = std::filesystem;  // Use filesystem namespace for path handling

static int usage(const char* exe) {
    cerr << "Usage: " << exe << " (options) (infile)\n";
    cerr << "\nOptions:\n";
    cerr << "\t -h: don't show header\n";
    cerr << "\t -H: show header\n";
    cerr << "\t -m: don't show minterms\n";
    cerr << "\t -M: show minterms\n";
    cerr << "\t -s: don't show summary\n";
    cerr << "\t -S: show summary\n";
    cerr << "\n";
    return 1;
}

int main(int argc, const char** argv) {
    //
    // Process command line arguments
    //
    bool show_header = true;
    bool show_minterms = true;
    bool show_summary = true;
    const char* infile = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            if (infile) return usage(argv[0]);
            infile = argv[i];
            continue;
        }
        if (argv[i][1] == 0 || argv[i][2] != 0) return usage(argv[0]);

        switch (argv[i][1]) {
            case 'h': show_header = false; break;
            case 'H': show_header = true; break;
            case 'm': show_minterms = false; break;
            case 'M': show_minterms = true; break;
            case 's': show_summary = false; break;
            case 'S': show_summary = true; break;
            default: return usage(argv[0]);
        }
    }

    //
    // Validate and Open Input File
    //
    parser* P = nullptr;

    try {
        if (infile) {
            // Convert to absolute path if needed
            fs::path filePath = fs::absolute(infile);

            // Check if the file exists
            if (!fs::exists(filePath)) {
                throw runtime_error("Error: File not found -> " + filePath.string());
            }

            P = new_parser(filePath.string().c_str());
            if (!P) throw runtime_error("Couldn't open input file: " + filePath.string());
            cerr << "Input from: " << filePath << "\n";

        } else {
            P = new_parser('p', ' ');  // Default PLA file parsing
            if (!P) throw runtime_error("Couldn't initialize parser.");
            cerr << "Input from stdin\n";
        }

        P->debug(show_header, show_minterms, show_summary);
        delete P;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
