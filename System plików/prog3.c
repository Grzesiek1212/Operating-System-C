#define _XOPEN_SOURCE 500
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAXFD 20

#define ERR(source)                                               \
  (perror(source), fprintf(stederr, "%s:%d", __FILE__, __LINE__), \
   exit(EXIT_FAILURE))

int files = 0, dirs = 0, links = 0, others = 0;

int walk(const char *file, const struct stat *sb, int flag,
         struct FTW *ftwbuf) {
  switch (flag) {
    case FTW_DNR:
    case FTW_D:
      dirs++;
      // printf("%s\n",file);
      break;
    case FTW_F:
      files++;
      break;
    case FTW_SL:
      links++;
      break;
    default:
      others++;
  }
  return 0;
}

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (nftw(argv[i], walk, MAXFD, FTW_PHYS) == 0) {
      printf("%s:\n", argv[i]);
      printf("files: %d, dirs:%d, links:%d, others:%d\n", files, dirs, links,
             others);
    } else {
      printf("%s: brak dostępu\n", argv[i]);
      files = dirs = links = others = 0;
    }
  }
  return EXIT_SUCCESS;
}