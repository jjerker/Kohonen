#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "bmp.h"
#include "fft.h"
#include "hopfield.h"

void initnetwork(HOPFIELD *net, int nsize) {
  int i;
	
  net->nodes = nsize;
  net->x = (char *)calloc(sizeof(char), nsize);
  net->temp_x = (char *)calloc(sizeof(char), nsize);
  net->w = (WTYPE **)calloc(sizeof(WTYPE *), nsize);
  net->wt = (WTYPE **)calloc(sizeof(WTYPE *), nsize);
  for (i=0; i<net->nodes; i++) {
    net->w[i] = (WTYPE *)calloc(sizeof(WTYPE), nsize);
    net->wt[i] = (WTYPE *)calloc(sizeof(WTYPE), nsize);
  }
}
	

int train_a(HOPFIELD *net, WTYPE **w, char *vector) {
  int i,j;
  int x1,y1, x2, y2;
  float dist;
  for	(i=0; i<net->nodes; i++) {
    vector[i] = vector[i] > 0? 1:-1;
    // printf("%i ", vector[i]);
  }
  
  for (i=0; i<net->nodes; i++) {
    for (j=0; j<net->nodes; j++) {
      if (i!=j) {
	x1 = i / 25; y1 = i % 25;
	x2 = j / 25; y2 = j % 25;
	//dist = 1.0/sqrt(1.0*(x1-x2)*(x1-x2)+1.0*(y1-y2)*(y1-y2));
	dist=1.0;
	w[i][j] += vector[i]*vector[j]*dist;
	//printf("%i %i: vikt: %i\n", i,j, net->w[i][j]);
	//printf("(%i,%i) - (%i, %i) = %f\n", x1,y1,x2,y2,dist);
      }
    }
  }
  return 0;
}

int train(HOPFIELD *net, binaryBitmap *pic) {

  int i;
  float *indata, *outdata;
  char *tvector;
  
  indata = (float*) calloc(sizeof(float), net->nodes);
  outdata = (float*) calloc(sizeof(float), net->nodes);
  tvector = (char*) calloc(sizeof(char), net->nodes);

  /* First, train the 'normal'*/
  printf("Training original\n");
  train_a(net, net->w, pic->data);


  /* Train transformation */
  printf("Training transformation\n");
  for (i=0; i<net->nodes; i++) {
    indata[i] = pic->data[i];
  }
  dct(pic->width, pic->height, indata, outdata);
  for(i=0; i<pic->size; i++) {
    tvector[i] = outdata[i]>0?1:-1;
  }
  
  train_a(net, net->wt, tvector);

  free(indata);
  free(outdata);
  free(tvector);
}
  



int solve_a(HOPFIELD *net, WTYPE **w, char *in, char *out) {
  int i,j, change, iter=0;
  double tmp;
  double sum;
  char buffer[200];


  for (i=0; i<net->nodes; i++) {
    net->x[i] = in[i]> 0? 1:-1;
  }

  while (change) {
    change=0;
    iter++;
    sum=0;
    for (i=0; i<net->nodes; i++) {
      tmp=0;
      for (j=0; j<net->nodes; j++) {
	tmp += w[i][j]*net->x[j];
      }
      sum += tmp;
      //printf("%i: %f\n", i, tmp);
      net->temp_x[i] = ( tmp>0.0?1:-1 );
      if (net->temp_x[i] != net->x[i]) {
	change++;
        //printf("%i, %i -> %i, %f\n", i, net->x[i], net->temp_x[i], tmp);
      }
      net->x[i] = net->temp_x[i];
    }
    printf(" ** Changed: %i, sum: %f\n", change, sum);
    for (i=0; i<net->nodes; i++) {
      net->x[i] = net->temp_x[i];
    }
    sprintf(buffer, "tmp_%i.bmp", iter);
    //writeBMPfile(pic, buffer);
    
  }
  for (i=0; i<net->nodes; i++) {
    out[i] = net->x[i] > 0? 1:0;
  }

  return 0;
}


int solve(HOPFIELD *net, binaryBitmap *inpic, binaryBitmap *outpic) {

  int i;
  char buffer[200];
  
  float *indata, *outdata;
  char *tvector, *tvector2;
  
  indata = (float*) calloc(sizeof(float), net->nodes);
  outdata = (float*) calloc(sizeof(float), net->nodes);
  tvector = (char*) calloc(sizeof(char), net->nodes);
  tvector2 = (char*) calloc(sizeof(char), net->nodes);

  /* First, solve the fransformation */

  for (i=0; i<net->nodes; i++) {
    indata[i] = inpic->data[i];
  }
  dct(inpic->width, inpic->height, indata, outdata);
  for(i=0; i<net->nodes; i++) {
    tvector[i] = outdata[i]>0?1:-1;
  }

  solve_a(net, net->wt, tvector, outpic->data);
  writeBMPfile(outpic, "ires.bmp");
 
  /* Do the inverse transfomration */
  
  for (i=0; i<net->nodes; i++) {
    indata[i] = outpic->data[i];
  }
  idct(inpic->width, inpic->height, indata, outdata);
  for(i=0; i<net->nodes; i++) {
    outpic->data[i] = tvector[i] = outdata[i]>0?1:-1;
  }

  writeBMPfile(outpic, "tres.bmp");
  /* Solve for picture */
  solve_a(net, net->w, tvector, outpic->data);
  writeBMPfile(outpic, "res.bmp");

  free(indata);
  free(outdata);
  free(tvector);
  free(tvector2);


}
  







