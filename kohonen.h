#ifndef KOHONEN_H
#define KOHONEN_H

#ifdef __cplusplus
extern "C" {
#endif

/* kohonen.h */

typedef struct t_kohonet{
  int sx;
  int sy;
  int n;
  float **w;
  float *x;
  int iter;
  int alpha;
  int beta;
  int transform;
} KOHONET;


KOHONET * koho_init(int x, int y, int n, int iter, int alpha, int beta, int trans);
int koho_free(KOHONET *knet);
int koho_match(KOHONET *knet, float *vect);
int koho_train(KOHONET *knet, float nvec, float **mat);
float koho_dist(KOHONET *knet, float *vect1, float *vect2);
binaryBitmap *picfromdata(KOHONET *knet, int w, int h);

#ifdef __cplusplus
}
#endif
#endif
