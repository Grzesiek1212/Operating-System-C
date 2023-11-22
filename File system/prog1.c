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

void ScanDir() {
  DIR* dirp;
  struct dirent* plik;
  struct stat sb;

  dirp = opendir(".");
  if (dirp == NULL) ERR("opendir");
  int pliki = 0, linki = 0, katalogi = 0, inne = 0;
  while ((plik = readdir(dirp)) != NULL) {
    errno = 0;
    if (lstat(plik->d_name, &sb)) ERR("lstat");
    if (S_ISDIR(sb.st_mode)) {
      katalogi++;
    } else if (S_ISREG(sb.st_mode)) {
      pliki++;
    } else if (S_ISLNK(sb.st_mode)) {
      linki++;
    } else {
      inne++;
    }
  }
  if (errno != 0) ERR("readdir");
  if (closedir(dirp)) ERR("closedir");

  printf("files: %d, links: %d, dirs: %d, other: %d\n", pliki, linki, katalogi,
         inne);
}

int main(int argc, char** argv) {
  ScanDir();
  return EXIT_SUCCESS;
}