#include "brave_dd.h"

using namespace BRAVE_DD;

static int usage(const char* exe) {
    std::cerr << "Usage: " << exe << " (options) (infile)\n\n"
              << "Options:\n"
              << "  -h : show header\n"
              << "  -m : show minterms\n";
            //   << "  -s : show summary\n\n";
    return 1;
}

int main(int argc, const char** argv) {
    bool showHeader   = false;
    bool showMinterms = false;
    // bool showSummary  = false;
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
                case 'h': showHeader   = true;  break;
                case 'm': showMinterms = true;  break;
                // case 's': showSummary  = true;  break;
                default : return usage(argv[0]);
            }
        }
    }

    // 2) Open and parse the file (or stdin if none provided)
    FileReader FR(infile);
    ParserPla parserPla(&FR);

    // 3) Run the parser’s debug logic
    parserPla.readHeader();
    if (showHeader) {
        std::cout << "Number of inbits: " << parserPla.getInBits() << std::endl;
        std::cout << "Number of outbits: " << parserPla.getOutBits() << std::endl;
        std::cout << "Number of asmts: " << parserPla.getNum() << std::endl;
    }
    std::vector<char> assignment(parserPla.getInBits());
    char out = 0;
    for (;;) {
        if (!parserPla.readAssignment(assignment, out)) break;
        if (showMinterms) {
            for (size_t i=0; i<assignment.size(); i++) {
                std::cout << assignment[i] << " ";
            }
            std::cout << "\t" << out << std::endl;
        }
    }

    return 0;
}
