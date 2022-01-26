#include <math.h>
#include <malloc.h>

#include "bmp.h"
#include "kohonen.h"

int callback(int t, binaryBitmap *pic);

KOHONET * koho_init(int x, int y, int n, int iter, int alpha, int beta, int trans) {
  int i,j;

  int mapsize=x*y;

  KOHONET *knet = calloc(sizeof(KOHONET), 1);
  if (!knet)
	  return NULL;

  knet->sx =x;
  knet->sy =y;
  knet->n=n;
  knet->iter = iter;
  knet->alpha = alpha;
  knet->beta = beta;
  knet->transform = trans;

  knet->x = (float *)calloc(sizeof (float), n);
  knet->w = (float **)calloc(sizeof (float *), mapsize);
  for (i=0; i<mapsize; i++) {
    knet->w[i] = (float *)calloc(sizeof (float), n);
  }

  for (i=0; i<mapsize; i++) {
    for (j=0; j<n; j++) {
      knet->w[i][j] = (1.0/1000.0)*(rand()%1000);
    }
  }

  return knet;
}

int koho_free(KOHONET *knet) {
    int i;
	if (knet) {
		for (i=0; i<knet->sx*knet->sy; i++) free(knet->w[i]);
		free(knet->w);
		free(knet->x);
		free(knet);
	}
	return 0;
}


float koho_dist(KOHONET *knet, float *vect1, float *vect2) {
  int idx1, idx2;
  int dx, dy;
  float d;

  idx1=koho_match(knet, vect1);
  idx2=koho_match(knet, vect2);

  dx = idx1%knet->sx-idx2%knet->sx;
  dy = idx1/knet->sy-idx2/knet->sy;
  d =sqrt(1.0*dx*dx+1.0*dy*dy);

  printf("match1: %i, match2: %i, dx: %i, dy: %i, d: %f\n", 
	 idx1, idx2, dx, dy, d);

  return d;
}


int koho_match(KOHONET *knet, float *vect) {

  int i,j;
  int idx=0;
  int mapsize = knet->sx*knet->sy;
  float min, sum,d;

  min = 1e20;
  for (i=0; i<mapsize; i++) {
    sum=0;
    for (j=0; j<knet->n; j++) {
      d =knet->w[i][j]-vect[j];
      sum += d*d;
    }
    //sum = sqrt(sum);
    //printf("sum: %f,   min: %f\n", sum, min);
    if (sum<min) {
      min=sum;
      idx = i;
    }
  }
  return idx;
}
  


int koho_train(KOHONET *knet, float nvec, float **mat) {

  int i,j;
  int idx;
  int dx, dy;
  int t;
  int n;
  int cbres;
  float d, cc;
  int mapsize = knet->sx*knet->sy;
  float *vect;
  char buf[200];
  binaryBitmap *pic;

  if (knet->transform) {
    for (n=0; n<nvec; n++) {
	  dct(64,64,mat[n], mat[n]);
	}
  }

  t=0;
  while (1) {
    t++;

    for (n=0; n<nvec; n++) {
      vect = mat[n];
      /* sök  viktvektor med minsta avvikelse */
    
      idx=koho_match(knet, vect);

#define ALPHA(t) (knet->alpha>0?(1.0*knet->alpha/t):1)
#define RO(t) (1.0*knet->beta/t)

      for (i=0; i<mapsize; i++) {
	dx = i%knet->sx-idx%knet->sx;
	dy = i/knet->sx-idx/knet->sx;
	d =sqrt(1.0*dx*dx+1.0*dy*dy);
	//printf("idx: %i, dx: %i, dy: %i, d=%f\n", i, dx, dy, d);
	cc = ALPHA(t)*exp(-d/RO(t));
	for (j=0; j<knet->n; j++) {
	  knet->w[i][j] += cc*(vect[j]-knet->w[i][j]);
	  //knet->w[i][j] += (vect[j]-knet->w[i][j]);
	}
      }
      
    }
    sprintf(buf, "train_%i.bmp", t);
    pic = picfromdata(knet, 64, 64);
    //printf("Calling callback\n");
    //writegBMPfile(pic, buf);
    cbres=callback(t, pic);
    disposeBinaryBitmap(pic);
    if (t>knet->iter || cbres) break;  
  }
}

  
binaryBitmap *picfromdata(KOHONET *knet, int w, int h) {

  int i,j,k,l,row;
  binaryBitmap *pic = (binaryBitmap *)calloc(sizeof(binaryBitmap),1);
  float *data = (float *)calloc(sizeof(float), w*h);

  pic->data = (char *)calloc(sizeof(char), w*h*knet->sx*knet->sy);
  pic->width=w*knet->sx;
  pic->height=h*knet->sy;
  pic->size = w*h*knet->sx*knet->sy;
  //printf("Picture width: %i, height: %i, size: %i\n", pic->width, pic->height, pic->size);
  

  for (i=0; i<knet->sy; i++) {
    for (j=0; j<knet->sx; j++) {
		if (knet->transform) {
			idct(64,64, knet->w[i*knet->sx+j], data);
		}
      for (k=0; k<h; k++) {
	for (l=0; l<w; l++) {
	  row = (k+i*h);
	  pic->data[row*w*knet->sx+j*w + l] =
		  (knet->transform? data[k*w+l]*255:knet->w[i*knet->sx+j][k*w+l]*255);
	}	
      }
    }
  }
  free(data);
  return pic;
}


    


