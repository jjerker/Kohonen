#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "bmp.h"
#include "fft.h"
#include "hopfield.h"
#include "kohonen.h"
#include "nnet.h"

scramble(int size, char *pic, float p) {
  
  int i;

  for (i=0; i<size; i++) {
    if (rand()<1.0*RAND_MAX*p) {
      pic[i] = pic[i] == 1 ? 0:1;
    }
  }
}


int hopfield_main() {
  HOPFIELD net,tnet;
  int i;
  binaryBitmap *pic1;
  binaryBitmap *pic2;
  binaryBitmap *pic3;
  binaryBitmap *pic4;
  binaryBitmap *pic5;
  binaryBitmap *pic6;
  binaryBitmap *pic7;
  binaryBitmap *start;
  binaryBitmap *res;
  float *indata, *outdata, max;


  char x1[10] = { 1,1,1,-1,1,-1,-1,-1,1,1 };
  char x2[10] = { 1,1,-1,1,-1,1,1,-1,1,1 };
  char x3[10] = { 1,1,-1,1,-1,-1,-1,-1,1,1 };

  char s1[16] = { 0,0,0,1,1,0,1,1,1,0,1,1,0,1,1,0 };
  char s3[16] = { 1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1 };
  char s2[8] = { 1,1,1,-1,1,-1,-1,-1 };

  otrans(16, s1,s3);

  for (i=0; i<16; i++) {
    printf("%i \n", s3[i]);
  }
  
/*    pic1 = readBMPfile("s.bmp"); */
/*    pic2 = readBMPfile("z.bmp"); */
/*    pic3 = readBMPfile("n.bmp"); */
/*    pic4 = readBMPfile("e.bmp"); */
/*    pic5 = readBMPfile("t.bmp"); */

  pic1 = readBMPfile("Jerker.bmp");
  pic2 = readBMPfile("mickey.bmp");
  printf("Read pic, size: %i x %i, data: %i\n", pic2->width, pic2->height, 
	 pic2->size);
  pic3 = readBMPfile("KajMikael.bmp");
  printf("Read pic, size: %i x %i, data: %i\n", pic3->width, pic3->height, 
	 pic3->size);
  pic4 = readBMPfile("Janne.bmp");
  printf("Read pic, size: %i x %i, data: %i\n", pic4->width, pic4->height, 
	 pic4->size);
  pic5 = readBMPfile("donald.bmp");
  printf("Read pic, size: %i x %i, data: %i\n", pic5->width, pic5->height, 
	 pic5->size);
  pic6 = readBMPfile("Stefan.bmp");
  printf("Read pic, size: %i x %i, data: %i\n", pic6->width, pic6->height, 
	 pic6->size);

/*    rtrans(pic1->size, pic1->data, pic1->data); */
/*    rtrans(pic2->size, pic2->data, pic2->data); */
/*    rtrans(pic3->size, pic3->data, pic3->data); */
/*    rtrans(pic4->size, pic4->data, pic4->data); */
/*    rtrans(pic5->size, pic5->data, pic5->data); */
/*    rtrans(pic6->size, pic6->data, pic6->data); */


  initnetwork(&net, pic1->size);
  //train(&net, pic1);
  train(&net, pic2);
  train(&net, pic3);
  train(&net, pic4);
  //train(&net, pic5);
  //train(&net, pic6);
	
  start = readBMPfile("mickey_ud.bmp");
  start = start;
  printf("Read pic, size: %i x %i, data: %i\n", pic3->width, pic3->height, 
	 pic3->size);

  //for (i=0; i<start->size/2; i++) 
  //  start->data[i] = pic2->data[i];
  //scramble(start->size, start->data, 0.30);
  writeBMPfile(start, "start.bmp");
  //rtrans(start->size, start->data, start->data);

  res = pic1;
  solve(&net, start, res);
  //  irtrans(res->size, res->data, res->data);

  for (i=0; i<res->size; i++) {
    //printf("%i: %i\n", i, res->data[i]);
  }

}

float kdist(KOHONET *knet, binaryBitmap *pic1, binaryBitmap *pic2) {

  int i;
  float d;
  float *array1 = calloc(sizeof(float), knet->n);
  float *array2 = calloc(sizeof(float), knet->n);

  for (i=0; i<knet->n; i++) {
    array1[i] = pic1->data[i]/255.0;
    array2[i] = pic2->data[i]/255.0;
  }
  d=koho_dist(knet, array1, array2);
  free(array1);
  free(array2);
  return d;
}

int ktrain(KOHONET *knet, int n, binaryBitmap **pic) {
  int i,k;

  float **mat = (float **)calloc(sizeof(float *), n);
  for (k=0; k<n; k++) 
    mat[k] = (float *)calloc(sizeof(float), knet->n);
  

  printf("Training %i pictures\n", n);
  for (k=0; k<n; k++) {    
    for (i=0; i<knet->n; i++) {
      mat[k][i] = pic[k]->data[i]/255.0;
    }
 
  }
  i=koho_train(knet, n, mat);
  for (k=0; k<n; k++) 
    free(mat[k]);
  free(mat);
  return i;
}




int kmatch(KOHONET *knet, binaryBitmap *pic) {
  int i,j;

  float *array;
  if (!knet) return -1;

  array = calloc(sizeof(float), knet->n);
  for (i=0; i<knet->n; i++) {
    array[i] = pic->data[i]/255.0;
  }
  if (knet->transform)
    dct(pic->width, pic->height, array, array);  

  i=koho_match(knet, array);
  free(array);
  return i;
}


int koho_main() {
  KOHONET *knet;
  int i,j;
  binaryBitmap *start;
  binaryBitmap *res;
  binaryBitmap *tpics[20];
  float *indata, *outdata, max;
  char *startn;
  char *spics[20];
  char *stpics[20];
  char buf[200];

  char x1[10] = { 1,1,1,-1,1,-1,-1,-1,1,1 };
  char x2[10] = { 1,1,-1,1,-1,1,1,-1,1,1 };
  char x3[10] = { 1,1,-1,1,-1,-1,-1,-1,1,1 };

  char s1[16] = { 0,0,0,1,1,0,1,1,1,0,1,1,0,1,1,0 };
  char s3[16] = { 1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1 };
  char s2[8] = { 1,1,1,-1,1,-1,-1,-1 };

#define NTRAINPICS 11
  stpics[0] = "atta.bmp";
  stpics[1] = "bush.bmp";
  stpics[2] = "binladin.bmp";
  stpics[3] = "gorilla.bmp";
  stpics[4] = "putin.bmp";
  stpics[5] = "castro.bmp";
  stpics[6] = "blair.bmp";
  stpics[7] = "hitler.bmp";
  stpics[8] = "bond.bmp";
  stpics[9] = "donaldg.bmp";
  stpics[10] = "daniela.bmp";

  for (i=0; i<NTRAINPICS; i++) {
    sprintf(buf, "/remote/attack/jbjorkqv/stud/nnet/prog/%s", stpics[i]);
    tpics[i] = readBMPfile(buf);
    sprintf(buf, "in_%i.bmp", i);
    writegBMPfile(tpics[i], buf);    
  }

  knet = koho_init(15,15, tpics[0]->size, 100, 10, 50,0);
  ktrain(knet, NTRAINPICS, tpics);

  kmatch(knet, tpics[0]);
  res = picfromdata(knet, tpics[0]->width, tpics[0]->height);
  writegBMPfile(res, "res.bmp");
  disposeBinaryBitmap(res);

  
  return 0;

#define NSTARTPICS 6
  spics[0] = "Jerkerg_ud.bmp";
  spics[1] = "mickeyg_ud.bmp";
  spics[2] = "KajMikaelg_ud.bmp";
  spics[3] = "Janneg_ud.bmp";
  spics[4] = "donaldg_ud.bmp";
  spics[5] = "Stefang_ud.bmp";

  for (i=0; i<NSTARTPICS; i++) {
    char buf[200];
 
    start = readBMPfile(spics[i]);
    
    //scramble(start->size, start->data, i/10.0);
    //sprintf(buf, "%s.%i.bmp", startn, i*10);
    //    writeBMPfile(start, buf);
    //printf("\n***** using picture '%s'\n", buf);
    for (j=0; j<NTRAINPICS; j++) {
      printf("distance between start %i and train %i: %f\n", i,j,kdist(knet, start, tpics[j]));
    }
  }
  return 0;
}







