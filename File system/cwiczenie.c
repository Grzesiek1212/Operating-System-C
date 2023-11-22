#define _XOPEN_SOURCE 500
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAXFD 20
#define MAX_PATH 200
#define ERR(source)                                                \
  (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
   exit(EXIT_FAILURE))

void usage(const char* const pname) {
  fprintf(stderr, "USAGE:%s path_1 path_2\n", pname);
  exit(EXIT_FAILURE);
}

void ScanDir(FILE* wyn) {
  DIR* dirp;
  struct dirent* plik;
  struct stat sb;

  dirp = opendir(".");
  if (dirp == NULL) ERR("opendir");
  fprintf(wyn, "\nLISTA PLIKÓW:\n");
  while ((plik = readdir(dirp)) != NULL) {
    errno = 0;
    if (strcmp(plik->d_name, ".") == 0 || strcmp(plik->d_name, "..") == 0) {
      continue;
    }
    if (lstat(plik->d_name, &sb)) ERR("lstat");
    fprintf(wyn, "%s ", plik->d_name);
    fprintf(wyn, "%ld\n", (long)sb.st_size);
  }
  if (errno != 0) ERR("readdir");
  if (closedir(dirp)) ERR("closedir");
}

int main(int argc, char** argv) {
  int c, i;
  char* folders[MAXFD];
  int number = 0;
  char path[MAX_PATH];
  FILE* wyn = stdout;
  while ((c = getopt(argc, argv, "p:o:")) != -1) switch (c) {
      case 'p':
        folders[number++] = optarg;
        break;
      case 'o':
        if (wyn == stdout) {
          if ((wyn = fopen(optarg, "w+")) == NULL) ERR("fopen");
        } else {
          usage(argv[0]);
        }
        break;
      default:
        usage(argv[0]);
    }

  if (getcwd(path, MAX_PATH) == NULL) ERR("getcwd");
  for (i = 0; i < number; i++) {
    if (chdir(folders[i])) ERR("chdir");
    fprintf(wyn, "SCIEZKA:\n %s", folders[i]);
    ScanDir(wyn);
    if (chdir(path)) ERR("chdir");
  }
  return EXIT_SUCCESS;
}