#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  for (int i = 0; i < 15; ++i) {
    fprintf(stderr, "%d", i);
    sleep(1);
  }
  return EXIT_SUCCESS;
}