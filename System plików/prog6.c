#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FILE_BUF_LEN 256

#define ERR(source)                                                \
  (perror(source), fprintf(stderr, "%s,%d\n", __FILE__, __LINE__), \
   exit(EXIT_FAILURE))

void usage(const char *name) {
  fprintf(stderr, "gdzies sie jebnoles:%s\n", name);
  exit(EXIT_FAILURE);
}

int main(const int argc, const char *const *const argv) {
  if (argc != 3) usage(argv[0]);

  const char *const path1 = argv[1];
  const char *const path2 = argv[2];
  const int fd_1 = open(path1, O_RDONLY);
  const int fd_2 = open(path2, O_WRONLY | O_CREAT, 0777);

  if (fd_1 == -1) ERR("open");
  if (fd_2 == -1) ERR("open");

  char file_buf[FILE_BUF_LEN];
  for (;;) {
    const size_t read_size = read(fd_1, file_buf, FILE_BUF_LEN);
    if (read_size == -1) ERR("read");

    if (read_size == 0) break;

    if (write(fd_2, file_buf, read_size) == -1) ERR("write");
  }

  if (close(fd_2) == -1) ERR("close");

  if (close(fd_1) == -1) ERR("close");
  return EXIT_SUCCESS;
}