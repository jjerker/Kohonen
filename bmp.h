#ifdef __cplusplus
extern "C" {
#endif


#ifndef _BMP_H_
#define _BMP_H_

#include <stdio.h>

#define DEFAULT_RESOLUTION             0x0EC4
#define DATA_OFFSET_FOR_BINARY_BITMAPS 0x3E
#define BITMAP_HEADER_SIZE             0x28

typedef struct binaryBitmap
{
  int width;
  int height;
  int size;
  unsigned char *data;
} binaryBitmap;

// Declarations
binaryBitmap *readBMPfile(char *filename);
void writeBMPfile(binaryBitmap *bBMP, char *filename);
void printASCIIBitmap(binaryBitmap *bBMP, FILE *fp);

binaryBitmap *constructBinaryBitmap(int width, int height, int size);
void disposeBinaryBitmap(binaryBitmap *bBMP);

#endif

#ifdef __cplusplus
}
#endif
