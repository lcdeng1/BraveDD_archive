
#include "parser.h"

#include <iostream>

using namespace std;

static int usage(const char* exe)
{
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

int main(int argc, const char** argv)
{
		//
		// Process command line arguments
		//

		bool show_header = true;
		bool show_minterms = true;
		bool show_summary = true;
		const char* infile = 0;

		for (int i=1; i<argc; ++i) {
			if ('-' != argv[i][0]) {
				if (infile) return usage(argv[0]);
				infile = argv[i];
				continue;
			}
			if (0==argv[i][1]) return usage(argv[0]);
			if (0!=argv[i][2]) return usage(argv[0]);
			switch (argv[i][1]) {
					case 'h':		show_header = false;		continue;
					case 'H':		show_header = true;			continue;
					case 'm':		show_minterms = false;	continue;
					case 'M':		show_minterms = true;		continue;
					case 's':		show_summary = false;		continue;
					case 'S':		show_summary = true;		continue;
					default:		return usage(argv[0]);
			}
		}

    //
    // Open input file
    //
    parser* P;
    if (infile) {
      P = new_parser(infile);
      cerr << "Input from " << infile << "\n";
    } else {
      P = new_parser('p', ' '); // TBD option for this
      cerr << "Input from stdin\n";
    }
    if (0==P) throw "Couldn't open input file.";

		P->debug(show_header, show_minterms, show_summary);

    delete P;
		return 0;
}
