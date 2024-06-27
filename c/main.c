#include <fcntl.h>
#include <limits.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define HASHTABLE_SIZE 512
#define MOD 1000000009

#define MIN(a, b) (a > b ? b : a)
#define MAX(a, b) (a > b ? a : b)

unsigned long polynomial_hash(const char *str, size_t len) {
  unsigned long p = 0;

  unsigned long pow = 1;
  for (size_t i = 0; i < len; i++) {
    p = (p + (str[i] - ' ' + 1) * pow) % MOD;
    pow = (pow * 53) % MOD;
  }

  return p % HASHTABLE_SIZE;
}

typedef struct bucket bucket;

typedef struct bucket {
  char *str;
  int min, max;
  unsigned long count, sum;
  struct bucket *next;
} bucket;

bucket bucket_new() {
  bucket b;
  b.str = 0;
  b.max = INT_MIN;
  b.min = INT_MAX;
  b.count = 0;
  b.sum = 0;
  b.next = 0;
  return b;
}

typedef struct {
  bucket *buckets;
  size_t capacity;
} hashmap;

hashmap hashmap_new() {
  hashmap map;
  map.capacity = HASHTABLE_SIZE;
  map.buckets = (bucket *)malloc(map.capacity * sizeof(bucket));

  for (size_t i = 0; i < map.capacity; i++)
    map.buckets[i] = bucket_new();

  return map;
}

bucket *hashmap_get(hashmap *map, const char *str, size_t len) {
  unsigned long hash = polynomial_hash(str, len);

  bucket *b = &map->buckets[hash];
  if (map->buckets[hash].str == 0) {
    map->buckets[hash].str = strndup(str, len);
  } else {
    for (; b != 0; b = b->next)
      if (!strncmp(str, b->str, MIN(len, strlen(b->str))))
        break;

    // If no bucket exists, insert after the first one
    if (!b) {
      fprintf(stderr, "HASH COLLISION for %.*s\n", len, str);
      bucket *new_b = (bucket *)malloc(sizeof(bucket));
      *new_b = bucket_new();
      new_b->str = strndup(str, len);

      bucket *first = &map->buckets[hash];
      bucket *first_next = first->next;

      first->next = new_b;
      new_b->next = first_next;
      b = new_b;
    }
  }

  return b;
}

const int pow10[10] = {10, 100, 1000, 1000, 10000};

int main(void) {
  int fd = open("measurements.txt", O_RDONLY | O_NONBLOCK);
  struct stat sb;
  if (fstat(fd, &sb))
    return -1;

  size_t len = sb.st_size;
  const char *memory =
      (const char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);

  hashmap map = hashmap_new();

  for (size_t i = 0; i < len; i++) {
    size_t j = i;
    while (memory[i] != ';')
      i++;
    bucket *bucket = hashmap_get(&map, memory + j, i - j);
    i++;

    bucket->count++;

    j = i;
    while (memory[i] != '\n')
      i++;

    int s = 1;
    if (memory[j] == '-') {
      s = -1;
      j++;
    }

    int n = memory[i - 1] - '0';
    for (int k = j; k < i - 2; k++)
      n += (memory[k] - '0') * pow10[i - k - 3];
    n *= s;

    bucket->max = MAX(bucket->max, n);
    bucket->min = MIN(bucket->min, n);
    bucket->sum += n;
  }

  for (size_t i = 0; i < map.capacity; i++) {
    if (map.buckets[i].str == 0)
      continue;

    for (bucket *b = &map.buckets[i]; b != 0; b = b->next) {
      int max_major = b->max / 10;
      int max_minor = abs(b->max % 10);

      int min_major = b->min / 10;
      int min_minor = abs(b->min % 10);

      printf("%s    max: %d.%d min: %d.%d avg: %.1lf\n", b->str, max_major,
             max_minor, min_major, min_minor,
             (double)b->sum / (10. * b->count));
    }
  }
}
