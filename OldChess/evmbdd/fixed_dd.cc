
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
    edge_child0 = nullptr;
    edge_child1 = nullptr;
#ifdef WITH_SWAPS
    swap_child0 = nullptr;
    swap_child1 = nullptr;
#endif
    ut = nullptr;
    ut_next = nullptr;
}

fixed_dd::~fixed_dd()
{
    std::free(node_level);
    std::free(node_child0);
    std::free(node_child1);
    std::free(edge_child0);
    std::free(edge_child1);

#ifdef WITH_SWAPS
    std::free(swap_child0);
    std::free(swap_child1);
#endif


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

unsigned* fixed_dd::add_nodes_from_file(const char* pathname)
{
    if (nullptr != node_level && nullptr == ut) {
        BDD_READER_TERMINATE("Can't add nodes after calling done_building()");
    }

    //
    // Determine extension, and open file/pipeline for reading
    //
    const char* ext = nullptr;
    for (const char* ptr = pathname; *ptr; ptr++) {
        if ('.' == *ptr) ext = ptr + 1;
    }
    const char* zcat = nullptr;
    if (ext) {
        if (0 == strcmp("xz", ext))   zcat = "xzcat";
        if (0 == strcmp("gz", ext))   zcat = "zcat";
        if (0 == strcmp("bz2", ext))  zcat = "bzcat";
    }
    FILE* fin = nullptr;
    if (zcat) {
        char cmd[1024];
        std::snprintf(cmd, 1024, "%s %s", zcat, pathname);
        fin = popen(cmd, "r");
        if (nullptr == fin) {
            BDD_READER_TERMINATE("Couldn't popen file");
        }
    }
    else {
        fin = fopen(pathname, "r");
        if (nullptr == fin) {
            BDD_READER_TERMINATE("Couldn't fopen file");
        }
    }

    //
    // Parse input 
    //
    char buffer[6];
    fread(buffer, 1, 6, fin);
    if (('E' != buffer[0]) or ('B' != buffer[2]) or ('D' != buffer[3]) or ('D' != buffer[4]) or ('\n' != buffer[5])) {
        BDD_READER_TERMINATE("Not a BDD file");
    }

    //
    // Input order L, T, N, R
    //
    expect_char(fin, 'L');
    unsigned file_levels = expect_unsigned(fin);
#ifdef DEBUG_PARSER
    std::printf("Got L %u\n", file_levels);
#endif
    if (0 == levels) {
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
    bool* file2swaps = new bool[file_nodes + file_terms];
    unsigned* file2edge = new unsigned[file_nodes + file_terms];
    for (unsigned u = 0; u < file_terms + file_nodes; u++) {
        file2swaps[u] = false;
        file2edge[u] = 0;
    }
#endif


    expect_char(fin, 'M');
    unsigned modVal = expect_unsigned(fin);
#ifdef DEBUG_PARSER
    std::printf("Got R %u\n", file_roots);
#endif
    if (modVal < 1) {
        BDD_READER_TERMINATE("Invalid Mod Value");
    }

    //
    // For each level, read nodes
    //
    unsigned file_index = file_terms;
    unsigned k;
    for (k = 1; k <= file_levels; k++) {
        expect_char(fin, 'n');
        unsigned nk = expect_unsigned(fin);
#ifdef DEBUG_PARSER
        std::printf("Reading %u nodes at level %u\n", nk, k);
#endif
        for (; nk; --nk) {
            unsigned child = expect_unsigned(fin);
            unsigned low = file2handle[child];
            char low_edge = (expect_unsigned(fin) + file2edge[child]) % 5;
#ifdef WITH_SWAPS
            bool low_swap = expect_unsigned(fin) ^ file2swaps[child];
#else
            bool low_swap = false;
#endif

            child = expect_unsigned(fin);
            unsigned high = file2handle[child];
            char high_edge = (expect_unsigned(fin) + file2edge[child]) % 5;
#ifdef WITH_SWAPS
            bool high_swap = expect_unsigned(fin) ^ file2swaps[child];
#else
            bool high_swap = false;
#endif
            assert(low < 0xffffffff);
            assert(low_edge < 5 && low_edge >= 0);
            assert(low_swap == false | low_swap == true);
            assert(high < 0xffffffff);
            assert(high_edge < 5 && high_edge >= 0);
            assert(high_swap == false | high_swap == true);

#ifdef WITH_SWAPS
            if (file_swap) {
                if ((low > high) || ((low == high) && (low_swap > high_swap)) || ((low == high) && (low_swap == high_swap) && (high_edge-low_edge > modVal/2))) {
                    file2swaps[file_index] = true;

                    unsigned tempNode = low;
                    low = high;
                    high = tempNode;

                    unsigned tempEdge = low_edge;
                    low_edge = high_edge;
                    high_edge = tempEdge;

                    bool tempSwap = low_swap;
                    low_swap = high_swap;
                    high_swap = tempSwap;
                }
            }
#endif
            if (low == 0) {
                file2edge[file_index] = high_edge;
                high_edge = 0;
                low_edge = 0;
            }
            else if (high == 0) {
                file2edge[file_index] = high_edge;
                high_edge = 0;
                low_edge = 0;
            }
            else {
                file2edge[file_index] = low_edge;
                high_edge = (modVal + high_edge - low_edge) % modVal;
                low_edge = 0;
            }

            unsigned handle = add_unique(static_cast <unsigned char>(k), low, high, low_edge, high_edge, low_swap, high_swap);

            file2handle[file_index] = handle;
            ++file_index;
        } // for nk
    } // for k

    //
    // Read roots
    //
#ifdef WITH_SWAPS
    unsigned* root = new unsigned[3];
#else
    unsigned* root = new unsigned[2];
#endif
    expect_char(fin, 'r');
    root[0] = file2handle[expect_unsigned(fin)];
    expect_char(fin, 'v');
    root[1] = expect_unsigned(fin);
#ifdef WITH_SWAPS
    expect_char(fin, 's');
    root[2] = expect_unsigned(fin);
#endif

    //
    // Cleanup
    //
    if (ext) {
        pclose(fin);
    }
    else {
        fclose(fin);
    }
    delete[] file2handle;
    return root;
}

void fixed_dd::done_building()
{
    std::free(ut);
    std::free(ut_next);

    ut = nullptr;
    ut_next = nullptr;
}

unsigned* fixed_dd::evaluate(unsigned f_root, const bool* vars, unsigned root_val, bool root_swap) const
{
    bool dir;
    static unsigned val[2];
    val[0] = 0;
    val[1] = 0;

    if (f_root >= next_node) {
        BDD_READER_TERMINATE("Bad f_root in call to evaluate");
    }
    for (;;) {
        if (0 == node_level[f_root]) {
            val[1] = (f_root == 0);
            val[0] = (f_root == 0) ? 0 : (val[0] + root_val) % 5;
            return val;
        }

        dir = (vars[node_level[f_root]]) ^ (root_swap);

        if (dir) { //Move along high-edge
#ifdef WITH_SWAPS
            root_swap = swap_child1[f_root];
#endif
            val[0] += edge_child1[f_root];
            f_root = node_child1[f_root];
        }

        else { //move along low-edge
#ifdef WITH_SWAPS
            root_swap = swap_child0[f_root];
#endif
            val[0] += edge_child0[f_root];
            f_root = node_child0[f_root];
        }

    }
}

inline void hashbyte(unsigned& H, unsigned x)
{
    H = 37 * H ^ x;
}

inline void hashword(unsigned& H, unsigned x)
{
    hashbyte(H, x >> 24);
    hashbyte(H, (x & 0x0ff0000) >> 16);
    hashbyte(H, (x & 0x000ff00) >> 8);
    hashbyte(H, x & 0x00000ff);
}

inline unsigned hashnode(unsigned char level, unsigned child0, unsigned child1, unsigned edge0, unsigned edge1, bool swap0, bool swap1)
{
    unsigned H = 0x480bdd;
    hashbyte(H, level);
    hashword(H, child0);
    hashword(H, child1);
    hashword(H, edge0);
    hashword(H, edge1);
    hashword(H, swap0);
    hashword(H, swap1);
    return H;
}

unsigned fixed_dd::add_unique(unsigned char level, unsigned child0, unsigned child1, char edge0, char edge1, bool swap0, bool swap1)
{
    CHECK(level);
    if (next_node >= max_nodes)
    {
        enlarge_ut(next_node + 1);
    }

    // Hash node
    unsigned H = hashnode(level, child0, child1, edge0, edge1, swap0, swap1) % max_nodes;

    // Check all nodes in this bucket for equality
    for (unsigned curr = ut[H]; curr; curr = ut_next[curr]) {
        if (node_level[curr] != level)    continue;
        if (node_child0[curr] != child0)  continue;
        if (node_child1[curr] != child1)  continue;
        if (edge_child0[curr] != edge0)  continue;
        if (edge_child1[curr] != edge1)  continue;
#ifdef WITH_SWAPS
        if (swap_child0[curr] != swap0)  continue;
        if (swap_child1[curr] != swap1)  continue;
#endif
        return curr;
    }

    // This is a new node; create it and add to UT
    node_level[next_node] = level;
    node_child0[next_node] = child0;
    node_child1[next_node] = child1;
    edge_child0[next_node] = edge0;
    edge_child1[next_node] = edge1;
#ifdef WITH_SWAPS
    swap_child0[next_node] = swap0;
    swap_child1[next_node] = swap1;
#endif
    ut_next[next_node] = ut[H];
    ut[H] = next_node;
    return next_node++;
}

void fixed_dd::enlarge_ut(unsigned req_max)
{
    if (0 == req_max) {
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
    if (nullptr == node_level) {
        BDD_READER_TERMINATE("realloc fail: node level");
    }
    for (u = max_nodes; u < rm; u++) node_level[u] = 0;

    node_child0 = static_cast <unsigned*> (std::realloc(node_child0, rm * sizeof(unsigned)));
    if (nullptr == node_child0) {
        BDD_READER_TERMINATE("realloc fail: node_child0");
    }

    node_child1 = static_cast <unsigned*> (std::realloc(node_child1, rm * sizeof(unsigned)));
    if (nullptr == node_child1) {
        BDD_READER_TERMINATE("realloc fail: node_child1");
    }

    edge_child0 = static_cast <char*> (std::realloc(edge_child0, rm * sizeof(unsigned)));
    if (nullptr == edge_child0) {
        BDD_READER_TERMINATE("realloc fail: edge_child0");
    }

    edge_child1 = static_cast <char*> (std::realloc(edge_child1, rm * sizeof(unsigned)));
    if (nullptr == edge_child1) {
        BDD_READER_TERMINATE("realloc fail: edge_child1");
    }

#ifdef WITH_SWAPS
    swap_child0 = static_cast <bool*> (std::realloc(swap_child0, rm * sizeof(unsigned)));
    if (nullptr == swap_child0) {
        BDD_READER_TERMINATE("realloc fail: swap_child0");
    }

    swap_child1 = static_cast <bool*> (std::realloc(swap_child1, rm * sizeof(unsigned)));
    if (nullptr == swap_child1) {
        BDD_READER_TERMINATE("realloc fail: swap_child1");
    }
#endif

    ut = static_cast <unsigned*> (std::realloc(ut, rm * sizeof(unsigned)));
    if (nullptr == ut) {
        BDD_READER_TERMINATE("realloc fail: ut");
    }

    ut_next = static_cast <unsigned*> (std::realloc(ut_next, rm * sizeof(unsigned)));
    if (nullptr == ut_next) {
        BDD_READER_TERMINATE("realloc fail: ut_next");
    }


    //
    // Re-hash everything into UT
    //
    for (u = 0; u < rm; u++) {
        ut[u] = 0;
        ut_next[u] = 0;
    }
    for (u = 0; u < terminals; u++) {
        node_level[u] = 0;
    }
    for (u = terminals; u < next_node; u++) {
        if (0 == node_level[u]) continue;
#ifdef WITH_SWAPS
        unsigned H = hashnode(node_level[u], node_child0[u], node_child1[u], edge_child0[u], edge_child1[u], swap_child0[u], swap_child1[u]) % rm;
#else
        unsigned H = hashnode(node_level[u], node_child0[u], node_child1[u], edge_child0[u], edge_child1[u], false, false) % rm;
#endif

        ut_next[u] = ut[H];
        ut[H] = u;
    }

    max_nodes = rm;
}

