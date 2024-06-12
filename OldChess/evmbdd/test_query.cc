#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "fixed_dd.h"
#include "parser.h"

using namespace std;

#include <sys/time.h>
#include <time.h>

inline long get_time_in_us()
{
  struct timeval the_time;
  gettimeofday(&the_time, 0);
  return the_time.tv_sec * 1000000 + the_time.tv_usec;
}

inline void show_elapsed(const char* what, const char* avg, long time, unsigned nmt)
{
  const long elapsed_sec = time / 1000000;
  const long elapsed_usec = time % 1000000;
  long double res = static_cast<long double>(time) / 1000000;
  fprintf(stderr, "%s took %ld.%06ld seconds\n", what,
    elapsed_sec, elapsed_usec);
  fprintf(stderr, "%s took %Lf microseconds\n", avg,
    res/nmt*1000000);
}

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

static int next_useful(char* inn)
{
  for (unsigned i=0;;i++) {
  	// printf("c: %s", inn);
    int c = static_cast<int>(inn[0]);
    if ('.' == c) {
      continue;
    }
    if (' ' == c)   continue;
    if ('\n' == c)  continue;
    if ('\t' == c)  continue;
    if ('\r' == c)  continue;
    return c;
  }
}

static bool get_query(char* inn, bool* vars, unsigned numvars)
{
  int c = next_useful(inn);
  if (EOF == c) return false;
  vars[1] = (c != '0');
  for (unsigned u=2; u<=numvars; u++) {
  	// printf("c: %u", c);
    vars[u] = ( inn[u-2] != '0' );
  }
  return true;
}

static void show_query(std::ostream &out, const bool* vars, unsigned numvars, unsigned* term)
{
  for (unsigned u=1; u<=numvars; u++) {
    out << ( vars[u] ? "1" : "0" );
  }
  out << " " << term[0] << "," << term[1] << "\n";
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
		

		infile = argv[1];

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
		unsigned ib, ob, nmt;
		P->read_header(ib, ob, nmt, show_header ? stdout : 0);

		if (show_summary) {
			printf("input  bits: %u\n", ib);
			printf("output bits: %u\n", ob);
			printf("expected #minterms: %u\n", nmt);
			fflush(stdout);
		}

		char* minterm = new char[ib+2];
		char** min_arr = new char*[nmt];
		char term;
		unsigned i;
		for (i=0; P->read_minterm(minterm, term); ++i) {
				int traw = term - '0';
				if (show_minterms)
						min_arr[i] = new char[ib+2];
          	strcpy(min_arr[i], minterm);
		}
		if (show_summary) {
				printf("actual   #minterms: %u\n", i);
		}
		delete[] minterm;

    fixed_dd BDD;
      unsigned* root_vals = BDD.add_nodes_from_file(argv[2]);
      unsigned root = root_vals[0];
      unsigned root_val = root_vals[1];
    #ifdef WITH_SWAPS
      bool root_swap = root_vals[2];
    #else
      bool root_swap = false;
    #endif

		BDD.done_building();
//    std::cerr << "Got root " << root << "\n";
		std::cerr << "Read BDD with " << BDD.num_nonterminals() << " non-terminal nodes\n";
		std::cerr << "Bytes required: " << BDD.bytes_required() << "\n";
		std::cerr << "Enter queries, one per line, eof to stop\n";

    bool* vars = new bool[BDD.num_levels()+1];
    long start_time = get_time_in_us();
    for (unsigned i = 0;i<nmt;i++) {
        if (get_query(min_arr[i], vars, BDD.num_levels()))
			    {
			      show_query(std::cout, vars, BDD.num_levels(), BDD.evaluate(root, vars, root_val, root_swap));
			    }
    }
    show_elapsed("Querying took", "Average query time", get_time_in_us() - start_time, nmt);
    delete[] min_arr;
    delete[] vars;
		std::cerr << "Done\n";


    delete P;
		return 0;
}
