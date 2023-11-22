#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH 101

#define ERR(source)                                                \
  (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
   exit(EXIT_FAILURE))

void Scandir() {
  DIR *dirp;
  struct dirent *plik;
  struct stat sb;

  dirp = opendir(".");
  if (dirp == NULL) ERR("opendir");

  int files = 0, dirs = 0, links = 0, others = 0;
  while ((plik = readdir(dirp)) != NULL) {
    errno = 0;
    if (lstat(plik->d_name, &sb)) ERR("lstat");
    if (S_ISDIR(sb.st_mode)) {
      dirs++;
    } else if (S_ISLNK(sb.st_mode)) {
      links++;
    } else if (S_ISREG(sb.st_mode)) {
      files++;
    } else {
      others++;
    }
  }
  if (errno != 0) ERR("readdir");
  if (closedir(dirp)) ERR("closedir");
  printf("files: %d, dirs: %d, links: %d, others: %d\n", files, dirs, links,
         others);
}

int main(int argc, char **argv) {
  char path[MAX_PATH];
  if (getcwd(path, MAX_PATH) != NULL) {
    for (int i = 1; i < argc; i++) {
      if (chdir(argv[i])) continue;
      printf("%s:\n", argv[i]);
      Scandir();
      if (chdir(path)) ERR("chdir");
    }
  } else {
    ERR("getcwd");
  }
  return EXIT_SUCCESS;
}