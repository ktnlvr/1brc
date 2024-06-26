#include <fcntl.h>
#include <limits.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define HASHTABLE_STARTING_SIZE 512

unsigned long polynomial_hash(const char *str, size_t len, size_t size) {
  unsigned long p = 0;

  long pow = 1;
  for (size_t i = 0; i < len; i++) {
    p = (p + str[i] * pow) % 1000000009;
    pow *= 137;
  }

  return p % size;
}

typedef struct bucket bucket;

typedef struct bucket {
  char *str;
  long min, max;
  unsigned long count, sum;
  struct bucket *next;
} bucket;

bucket bucket_new() {
  bucket b;
  b.str = 0;
  b.max = LONG_MIN;
  b.min = LONG_MAX;
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
  map.capacity = HASHTABLE_STARTING_SIZE;
  map.buckets = (bucket *)malloc(map.capacity * sizeof(bucket));

  for (size_t i = 0; i < map.capacity; i++)
    map.buckets[i] = bucket_new();

  return map;
}

size_t min(size_t a, size_t b) { return (a > b) * b + (b >= a) * a; }

bucket *hashmap_get(hashmap *map, const char *str, size_t len) {
  unsigned long hash = polynomial_hash(str, len, map->capacity);

  bucket *b = &map->buckets[hash];
  if (map->buckets[hash].str == 0) {
    map->buckets[hash].str = strndup(str, len);
  } else {
    for (; b != 0; b = b->next)
      if (!strncmp(str, b->str, min(len, strlen(b->str))))
        break;

    // If no bucket exists, insert after the first one
    if (!b) {
#ifdef REPORT_HASH_COLLISIONS
      fprintf(stderr, "HASH COLLISION for %.*s\n", len, str);
#endif
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
    bucket->count++;

    while (memory[i] != '\n')
      i++;
  }

  for (size_t i = 0; i < map.capacity; i++) {
    if (map.buckets[i].str == 0)
      continue;

    for (bucket *b = &map.buckets[i]; b != 0; b = b->next)
      printf("%s %d\n", b->str, b->count);
  }
}
