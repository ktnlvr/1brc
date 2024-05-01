#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <time.h>

#define THREADS_PER_PROC 2

static int pow10[10] = {1,      10,      100,      1000,      10000,
                        100000, 1000000, 10000000, 100000000, 1000000000};

size_t find_next_newline_split(const char *buf, size_t i, size_t len) {
  for (; i < len; i++)
    if (buf[i] == '\n' || buf[i] == '\0')
      return i;
  return len;
}

typedef struct {
  struct {
    const char *str;
    size_t len;
  } in;

  struct {
    size_t min, max;
  } out;
} thread_data;

void *thread(void *d) {
  thread_data *data = (thread_data *)d;
  printf("%d\n", data->in.len);
  return &data->out;
}

int main(void) {
  int fd = open("measurements-1-000-000.txt", O_RDONLY | O_NONBLOCK);
  struct stat sb;
  if (fstat(fd, &sb))
    return -1;

  const char *memory =
      (const char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  // Will kill itself on systems with not all CPUs allocated
  // https://stackoverflow.com/questions/4586405/how-to-get-the-number-of-cpus-in-linux-using-c
  size_t threads_n = THREADS_PER_PROC * get_nprocs();
  pthread_t *pool = (pthread_t *)calloc(threads_n, sizeof(pthread_t));

  typedef struct {
    size_t begin, end;
  } bound;

  bound *bounds = (bound *)calloc(sizeof(bound), threads_n);
  memset(bounds, 0x00, sizeof(bound) * threads_n);

  size_t chunk_size = sb.st_size / threads_n;
  bounds[0].begin = 0;
  bounds[0].end = find_next_newline_split(memory, chunk_size, sb.st_size);
  for (size_t i = 1; i < threads_n; i++) {
    bounds[i].begin = bounds[i - 1].end;
    size_t next_newline_i =
        find_next_newline_split(memory, chunk_size * (i + 1), sb.st_size);
    bounds[i].end = next_newline_i;
  }
  bounds[threads_n - 1].end = sb.st_size;

  thread_data *data = (thread_data *)calloc(sizeof(thread_data), threads_n);
  for (size_t i = 0; i < threads_n; i++) {
    data[i].in.str = memory;
    data[i].in.len = bounds[i].end - bounds[i].begin;
    pthread_create(&pool[i], NULL, thread, &data[i]);
  }

  for (size_t i = 0; i < threads_n; i++)
    pthread_join(pool[i], NULL);

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
