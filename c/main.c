#include <fcntl.h>
#include <stdio.h>

#include <sys/mman.h>
#include <sys/stat.h>

int main(void) {
  int fd = open("measurements.txt", O_RDONLY | O_NONBLOCK);
  struct stat sb;
  if (fstat(fd, &sb))
    return -1;

  const char *memory =
      (const char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  printf("Hello, world!\n");
}
