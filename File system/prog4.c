#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define ERR(source)                                                 \
  (perror(source), fprintf(stderr, "%s,%d:\n", __FILE__, __LINE__), \
   exit(EXIT_FAILURE))

void usage(char *name) {
  fprintf(stderr, "gdzies sie jebnoles: %s", name);
  exit(EXIT_FAILURE);
}

void makefile(char *name, size_t size, mode_t perms, int procent) {
  int i;
  FILE *file;
  umask(~perms & 0777);
  file = fopen(name, "w+");
  if (file == NULL) ERR("fopen");

  fseek(file, size - 1, SEEK_SET);
  fprintf(file, "%c", 'X');
  for (i = 0; i < size * procent / 100; i++) {
    if (fseek(file, rand() % size, SEEK_SET)) ERR("fseek");
    fprintf(file, "%c", 'A' + (i % ('Z' - 'A' + 1)));
  }
  if (fclose(file)) ERR("fclose");
}

int main(int argc, char **argv) {
  int c;
  char *name = NULL;
  size_t size = -1;
  mode_t perms = -1;
  while ((c = getopt(argc, argv, "n:p:s:")) != -1) {
    switch (c) {
      case 'n':
        name = optarg;
        break;
      case 'p':
        perms = strtol(optarg, (char **)NULL, 8);
        break;
      case 's':
        size = strtol(optarg, (char **)NULL, 10);
        break;
      case '?':
      default:
        usage(argv[0]);
    }
  }
  if ((name == NULL) || ((mode_t)-1 == perms) || (size == -1)) usage(argv[0]);
  if (unlink(name) && errno != ENOENT) ERR("unlink");
  srand(time(NULL));
  makefile(name, size, perms, 10);
  return EXIT_SUCCESS;
}