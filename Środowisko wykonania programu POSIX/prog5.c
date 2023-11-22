#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(char* name) {
  fprintf(stderr, "%s - bład\n", name);
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  if (argc != 3) {
    usage("nieprawdiłowa ilość argumentow");
  }

  char* endptr;
  int i, j = strtol(argv[2], &endptr, 10);
  if (argv[2] == endptr) usage("zle podane argumenty");
  for (i = 0; i < j; i++) {
    printf("%s\n", argv[1]);
  }
  return EXIT_SUCCESS;
}