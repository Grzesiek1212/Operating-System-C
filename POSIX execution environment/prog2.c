#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define MAX_LENGTH 20

#define ERR(source)                                                \
  (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
   exit(EXIT_FAILURE))

int main(int argc, char** argv) {
  char p[MAX_LENGTH + 2];
  scanf("%" STR(MAX_LENGTH) "s", p);
  if (strlen(p) > MAX_LENGTH) ERR("za dlugie");
  printf("hello %s\n", p);
  return EXIT_SUCCESS;
}