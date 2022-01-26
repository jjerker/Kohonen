
#ifndef _GFUNCTIONS_H_
#define _GFUNCTIONS_H_

#include <stdio.h>

void *gmalloc(const int n);
void *gcalloc(const int n, const int el_size);
FILE *gfopen(const char *filename, const char *mode);
int gfclose(FILE *fp);

#endif
