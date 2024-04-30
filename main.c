#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>

static int pow10[10] = {1,      10,      100,      1000,      10000,
                        100000, 1000000, 10000000, 100000000, 1000000000};

int main(void) {
  FILE *file = fopen("measurements.txt", "r");
  if (file == NULL)
    return -1;

  int fd = fileno(file);
  struct stat sb;
  if (fstat(fd, &sb))
    return -2;

  const char *memory =
      (const char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  int min = INT_MAX, max = INT_MIN;
  size_t idx = 0;
  while (idx < sb.st_size) {
    if (memory[idx++] == ';') {
      int neg = 1;
      if (memory[idx] == '-')
        neg = -1, idx++;
      size_t l = 0;
      while (memory[idx++] != '.')
        l++;

      int x = memory[idx] - '0';
      for (size_t i = 0; i < l; i++)
        x += pow10[l - i] * (memory[idx - l - 1 + i] - '0');
      x *= neg;

      if (x < min)
        min = x;
      if (x > max)
        max = x;
    }
  }

  printf("%d %d\n", min, max);
}
