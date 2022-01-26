
/* hopfield.h */

typedef float WTYPE;


struct t_hopfield {
  int nodes;
  char *x;
  char *temp_x;
  WTYPE **w;
  WTYPE **wt;
};

typedef struct t_hopfield HOPFIELD;



void initnetwork(HOPFIELD *net, int nsize);
int train(HOPFIELD *net, binaryBitmap *pic);
int solve(HOPFIELD *net, binaryBitmap *inpic, binaryBitmap *outpic);



