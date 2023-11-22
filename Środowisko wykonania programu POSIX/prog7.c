#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

int main(int argc, char **argv) {
  int index = 0;
  while (environ[index]) {
    printf("%s\n", environ[index]);
    index++;
  }

  return EXIT_SUCCESS;
}