#include <stdlib.h>

otrans(int size, char *indata, char *outdata) {

  int count, tot, i,j, k, idx=0;

  for (j=1; j<=size; j++) {
    count =0; tot=0;
    for (k=0; k<j; k++) {
      for (i=0; (k+i*j)<size; i++) {
	tot++;
	if (indata[k+i*j] ==1) count++;
      }
    }
    outdata[j-1] = (count*2>tot ? 1:0);
    printf("count: %i, tot: %i\n", count, tot);
  }
  return;
}


rtrans(int size, char *indata, char *outdata) {

  char *tmp, *used;
  int i, idx;

  used = (char *)calloc(sizeof(char), size);
  tmp = (char *)calloc(sizeof(char), size);
  

  srand(0);

  for (i=0; i<size; i++) {
    do {
      idx = rand() % size;
    } while (used[idx]);

    used[idx] = 1;
    tmp[idx] = indata[i];
  }
  for (i=0; i<size; i++) outdata[i] = tmp[i];
  free(used);
  free(tmp);
}

irtrans(int size, char *indata, char *outdata) {

  char *tmp, *used;
  int i, idx;

  used = (char *)calloc(sizeof(char), size);
  tmp = (char *)calloc(sizeof(char), size);
  
  srand(0);

  for (i=0; i<size; i++) {
    do {
      idx = rand() % size;
    } while (used[idx]);

    used[idx] = 1;
    tmp[i] = indata[idx];
  }
  for (i=0; i<size; i++) outdata[i] = tmp[i];
  free(used);
  free(tmp);
}



//itrans(int size, char *indata, char *outdata) {
  
	
