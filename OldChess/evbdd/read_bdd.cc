
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "fixed_dd.h"

static void skip_to_eol(std::istream &in)
{
  for (;;) {
    int c = in.get();
    if (EOF == c)   return;
    if ('\n' == c)  return;
  }
}

static int next_useful(std::istream &in)
{
  for (;;) {
    int c = in.get();
    if ('.' == c) {
      skip_to_eol(in);
      continue;
    }
    if (' ' == c)   continue;
    if ('\n' == c)  continue;
    if ('\t' == c)  continue;
    if ('\r' == c)  continue;
    return c;
  }
}

static bool get_query(std::istream &in, bool* vars, unsigned numvars)
{
  int c = next_useful(in);
  if (EOF == c) return false;
  vars[1] = (c != '0');
  for (unsigned u=2; u<=numvars; u++) {
    vars[u] = ( in.get() != '0' );
  }
  skip_to_eol(in);
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
  if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " infile\n";
    return EXIT_FAILURE;
  }

  // try {
    fixed_dd BDD;
    unsigned* root_vals = BDD.add_nodes_from_file(argv[1]);
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

    while (get_query(std::cin, vars, BDD.num_levels()))
    {
      show_query(std::cout, vars, BDD.num_levels(), BDD.evaluate(root, vars, root_val, root_swap));
    }

    delete[] vars;
		std::cerr << "Done\n";
  // }
	/*
  catch (const char* error) {
		std::cerr << "Error: " << error << "\n";
    return 1;
  }
	*/

  return 0; 
}
