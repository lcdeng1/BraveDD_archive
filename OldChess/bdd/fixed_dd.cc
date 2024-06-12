
#include "fixed_dd.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>   // malloc, realloc, and free :)
#include <cassert>
#include <limits>

#include "exception_neutral_macros.hpp"

// #define DEBUG_PARSER

inline void CHECK(bool b)
{
  assert(b);
}

fixed_dd::fixed_dd()
{
  levels = 0;
  terminals = 0;
  next_node = 0;

  file_nodes = 0;

  max_nodes = 0;
  node_level = nullptr;
  node_child0 = nullptr;
  node_child1 = nullptr;
  node_shift0 = nullptr;
  node_shift1 = nullptr;

  ut = nullptr;
  ut_next = nullptr;
}

fixed_dd::~fixed_dd()
{
	std::free(node_level);
	std::free(node_child0);
	std::free(node_child1);
    std::free(node_shift0);
    std::free(node_shift1);

	std::free(ut);
	std::free(ut_next);
}

static char skip_whitespace(FILE* fin)
{
  char comment = 0;
  for (;;) {
    int c = fgetc(fin);
    if (EOF == c) {
			BDD_READER_TERMINATE("Unexpected eof");
		}
    if ('\n' == c) {
      comment = 0;
      continue;
    }
    if (';' == c) {
      comment = 1;
      continue;
    }
    if (comment) continue;
    if (' ' == c) continue;
    if ('\t' == c) continue;
    if ('\r' == c) continue;

    return static_cast <char>(c);
  }
}

inline void expect_char(FILE* fin, char next)
{
  if (skip_whitespace(fin) != next) {
		BDD_READER_TERMINATE("Syntax error");
	}
}

inline unsigned expect_unsigned(FILE* fin)
{
  char c = skip_whitespace(fin);
  if ((c > '9') || (c < '0')) {
		BDD_READER_TERMINATE("Integer expected");
	}
	std::ungetc(c, fin);
  unsigned u;
	std::fscanf(fin, "%u", &u);
  return u;
}

void fixed_dd::add_nodes_from_file(const char* pathname, unsigned &root, bool &swap, short &shift)
{
  if (nullptr!=node_level && nullptr==ut) {
		BDD_READER_TERMINATE("Can't add nodes after calling done_building()");
  }

  //
  // Determine extension, and open file/pipeline for reading
  //
  const char* ext = nullptr;
  for (const char* ptr = pathname; *ptr; ptr++) {
    if ('.' == *ptr) ext = ptr+1;
  }
  const char* zcat = nullptr;
  if (ext) {
    if (0==strcmp("xz", ext))   zcat = "xzcat";
    if (0==strcmp("gz", ext))   zcat = "zcat";
    if (0==strcmp("bz2", ext))  zcat = "bzcat";
  }
  FILE* fin = nullptr;
  if (zcat) {
    char cmd[1024];
		std::snprintf(cmd, 1024, "%s %s", zcat, pathname);
    fin = popen(cmd, "r");
    if (nullptr==fin) {
			BDD_READER_TERMINATE("Couldn't popen file");
		}
  } else {
    fin = fopen(pathname, "r");
    if (nullptr==fin) {
			BDD_READER_TERMINATE("Couldn't fopen file");
		}
  }
  
    //
    // Parse input 
    //
    char buffer[4];
    fread(buffer, 1, 4, fin);
    if ( ('B' != buffer[0]) or ('D' != buffer[1]) or ('D' != buffer[2]) or ('\n' != buffer[3]) ) {
			BDD_READER_TERMINATE("Not a BDD file");
		}

    //
    // Input order L, T, N, R, S, V
    //
    expect_char(fin, 'L');
    unsigned file_levels = expect_unsigned(fin);
#ifdef DEBUG_PARSER
		std::printf("Got L %u\n", file_levels);
#endif
    if (0==levels) {
      assert(file_levels <= std::numeric_limits<decltype(levels)>::max());
      levels = static_cast <decltype(levels)> (file_levels);
    }
    if (levels != file_levels) {
        BDD_READER_TERMINATE("Incompatible number of BDD levels");
    }

    expect_char(fin, 'T');
    unsigned file_terms = expect_unsigned(fin);
#ifdef DEBUG_PARSER
    std::printf("Got T %u\n", file_terms);
#endif
    if (0 == terminals) {
        terminals = file_terms;
        next_node = file_terms;
    }
    if (file_terms > terminals) {
        BDD_READER_TERMINATE("Incompatible number of BDD terminals");
    }

    expect_char(fin, 'N');
    file_nodes = expect_unsigned(fin);
#ifdef DEBUG_PARSER
    std::printf("Got N %u\n", file_nodes);
#endif
    enlarge_ut(file_nodes + next_node);
    unsigned* file2handle = new unsigned[file_nodes + file_terms];
    for (unsigned u = 0; u < file_terms; u++) {
        file2handle[u] = u;
    }
    for (unsigned u = 0; u < file_nodes; u++) {
        file2handle[u + file_terms] = 0xffffffff;
    }

    // TBD - build mapping from file node index to forest index

    expect_char(fin, 'R');
    unsigned file_roots = expect_unsigned(fin);
#ifdef DEBUG_PARSER
    std::printf("Got R %u\n", file_roots);
#endif
    if (file_roots > 1) {
        BDD_READER_TERMINATE("Too many roots in file");
    }

#ifdef WITH_SWAPS
    expect_char(fin, 'S');
    bool file_swap = expect_unsigned(fin);

    bool* file2swap = new bool[file_nodes + file_terms];
    for (unsigned u = 0; u < file_terms + file_nodes; u++) {
        file2swap[u] = false;
    }
#endif

#ifdef WITH_SHIFTS
    expect_char(fin, 'V');
    bool file_shift = expect_unsigned(fin);
#endif
//
// For each level, read nodes
//
unsigned file_index = file_terms;
unsigned k;
for (k = 1; k <= file_levels; k++) {
    unsigned nk;
#ifndef WITH_SHIFTS
    expect_char(fin, 'n');
    nk = expect_unsigned(fin);
#else
    if (!file_shift) {
        expect_char(fin, 'n');
        nk = expect_unsigned(fin);
    }
    else {
        if (k != file_levels) continue;
        nk = file_nodes;
    }
#endif

#ifdef DEBUG_PARSER
			std::printf("Reading %u nodes at level %u\n", nk, k);
#endif
      for (; nk; --nk) {
        unsigned node = expect_unsigned(fin);
        unsigned low  = file2handle[node];
#ifdef WITH_SWAPS
        bool lswap = expect_unsigned(fin) ^ file2swap[node];
#else
        bool lswap = false;
#endif
#ifdef WITH_SHIFTS
        bool lshift = expect_unsigned(fin);
#else
        bool lshift = false;
#endif

        node = expect_unsigned(fin);
        unsigned high = file2handle[node];
#ifdef WITH_SWAPS
        bool hswap = expect_unsigned(fin) ^ file2swap[node];
#else
        bool hswap = false;
#endif
#ifdef WITH_SHIFTS
        bool hshift = expect_unsigned(fin);
#else
        bool hshift = false;
#endif

        assert(low < 0xffffffff);
        assert(high < 0xffffffff);
        assert(lswap < 2);
        assert(hswap < 2);
        assert(lshift < 0xffffffff);
        assert(hshift < 0xffffffff);

#ifdef WITH_SWAPS
        if (file_swap) { //if swapping is active
            if ((low > high) || ((low == high) && (lswap > hswap))) { //if node should be swapped
                file2swap[file_index] = true;

                unsigned tmpNode = low;
                low = high;
                high = tmpNode;

                bool tmpSwap = lswap;
                lswap = hswap;
                hswap = tmpSwap;
            }
        }
#endif
        unsigned handle = add_unique(static_cast <unsigned char>(k), low, lswap, lshift, high, hswap, hshift);
        file2handle[file_index] = handle;
        ++file_index;
      } // for nk
    } // for k

    //
    // Read roots
    //
    expect_char(fin, 'r');
    root = file2handle[expect_unsigned(fin)];
#ifdef WITH_SWAPS
    expect_char(fin, 's');
    swap = expect_unsigned(fin);
#else
    swap = false;
#endif

#ifdef WITH_SHIFTS
    expect_char(fin, 'v');
    shift = expect_unsigned(fin);
#else
    shift = false;
#endif

    //
    // Cleanup
    //
    if (ext) {
      pclose(fin);
    } else {
      fclose(fin);
    }
    delete[] file2handle;
    //return root;
}

void fixed_dd::done_building()
{
	std::free(ut);
	std::free(ut_next);

  ut = nullptr;
  ut_next = nullptr;
}

unsigned power(unsigned x, unsigned n) {
    unsigned powr = 1;
    for (unsigned i = 0; i < n; i++) {
        powr = powr * x;
    }
    return powr;
}

unsigned fixed_dd::evaluate(unsigned f_root, bool f_swap, short f_shift, const bool* vars) const
{
  if (f_root >= next_node) {
    BDD_READER_TERMINATE("Bad f_root in call to evaluate");
  }
  unsigned tmp = f_root;
  for (;;) {
    if (0==node_level[f_root]) return f_root;

    bool dir = (vars[node_level[tmp]] ^ f_swap);
    f_root = (dir) ? node_child1[tmp] : node_child0[tmp];
    f_swap = false;
    if (f_root >= power(2, 31)) {
        f_root -= power(2, 31);
        f_swap = true;
    }

    tmp = f_root;
  }
}

inline void hashbyte(unsigned &H, unsigned x)
{
  H=37*H^x;
}

inline void hashword(unsigned &H, unsigned x)
{
  hashbyte(H,  x>>24);
  hashbyte(H, (x & 0x0ff0000) >> 16);
  hashbyte(H, (x & 0x000ff00) >> 8);
  hashbyte(H,  x & 0x00000ff);
}

inline unsigned hashnode(unsigned char level, unsigned child0, unsigned child1, short shift0, short shift1)
{
  unsigned H = 0x480bdd;
  hashbyte(H, level);
  hashword(H, child0);
  hashword(H, child1);
  hashword(H, shift0);
  hashword(H, shift1);
  return H;
}

unsigned fixed_dd::add_unique(unsigned char level, unsigned child0, bool swap0, short shift0, unsigned child1, bool swap1, short shift1)
{
  CHECK(level);
  if (next_node >= max_nodes)
  {
    enlarge_ut(next_node+1);
  }

  if (swap0 == true)
      child0 += power(2, 31);
  if (swap1 == true)
      child1 += power(2, 31);

  // Hash node
  unsigned H = hashnode(level, child0, child1, shift0, shift1) % max_nodes;

  // Check all nodes in this bucket for equality
  for (unsigned curr = ut[H]; curr; curr = ut_next[curr]) {
    if (node_level[curr] != level)    continue;
    if (node_child0[curr] != child0)  continue;
    if (node_child1[curr] != child1)  continue;
    if (node_shift0[curr] != shift0)  continue;
    if (node_shift1[curr] != shift1)  continue;
 
    return curr;
  }

  // This is a new node; create it and add to UT
  node_level[next_node] = level;
  node_child0[next_node] = child0;
  node_child1[next_node] = child1;
  node_shift0[next_node] = shift0;
  node_shift1[next_node] = shift1;
  ut_next[next_node] = ut[H];
  ut[H] = next_node;
  return next_node++;
}

void fixed_dd::enlarge_ut(unsigned req_max)
{
  if (0==req_max) {
		BDD_READER_TERMINATE("Unique table overflow");
	}
  if (req_max <= max_nodes) return;

  unsigned u, rm = 1023;
  while (rm < req_max) {
    rm *= 2;
    ++rm;
  }

  //
  // Enlarge node and UT storage
  //

  node_level = static_cast <unsigned char*> (std::realloc(node_level, rm));
  if (nullptr==node_level) {
		BDD_READER_TERMINATE("realloc fail: node level");
	}
  for (u=max_nodes; u<rm; u++) node_level[u] = 0; 

  node_child0 = static_cast <unsigned*> (std::realloc(node_child0, rm * sizeof(unsigned)));
  if (nullptr==node_child0) {
	 	BDD_READER_TERMINATE("realloc fail: node_child0");
	}

  node_child1 = static_cast <unsigned*> (std::realloc(node_child1, rm * sizeof(unsigned)));
  if (nullptr==node_child1) {
	 	BDD_READER_TERMINATE("realloc fail: node_child1");
	}

  node_shift0 = static_cast <short*> (std::realloc(node_shift0, rm * sizeof(short)));
  if (nullptr == node_shift0) {
      BDD_READER_TERMINATE("realloc fail: node_shift0");
  }

  node_shift1 = static_cast <short*> (std::realloc(node_shift1, rm * sizeof(short)));
  if (nullptr == node_shift1) {
      BDD_READER_TERMINATE("realloc fail: node_shift1");
  }

  ut = static_cast <unsigned*> (std::realloc(ut, rm * sizeof(unsigned)));
  if (nullptr==ut) {
	 	BDD_READER_TERMINATE("realloc fail: ut");
	}

  ut_next = static_cast <unsigned*> (std::realloc(ut_next, rm * sizeof(unsigned)));
  if (nullptr==ut_next) {
	 	BDD_READER_TERMINATE("realloc fail: ut_next");
	}

  
  //
  // Re-hash everything into UT
  //
  for (u=0; u<rm; u++) {
    ut[u] = 0;
    ut_next[u] = 0;
  }
  for (u=0; u<terminals; u++) {
    node_level[u] = 0;
  }
  for (u=terminals; u<next_node; u++) {
    if (0==node_level[u]) continue;
    unsigned H = hashnode(node_level[u], node_child0[u], node_child1[u], node_shift0[u], node_shift1[u]) % rm;

    ut_next[u] = ut[H];
    ut[H] = u;
  }

  max_nodes = rm;
}

