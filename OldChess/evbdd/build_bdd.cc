
//
// See http://www.ecs.umass.edu/ece/labs/vlsicad/ece667/links/espresso.5.html
//

#include "parser.h"
#include "bdd.h"
#include "buffer.h"
#include "ct.h"

#include "resolv.h"
#include "switches.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#define SHOW_TIMING

using namespace std;

// concretization algorithms

const unsigned NONE			= 0;
const unsigned RESTR		= 1;
const unsigned OSM			= 2;
const unsigned TSM			= 3;
const unsigned NUMALGS	= 4;

static enum_arg::value ALGOS[NUMALGS];


// encodings

const unsigned MTERM		= 0;
const unsigned BTERM		= 1;
const unsigned DCBIN		= 2;
const unsigned NUMENCS	= 3;

static enum_arg::value ENCODINGS[NUMENCS];

struct switches {
  const char* reorder;      // reorder string 
  const char* reord_file;   // reorder filename
  const char* input_file;   // input filename
  const char* verify_file;  // verify filename
  const char* write_file;   // write filename
  const char* permutation; //permutation sequence
  unsigned bufsize;         // size of buffer when building from minterms
  unsigned gcount;          // number of times to run GC while building
	unsigned gtimer;
  unsigned ctsize;          // compute table size
  char show_progress;       // show progress while reading input?
  char show_minterms;       // display minterms when done?
  char use_swap_bit;        // use swap bit when encoding?
	unsigned encoding;				// encoding scheme to use
  unsigned DALG;            // algorithm for undefined, during
  unsigned Dlevel;          // level number for triggering resolver, during
  unsigned UALG;            // algorithm for resolving undefined, after
  int Ubatch;               // batch size for UALG; 0 means do all levels.
};



static resolver* new_resolver(unsigned type, bdd_forest &bdd, compute_table &CT)
{
  switch (type) {
      case RESTR:
            return restrict_mt(bdd, CT);

      case OSM:
            return one_side_match_mt(bdd, CT);

      case TSM:
            return two_side_match_mt(bdd, CT);

      default:  // none
            return 0;
  }
}

// **********************************************************************
//
// Fancy output
// 
// **********************************************************************


static void report(unsigned long value, const char* item)
{
  bool printed = false;
  fprintf(stderr, "  ");
  unsigned long expo = 1000;
  expo *= 1000;
  expo *= 1000;
  expo *= 1000;
  while (expo>1) {
    unsigned long digits = value / expo;
    if (printed) {
      fprintf(stderr, ",%03lu", digits);
      value %= expo;
    } else {
      if (digits) {
        fprintf(stderr, " %3lu", digits);
        value %= expo;
        printed = true;
      } else {
        fprintf(stderr, "    ");
      }
    }
    expo /= 1000;
  }
  if (printed) {
    fprintf(stderr, ",%03lu", value);
  } else {
    fprintf(stderr, " %3lu", value);
  }
  if (item) {
    fprintf(stderr, "  %s", item);
  } 
}

// **********************************************************************
//
// Helpers for various switches 
// 
// **********************************************************************

static unsigned* build_reorder(unsigned nbits, istream &in)
{
  unsigned* map = new unsigned[nbits+1];
  map[0] = 0; // always!
  unsigned level;
  in >> level;
  map[1] = level;
  for (unsigned b=2; b<=nbits; b++) {
    char comma;
    in >> comma;
    if (comma != ',') throw "Expected , in reorder list";
    in >> level;
    map[b] = level;
  }

  return map;
}

static void check_reorder(unsigned nbits, const unsigned* map)
{
  bool* saw_value = new bool[nbits+1];
  for (unsigned i=0; i<=nbits; i++) saw_value[i] = false;

  if (map[0] != 0) throw "Invalid map[0], how?";

  for (unsigned i=1; i<=nbits; i++) {
    if ((map[i] < 1) || (map[i] > nbits)) {
      throw "Invalid level in reorder";
    }
    if (saw_value[map[i]]) throw "Duplicate level in reorder";
    saw_value[map[i]] = true;
  }

  delete[] saw_value;
}

// **********************************************************************
//
// Timing stuff
//
// **********************************************************************

#ifdef SHOW_TIMING

#include <sys/time.h>
#include <time.h>

inline long get_time_in_us()
{
  struct timeval the_time;
  gettimeofday(&the_time, 0);
  return the_time.tv_sec * 1000000 + the_time.tv_usec;
}

inline void show_elapsed(const char* what, long time)
{
  const long elapsed_sec = time / 1000000;
  const long elapsed_usec = time % 1000000;
  fprintf(stderr, "%s took %ld.%06ld seconds\n", what,
    elapsed_sec, elapsed_usec);
}

#else
inline long get_time_in_us()
{
  return 0;
}
inline void show_elapsed(const char*, long)
{
}

#endif

// **********************************************************************
//
// Build MT BDD.
//
// **********************************************************************

static void build_mt(parser* P, const switches &S, bdd_forest &bdd, compute_table &CT, bdd_edge &root)
{
  char* linebuf = new char[5+bdd.get_num_vars()];

  resolver* R = new_resolver(S.DALG, bdd, CT);
  if (R) R->set_levels(static_cast <unsigned short>(S.Dlevel), 1);
  if (S.DALG) fprintf(stderr, "Concretizing using '%s' above level %u\n", 
    ALGOS[S.DALG].get_str(),
    S.Dlevel
  );

  //
  // Build buffer (TBD: for -r, build a buffer for each range value)
  //
  fprintf(stderr, "Using buffer of size %u\n", S.bufsize);
  minterm_buffer B(bdd, root, S.bufsize, R);

  //
  // Build BDD from minterms
  //
  fprintf(stderr, "Building BDD...\n");
  fflush(stderr);

  long start_time = get_time_in_us();

  unsigned num_gcs = 0;
  unsigned unions = 0;
	unsigned gtimer = S.gtimer;
  for (;;) {
    char term;
    if (! P->read_minterm(linebuf, term)) break;
    if ('0' == term) continue;

    if (S.permutation) {
        int index = term - '0';
        term = S.permutation[index - 1] + 1;
    }

    unions += B.add(linebuf, term);

		if (--gtimer) continue;
		fprintf(stderr, "  gc at %lu minterms\n", B.get_num_minterms());
		gtimer = S.gtimer;
    ++num_gcs;
    bdd.mark_and_sweep(root);
  } // for loop

  //
  // Add any remaining buffer
  //
  B.flush();
  ++unions;
  ++num_gcs;
  //root = bdd.reduce(root);
  unsigned count = bdd.mark_and_sweep(root);

  show_elapsed("BDD construction", get_time_in_us() - start_time);

  report(B.get_num_minterms(), "minterms\n");
  report(bdd.peak_nodes(), "peak BDD nodes\n");
  report(bdd.peak_nodes() * sizeof(bdd_node), "peak BDD bytes\n");
  report(count, "final BDD nodes\n");
  report(count * sizeof(bdd_node), "final BDD bytes\n");
  report(unions, "unions\n");
  report(num_gcs, "garbage collections\n");
  if (R) report(R->num_calls(), "resolutions\n");
  fflush(stderr);

	/*
	unsigned low[16], high[16];
	count = bdd.count_pattern(low, high);

	for (unsigned t=0; t<bdd.get_num_terminals(); t++) {
		if (low[t]) {
			report(low[t], " low ");
			fprintf(stderr, "%u nodes\n", t);
		}
	}	
	for (unsigned t=0; t<bdd.get_num_terminals(); t++) {
		if (high[t]) {
			report(high[t], " high ");
			fprintf(stderr, "%u nodes\n", t);
		}
	}	
	report(count, "  nodes\n");
	*/

  delete[] linebuf;
  delete R;
}

// **********************************************************************
//
// Concretize BDD(s) with "don't care" as value 0
//
// **********************************************************************

static void concretize(const switches &S, bdd_forest &bdd, compute_table &CT, bdd_edge* roots, unsigned nroots)
{
  long start_time = get_time_in_us();

  resolver* R = new_resolver(S.UALG, bdd, CT);
	if (0==R) return;	// Nothing to do

  unsigned count = 0;
  unsigned short i;
  unsigned short topL = static_cast <unsigned short>(bdd.get_num_vars());
  unsigned short botL = 1;

  fprintf(stderr, "Concretizing using '%s'", ALGOS[S.UALG].get_str());
  if (S.Ubatch>0) {
    fprintf(stderr, ", bottom up, batch size %d", S.Ubatch);
    topL = static_cast <unsigned short>(S.Ubatch);
  } 
  if (S.Ubatch<0) {
    fprintf(stderr, ", top down, batch size %d", -S.Ubatch);
    botL = topL + static_cast <unsigned short>(S.Ubatch) + 1;
  }
  fprintf(stderr, "...\n");
  fflush(stderr);
	for (unsigned ri=0; ri<nroots; ++ri) {
			if (0==roots[ri].node) continue;
			fprintf(stderr, "    BDD #%u\n", ri);
			for (i=1; ; ++i) {
				if (R) {
					R->set_levels(topL, botL);
					roots[ri].node = R->invoke(roots[ri]);
				} 
				count = bdd.mark_and_sweep(roots, nroots);
				fprintf(stderr, "\tIter %3u, levels %3d to %3d.  ", i, topL, botL);
				report(count, " nodes\n");
				if (0==S.Ubatch) break;
				topL += S.Ubatch;
				botL += S.Ubatch;
				if (botL > bdd.get_num_vars()) break;
				if (topL > bdd.get_num_vars()) {
					topL = static_cast <unsigned short>(bdd.get_num_vars());
				}
				if (topL < 1) break;
				if (botL < 1) botL = 1;
			}
	}

  show_elapsed("BDD concretization", get_time_in_us() - start_time);

  report(count, "nodes in concretized BDD\n");
  report(count*sizeof(bdd_node), "bytes for concretized BDD\n");
  fflush(stderr);

  delete R;
}

// **********************************************************************
//
// APPLY SWAP BIT TO MTBDD
// *To toggle this functionality, change #define WITH_SWAPS in
// bdd.h, fixed_dd.h, and build_bdd.cc*
//
// **********************************************************************

#ifdef WITH_SWAPS
inline void apply_swap(bdd_forest& bdd, bdd_edge* roots, unsigned nroots) {
    long start_time = get_time_in_us();

    fprintf(stderr, "Applying swap bit...\n");

    bdd.apply_swap(roots, nroots);
    unsigned count = bdd.mark_and_sweep(roots, nroots);

    show_elapsed("Swap bit application", get_time_in_us() - start_time);

    report(count, "nodes in swap-BDD\n");
    report(count * sizeof(bdd_node), "bytes for swap-BDD\n");
    fflush(stderr);
}
#endif


// **********************************************************************
//
// Verify MT BDD.  Checks BDD against minterms.
//
// **********************************************************************

static bool verify_mt(const bdd_forest &bdd, bdd_edge root, const char* vfile)
{
  if (0==vfile) return true;

  parser* P = new_parser(vfile);
  if (0==P) throw "Couldn't open input file.";

  fprintf(stderr, "Verifying against %s...\n", vfile);
  fflush(stderr);
  long start_time = get_time_in_us();

  unsigned inbits, outbits, nmt;
  P->read_header(inbits, outbits, nmt);

  if (inbits != bdd.get_num_vars()) throw "verify fail: different number of variables";
  char* minterm = new char[2+inbits];
  minterm[inbits+1] = 0;

  unsigned long checked = 0;
  unsigned long zeroed = 0;
  char* linebuf = new char[5+bdd.get_num_vars()];

  for (;;) {
    char value;
    if (!P->read_minterm(linebuf, value)) break;
    if ('0' == value) {
      zeroed++;
      continue;
    }

    bdd.varray2minterm(linebuf, minterm);
    char term = bdd.evaluate(root, minterm);
    if (value != term) {
      cout << "Verification failure on minterm #" << checked << "\n";
      cout << "\tInput line: " << linebuf << " (value " << value << ")\n";
      cout << "\tMinterm   : " << minterm+1 << "\n";
      cout << "\tTerminal  : " << term << "\n";
      //delete[] minterm;
      //delete[] linebuf;
      //delete P;
      //return false;
    }

    checked++;
  } // for loop
  show_elapsed("Verification", get_time_in_us() - start_time);
  report(checked, "minterms checked\n"); 
  report(zeroed , "minterms skipped (zero terminal)\n"); 
  report(nmt    , "minterms expected\n"); 
  delete[] minterm;
  delete[] linebuf;
  delete P;
  return true; 
}

// **********************************************************************
//
// Build bitwise BDD.
//
// **********************************************************************

static void build_bw(parser* P, const switches &S, bdd_forest &bdd, bdd_edge roots[], unsigned nroots)
{
  char* linebuf = new char[5+bdd.get_num_vars()];

	bool dontcare_map = (DCBIN == S.encoding);
	// No resolution during construction.

  //
  // Build buffers 
  //
  fprintf(stderr, "Using buffers of size %u\n", S.bufsize);
	minterm_buffer **B;
	B = new minterm_buffer*[nroots];
	for (unsigned i=0; i<nroots; ++i) {
		B[i] = new minterm_buffer(bdd, roots[i], S.bufsize, 0);
	}

  //
  // Build BDDs from minterms
  //
  fprintf(stderr, "Building BDDs...\n");
  fflush(stderr);

  long start_time = get_time_in_us();

  unsigned num_gcs = 0;
  unsigned unions = 0;
	unsigned gtimer = S.gtimer;
  for (;;) {
    char term;
    if (! P->read_minterm(linebuf, term)) break;
    if ('0' == term) continue;
		const unsigned traw = unsigned(term - '0');

		if (dontcare_map) {
				unions += B[0]->add(linebuf, '1');
		}
		unsigned mask = 0x01;
		for (unsigned i=1; i<nroots; ++i) {
			if (mask & traw)	{
				unions += B[i]->add(linebuf, '1');
			} else {
				if (!dontcare_map) {
					unions += B[i]->add(linebuf, '2');
				}
			}
			mask *= 2;
		}

		if (--gtimer) continue;
		fprintf(stderr, "  gc at %lu minterms\n", 
				B[1-dontcare_map]->get_num_minterms()
		);
		gtimer = S.gtimer;
    ++num_gcs;
    bdd.mark_and_sweep(roots, nroots);
  } // for loop

  //
  // Add any remaining buffers
  //
	for (unsigned i=0; i<nroots; ++i) {
		B[i]->flush();
	}
  ++unions;
  ++num_gcs;
  unsigned count = bdd.mark_and_sweep(roots, nroots);

  show_elapsed("BDD construction", get_time_in_us() - start_time);

	report(B[0]->get_num_minterms(), "care/don't care minterms\n");
	for (unsigned i=1; i<nroots; ++i) {
		report(B[i]->get_num_minterms(), "bit ");
		fprintf(stderr, "%u minterms\n", i);
	}
  report(bdd.peak_nodes(), "peak BDD nodes (forest)\n");
  report(bdd.peak_nodes() * sizeof(bdd_node), "peak BDD bytes (forest)\n");
  report(count, "final BDD nodes (forest)\n");
  report(count * sizeof(bdd_node), "final BDD bytes (forest)\n");
  report(unions, "unions\n");
  report(num_gcs, "garbage collections\n");
  fflush(stderr);

  delete[] linebuf;
	for (unsigned i=0; i<nroots; ++i) delete B[i];
	delete[] B;
}


// **********************************************************************
//
// Set up options
//
// **********************************************************************


static void init_switches(optman &OM, switches &S)
{
    //
    // Set up algorithm names and documentation
    ALGOS[NONE].set("none", "Leave any undefined values.");
    ALGOS[RESTR].set("restr", "'restrict'.  Nodes with an undefined\nchild become redundant.");
    ALGOS[OSM].set("osm", "Top-down, check for one-sided matches.");
    ALGOS[TSM].set("tsm", "Top-down, check for two-sided matches.");

		//
		// Set up encodings
		ENCODINGS[MTERM].set("mt", "Multi-terminal BDD (function range plus don't care).");
		ENCODINGS[BTERM].set("bt", "One BDD for each bit, outputs are 0 for don't care,\n1 for bit set, 2 for bit not set.");
		ENCODINGS[DCBIN].set("dcb", "One BDD for care/don't care, one BDD per output bit;\noutputs purely binary.");

    
    // -m
    OM.add_option('m', new letter_option(
      S.show_minterms, "Display minterms when done."
    ));
    S.show_minterms = 0;

    //USE SWAP FLAG IF ALLOWED
#ifdef WITH_SWAPS
    // -s
    OM.add_option('s', new letter_option(
        S.use_swap_bit, "Use swap bit on edges. \n"
        "Canonical by setting low edge's edge value to 0. If both have edge value\n"
        "0, then low edge points to lower node in UT. If both edges to same node, \n"
        "low edge has swap=false. Implimented after concretization."
    ));
    S.use_swap_bit = 0;
#endif

    // -p
    OM.add_option('p', new letter_option(
      S.show_progress, "Show progress."
    ));
    S.show_progress = 0;

    // -i
    OM.add_option('i', new option(
      "Read input from file.  If missing, read from\n"
      "standard input.  If extension .xz is present,\n"
      "input file is piped through xzcat.", 
      1,
      new string_arg(" file", S.input_file, 0)
    ));
    S.input_file = 0;

    // -v
    OM.add_option('v', new option(
      "Verify resulting BDD(s) against file, which\n"
      "has exactly the same format as the input.", 
      1,
      new string_arg(" file", S.verify_file, 0)
    ));
    S.verify_file = 0;

    // -o
    OM.add_option('o', new option(
      "Specifiy variable order, as a comma-separated list of\n"
      "levels.  If omitted, defaults to 1,2,3,...,L\n"
      "Note that level 1 is the bottom, level L is the top.", 
      1,
      new string_arg(" order", S.reorder, 0)
    ));
    S.reorder = 0;
    
    // -O
    OM.add_option('O', new option(
      "Read the variable order from a file.  The file format\n"
      "is similar to -o except whitespace is allowed.", 
      1,
      new string_arg(" file", S.reord_file, 0)
    ));
    S.reord_file = 0;


    // -b
    OM.add_option('b', new option(
      "Number of cubes to buffer, before adding to BDD.", 
      1,
      new unsigned_arg(" bufsize", S.bufsize, 0)
    ));
    S.bufsize = 1024;

    // -c
    OM.add_option('c', new option(
      "Size of compute table.",
      1,
      new unsigned_arg(" ctsize", S.ctsize, 0)
    ));
    S.ctsize = 1024*1024;

    // -g
    OM.add_option('g', new option(
      "Evenly spread out this many GC runs during BDD construction.",
      1,
      new unsigned_arg(" gcount", S.gcount, 0)
    ));
    S.gcount = 5;

    // -d
    OM.add_option('d', new option(
      "Attempt to resolve undefined values, during construction.", 
        2,
      new enum_arg("alg", S.DALG, "specifies the algorithm to use, run in batches.", NUMALGS, 
        ALGOS[0].link(), ALGOS[1].link(), ALGOS[2].link(), ALGOS[3].link()
      ),
      new unsigned_arg("k", S.Dlevel, 
        "specifies the level that triggers the resolver:\n"
        "when the minterm changes above level k, we resolve\n"
        "undefined values from level k downward."
      )
    ));
    S.DALG = NONE;
    S.Dlevel = 16;

		// -e
		OM.add_option('e', new option(
					"How to encode the output.",
					1,
					new enum_arg("enc", S.encoding, "specifies the encoder to use.", NUMENCS,
						ENCODINGS[0].link(), ENCODINGS[1].link(), ENCODINGS[2].link()
					)
		));
		S.encoding = MTERM;

    // -u
    OM.add_option('u', new option(
      "How to resolve undefined values, after construction.", 2,
      new enum_arg("alg", S.UALG, "specifies the algorithm to use, run in batches.", NUMALGS, 
        ALGOS[0].link(), ALGOS[1].link(), ALGOS[2].link(), ALGOS[3].link()
      ),
      new signed_arg("B", S.Ubatch, 
        "specifies the batch size.\n"
        "Positive B: run bottom up, B levels at a time\n"
        "Negative B: run top down, -B levels at a time\n"
        "Zero B: run for all levels at once"
      )
    ));
    S.UALG = NONE;
    S.Ubatch = 0;

    // -w
    OM.add_option('w', new option(
      "Write the final BDD to the specified file,\n"
      "in an exchange format.",
      1,
      new string_arg(" file", S.write_file, 0)
    ));
    S.write_file = 0;

    // -P
    OM.add_option('P', new option(
        "encode using permutation,\n"
        "accepts sequence of digits: eg '04231'.",
        1,
        new string_arg(" permutation", S.permutation, 0)
    ));
    S.permutation = 0;
}

// **********************************************************************
//
// Main program
//
// **********************************************************************

int main(int argc, const char** argv)
{
  try {
    //
    // Set up command line options
    //
    optman OM('h', "(options)");
    switches S;
		init_switches(OM, S);

    //
    // Process command line switches
    //
    int argproc = OM.parse_switches(argc, argv);
    if (argproc < argc) {
      cout << "Unknown switch: " << argv[argproc] << "\n\n";
      cout << "Run with -h to see valid command line options.\n";
      return 1;
    }

    //
    // Open input file
    //
    parser* P;
    if (S.input_file) {
      P = new_parser(S.input_file);
      cerr << "Input from " << S.input_file << "\n";
    } else {
      P = new_parser('p', ' '); // TBD option for this
      cerr << "Input from stdin\n";
    }
    if (0==P) throw "Couldn't open input file.";

    //
    // Read header portion of input
    //
    unsigned inbits, outbits, nmt;
    P->read_header(inbits, outbits, nmt);
    if (0==inbits) throw "Couldn't determine input bits.";
    fprintf(stderr, "Header information:\n");
    report(inbits, "Input bits\n");
    report(outbits, "Output bits\n");
    report(nmt, "Expected minterms\n");

		unsigned range = 0;
		switch (S.encoding) {
			case MTERM:
					range = 2;
					fprintf(stderr, "Multi-terminal encoding; range is 0..%u\n", range-1);
					break;

			case BTERM:
					range = 3;
					fprintf(stderr, "Bitwise encoding; range of each is 0..%u\n", range-1);
					break;

			case DCBIN:
					range = 2;
					fprintf(stderr, "Using %u BDDs for care/don't care plus bits\n",
							1+outbits);
					break;

			default:
					throw "Unknown encoding scheme.";
		}

    //
    // Build Variable ordering mapping
    // map[i] tells the BDD level for input variable i
    //
    unsigned* map = 0;
    if (S.reorder) {
      stringstream ss(S.reorder);
      map = build_reorder(inbits, ss);
    } else if (S.reord_file) {
      ifstream F(S.reord_file);
      if (!F) throw "Couldn't open order file";
      map = build_reorder(inbits, F);
    } else {
      map = new unsigned[1+inbits];
      for (unsigned b=0; b<=inbits; b++) map[b] = b;
    }

    //
    // Check ordering mapping
    //
    check_reorder(inbits, map);

    //
    // Build BDD forest and CT
    //
    compute_table CT(S.ctsize);
    bdd_forest BDD(static_cast <unsigned short>(inbits), map, static_cast <unsigned char>(range));
    BDD.set_CT(&CT);

    bdd_edge* roots = new bdd_edge[1 + outbits]; {
        for (unsigned i = 0; i <= outbits; ++i) {
            roots[i].node = 0;
            roots[i].edge = 0;
#ifdef WITH_SWAPS
            roots[i].swap = false;
#endif
        }
    }

		S.gtimer = nmt / (S.gcount+1);
		S.gtimer += 2;
		if (MTERM == S.encoding) {
			build_mt(P, S, BDD, CT, roots[0]);
		} else {
			build_bw(P, S, BDD, roots, 1+outbits);
		}
    delete P;

    //
    // Concretize BDD
    //
		if (DCBIN != S.encoding) {
			concretize(S, BDD, CT, roots, 1+outbits);
		} else {
			// nothing yet
		}

    //
    //Encode with swap bit
    //
#ifdef WITH_SWAPS
        if (S.use_swap_bit) {
            if (MTERM == S.encoding) {
                apply_swap(BDD, roots, outbits + 1);
            }
        }
#endif

    //
    // Verify BDD (if switch is set)
    //
		if (MTERM == S.encoding) {
			if (!verify_mt(BDD, roots[0], S.verify_file)) {
				return 1;
			}
		}

    if (S.show_minterms) {
			if (MTERM == S.encoding) {
				cout << "Final minterms:\n";
				BDD.show_minterms(cout, roots[0].node);
			}
    }

    if (S.write_file) {
      ofstream outf(S.write_file);
      if (!outf) {
        fprintf(stderr, "Couldn't write to file %s\n", S.write_file);
      } else {
				if (MTERM == S.encoding) {
					BDD.write_exch(outf, roots[0]);
				} else {
					BDD.write_exch(outf, roots, 1+outbits);
				}
      }
    }

  }
  catch (const char* err) {
    fprintf(stderr, "Error: %s\n", err);
    return 1;
  }
  return 0;
}
