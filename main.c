#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <time.h>

static int pow10[10] = {1,      10,      100,      1000,      10000,
                        100000, 1000000, 10000000, 100000000, 1000000000};

size_t find_next_newline_split(const char *buf, size_t i, size_t len) {
  for (; i < len; i++)
    if (buf[i] == '\n' || buf[i] == '\0')
      return i;
  return len;
}

int main(void) {
  int fd = open("measurements-1-000-000.txt", O_RDONLY | O_NONBLOCK);
  struct stat sb;
  if (fstat(fd, &sb))
    return -2;

  const char *memory =
      (const char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  // Will kill itself on systems with not all CPUs allocated
  // https://stackoverflow.com/questions/4586405/how-to-get-the-number-of-cpus-in-linux-using-c
  size_t cores = get_nprocs();
  typedef struct {
    size_t begin, end;
  } bound;

  bound *bounds = (bound *)calloc(sizeof(bound), cores);
  memset(bounds, 0x00, sizeof(bound) * cores);

  size_t chunk_size = sb.st_size / cores;
  for (size_t i = 0; i < cores; i++) {
    bounds[i].begin = chunk_size * i;
    size_t next_newline_i =
        find_next_newline_split(memory, chunk_size * (i + 1), sb.st_size);
    bounds[i].end = next_newline_i;
  }

  bounds[cores - 1].end = sb.st_size;

  return 0;

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
