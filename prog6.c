#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage(char* name) {
  fprintf(stderr, "Usage:%s ([-t,x] -n Name\n)", name);
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  int opt, i;
  int x = 1;
  while ((opt = getopt(argc, argv, "t:n:")) != -1) {
    switch (opt) {
      case 't':
        x = atoi(optarg);
        break;
      case 'n':
        for (i = 0; i < x; i++) printf("Hello %s\n", optarg);
        break;
      case '?':
      default:
        usage("zle myslisz");
    }
  }
  if (argc > optind) usage(argv[0]);
  return EXIT_SUCCESS;
}