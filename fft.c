#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "fft.h"


#define PI 3.141592654


void dct_a(int size, float *in, float *out) {
  float sum,sum1,t;
  int k,n;

  for (k=0; k<size; k++) {
    sum=0;
    sum1=0;
    for (n=0; n<size; n++) {
      sum+=in[n]*cos(PI*k*(n+0.5)/size);
      //sum1+=in[n]*sin(PI*k*(n+0.5)/size);
    }
    //out[k] = (sum * (double)(k==0?1.0/sqrt(size):sqrt(2.0/size)));
    out[k] = (sum * (double)(k==0?1.0/sqrt(size):sqrt(2.0/size)));
    //    printf("---------- %f\n", out[k]);
  }
}

void idct_a(int size, float *in, float *out) {
  float sum,sum1,t;
  int k,n;

  for (k=0; k<size; k++) {
    sum=0;
    sum1=0;
    for (n=0; n<size; n++) {
      sum+=in[n]*cos(PI*k*(n+0.5)/size);
      //sum1+=in[n]*sin(PI*n*(k+0.5)/size);
    }
    out[k] = (sum * (double)(k==0?1.0/sqrt(size):sqrt(2.0/size)));
    //out[k+size] = (1.0*(sum)/size);
  }
}


void dct(int hsize, int vsize, float *in, float *out) {

  int x,y,d,num;
  float **mat;
  float *xarray;
  float *yarray;
  float *yarray_1;
  float *temp;

  xarray = (float *)calloc(sizeof(float), hsize);
  yarray = (float *)calloc(sizeof(float), vsize);
  yarray_1 = (float *)calloc(sizeof(float), vsize);
  //temp = (float *)calloc(sizeof(float), vsize*hsize);
  mat = (float **)calloc(sizeof(float*), vsize);
  for (y=0; y<vsize; y++) {
    mat[y] = (float *)calloc(sizeof(float), hsize);
  }

  //  dct_a(vsize*hsize, in, temp);
  // memcpy(out, temp, sizeof(float)*vsize*hsize);
  //return;

  /* In x: */
  
  for (y=0; y<vsize; y++) {
    for (x=0; x<hsize; x++) {
      xarray[x] = in[y*hsize+x];
    }
    dct_a(hsize, xarray, mat[y]);
  }

  /* in y: */  
  for (x=0; x<hsize; x++) {
    for (y=0; y<vsize; y++) {
      yarray[y] = mat[y][x];
    }
    dct_a(vsize, yarray, yarray_1);
    for (y=0; y<vsize; y++) {
      mat[y][x] = yarray_1[y];
    }
  }

  x=y=0; d = 1;
  num=0;
  while (x<hsize && y<vsize) {
    num++;
    //printf("(dir: %i), num: %i (%i, %i) = %f\n", d, num,x,y,mat[y][x]);
    x += d;
    if (x<0) {x=0; y+=2; d=-d;}
    if (x>=hsize) {x=hsize-1; d=-d;}
    y -=d;
    if (y<0) {y=0; d=-d;} 
    if (y>=vsize) {y=vsize-1; x+=2; d=-d;}
  }    



  //printf("\n\n****\n\n");
  for (y=0; y<vsize; y++) {
    for (x=0; x<hsize; x++) {
      out[y*hsize+x] = mat[y][x];
      //printf("%f ", out[y*hsize+x]);
    }
    //printf("\n");
  }

  free(xarray);
  free(yarray);
  free(yarray_1);
  for (y=0; y<vsize; y++) {
    free(mat[y]);
  }
  free(mat);

}

void idct(int hsize, int vsize, float *in, float *out) {

  int x,y,d,num;
  float **mat;
  float *xarray;
  float *yarray;
  float *yarray_1;

  xarray = (float *)calloc(sizeof(float), hsize);
  yarray = (float *)calloc(sizeof(float), vsize);
  yarray_1 = (float *)calloc(sizeof(float), vsize);
  mat = (float **)calloc(sizeof(float*), vsize);
  for (y=0; y<vsize; y++) {
    mat[y] = (float *)calloc(sizeof(float), hsize);
  }



  /* in y: */  
  for (x=0; x<hsize; x++) {
    for (y=0; y<vsize; y++) {
      yarray[y] = in[y*hsize+x];
    }
    idct_a(vsize, yarray, yarray_1);
    for (y=0; y<vsize; y++) {
      //printf("%i %f %f\n", y, yarray[y], yarray_1[y]);
      mat[y][x] = yarray_1[y];
    }
  }

  for (y=0; y<vsize; y++) {
    for (x=0; x<hsize; x++) {
      out[y*hsize+x] = mat[y][x];
      //printf("%4f ", mat[y][x]);
    }
    //printf("\n");
  }


  /* In x: */

  for (y=0; y<vsize; y++) {
    for (x=0; x<hsize; x++) {
      xarray[x] = mat[y][x];
    }
    idct_a(hsize, xarray, mat[y]);
  }

  x=y=0; d = 1;
  num=0;
  while (x<hsize && y<vsize) {
    num++;
    //    printf("(dir: %i), num: %i (%i, %i) = %f\n", d, num,x,y,mat[y][x]);
    x += d;
    if (x<0) {x=0; y+=2; d=-d;}
    if (x>=hsize) {x=hsize-1; d=-d;}
    y -=d;
    if (y<0) {y=0; d=-d;} 
    if (y>=vsize) {y=vsize-1; x+=2; d=-d;}
  }    
  for (y=0; y<vsize; y++) {
    for (x=0; x<hsize; x++) {
      out[y*hsize+x] = mat[y][x];
      //printf("%f ", out[y*hsize+x]);
    }
    // printf("\n");
  }
  free(xarray);
  free(yarray);
  free(yarray_1);
  for (y=0; y<vsize; y++) {
    free(mat[y]);
  }
  free(mat);

}





void wavelet(int size, char *in, char *out) {
	
	int i;
	int h = size;
	float *ss, *s;
	ss= (float *)calloc(sizeof(float), size);
	s= (float *)calloc(sizeof(float), size);

	for (i=0; i<size; i++) {
		s[i] = in[i]/sqrt(size);
		//printf("%f\n", s[i]);
	}

	while (h>=1) {
			
		for	(i=0;i<(int)(h/2)-1; i++) {
			ss[i] = (s[2*i]+s[2*i+1])/sqrt(2.0); 
			ss[(int)(h/2)+1] = (s[2*i]-s[2*i+1])/sqrt(2.0);
		}
		for (i=0; i<size; i++){
			s[i] = ss[i];
			//printf("%i, %i: %f\n", h, i, s[i]);
		}
			
		h=h/2;
	}
	for (i=0; i<size; i++)
		out[i] = ss[i];
	
}


