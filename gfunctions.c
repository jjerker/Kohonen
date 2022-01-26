// "Graceful" versions of malloc, fopen, fclose

#include <stdlib.h>
#include <stdio.h>

#include "gfunctions.h"

void *gmalloc(const int n)
{
  void *p;

  if ((p = malloc(n)) == NULL)
  {
    fprintf(stderr, "\nUnable to allocate memory\n");
    exit(1);
  }
  return p;
}

void *gcalloc(const int n, const int el_size)
{
  void *p;

  if ((p = calloc(n, el_size)) == NULL)
  {
    fprintf(stderr, "\nUnable to allocate memory\n");
    exit(1);
  }
  return p;
}

FILE *gfopen(const char *filename, const char *mode)
{
  FILE *fp;

  if ((fp = fopen(filename, mode)) == NULL)
  {
    fprintf(stderr, "\nUnable to open file: %s\n", filename);
    exit(1);
  }
  return fp;
}

int gfclose(FILE *fp)
{
  int returnV;
  if ((returnV = fclose(fp)) == EOF)
  {
    fprintf(stderr, "\nError while closing a file\n");
  }
  return returnV;
}
