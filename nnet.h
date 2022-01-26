#ifdef __cplusplus
extern "C" {
#endif

#include "kohonen.h"


int ktrain(KOHONET *knet, int n, binaryBitmap **pic);
int kmatch(KOHONET *knet, binaryBitmap *pic);

#ifdef __cplusplus
}
#endif
