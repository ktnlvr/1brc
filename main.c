#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRBUF_SIZE 32

int main(void) {
  FILE *file = fopen("measurements.txt", "r");
  if (file == NULL)
    return -1;

  char string_buf[STRBUF_SIZE];
  int min = INT_MAX, max = INT_MIN;
  double aa;
  size_t idx = 0;

  do {
    char c = fgetc(file);
    string_buf[idx++] = c;

    if (c == '\n') {
      string_buf[idx - 1] = '\0';
      size_t n = idx;
      while (string_buf[n] != ';')
        n--;
      char *number_str = string_buf + n + 1;
      int neg = 1;
      if (number_str[0] == '-') {
        number_str++;
        neg = -1;
      }

      n = 0;
      while (number_str[n] != '.')
        n++;

      int x = (atoi(number_str) * 10 + atoi(number_str + n + 1)) * neg;
      if (x > max)
        max = x;
      if (x < min)
        min = x;

      idx = 0;
      memset(string_buf, sizeof(char), STRBUF_SIZE);
    }
  } while (!feof(file));

  printf("%d %d\n", min, max);
}
