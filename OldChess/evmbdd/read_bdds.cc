
#include "fixed_dd.h"

#include <stdio.h>

using namespace std;

int main(int argc, const char** argv)
{
  if (argc < 2) {
    fprintf(stderr, "Usage: %s infile infile ...\n", argv[0]);
    return 1;
  }

  try {
    fixed_dd BDD;
    printf("%-30s  %12s  %12s\n", "File", "File nodes", "Forest nodes");
    printf("%-30s  %12s  %12s\n", "==============================", "============", "============");
    unsigned file_total = 0;
    unsigned prev_nodes = 0;
    for (int i=1; i<argc; i++) {
      BDD.add_nodes_from_file(argv[i]);
      printf("%-30s  %12u  %12u\n", 
        argv[i], 
        BDD.num_file_nodes(), 
        BDD.num_nonterminals() - prev_nodes
      );
      file_total += BDD.num_file_nodes();
      prev_nodes = BDD.num_nonterminals();
    }
    printf("%-30s  %12s  %12s\n", "==============================", "============", "============");
    printf("%-30s  %12u  %12u\n", 
      "Total", 
      file_total,
      BDD.num_nonterminals()
    );
  }
  catch (const char* error) {
    fprintf(stderr, "Error: %s\n", error);
    return 1;
  }

  return 0; 
}
