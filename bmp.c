
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "gfunctions.h"
#include "bmp.h"

// Local declarations
long getFilesize(FILE *fp);
unsigned int get2bytes(FILE *fp);
unsigned int get4bytes(FILE *fp);
void put2bytes(unsigned int value, FILE *fp);
void put4bytes(unsigned int value, FILE *fp);

int getnbits(char *buf, int start, int len) {
  int i;
  unsigned int val;
  static intsize = sizeof(unsigned int);
  static intbits = sizeof(unsigned int)*8;

  //printf("size: %i\n", sizeof(char));
  val = *(unsigned int *)(void *)(&buf[start/8]);
  val = val<<start%8;
  val = val>>(intbits-len);

  //  printf("%i bytes at %i: %x\n", len, start, val);

  //  for (i=0; i<start/8+1; i++) {
  //  printf("%c ", (char)buf[i]);
  //}

  return val;
}


binaryBitmap *readBMPfile(char *filename)
{
  int filesize, datasize, dataOffset;
  int i,j;
  int bytesLeftOnCurrentLine;
  int pos, endPos;
  int ch;
  int colors;
  int colorsused, colorsimportant;
  int colortable[256];

  binaryBitmap *bBMP = (binaryBitmap*) gcalloc(1, sizeof(binaryBitmap));

  FILE *fp = gfopen(filename, "rb");

  filesize = getFilesize(fp);
  if (filesize <= DATA_OFFSET_FOR_BINARY_BITMAPS)
  {
    free(bBMP);
    gfclose(fp);
    fprintf(stderr, "%s is too small\n", filename);
    return 0;
  }

  // The first two bytes must be "BM"
  if (fgetc(fp) != 'B' || fgetc(fp) != 'M' || get4bytes(fp) != filesize)
  {
    free(bBMP);
    gfclose(fp);
    fprintf(stderr, "%s is not a BMP-file\n", filename);
    return 0;
  }

  // Move 4 bytes forward
  fseek(fp, 4, SEEK_CUR);

  // Offset to beginning of bitmap data in bytes
  dataOffset = get4bytes(fp);

  // Skipping bitmap header size, always 0x28
  fseek(fp, 4, SEEK_CUR);

  // Read the width and height
  bBMP->width = get4bytes(fp);
  bBMP->height = get4bytes(fp);

  // Check number of planes in the bitmap, must be 1
  // Check bits per pixel, must be 1
  // The file must not be compressed

  if (get2bytes(fp) != 1)
  {
    free(bBMP);
    gfclose(fp);
    fprintf(stderr, "%s has more than one plane, cant be used\n", filename);
    return 0;
  }

  if ((colors = get2bytes(fp)) > 8)
  {
    free(bBMP);
    gfclose(fp);
    fprintf(stderr, "%s contains more that 256 colors can not be used\n", filename);
    return 0;
  }

  if (get4bytes(fp) != 0)
  {
    free(bBMP);
    gfclose(fp);
    fprintf(stderr, "%s is compressed, can not be used\n", filename);
    return 0;
  }

  // Check the size of the bitmap data
  datasize = get4bytes(fp);
  if (datasize != (filesize - dataOffset))
  {
    free(bBMP);
    gfclose(fp);
    fprintf(stderr, "%s is corrupt\n", filename);
    return 0;
  }

  // Move 16 bytes forward
  fseek(fp, 8, SEEK_CUR);

  colorsused = get4bytes(fp);
  colorsimportant = get4bytes(fp);

  //printf("bits: %i, used: %i, imp, %i\n", colors, colorsused, colorsimportant);

  // Check the palette
  if (colors==1) {
    if (get4bytes(fp) != 0 || get4bytes(fp) != 0x00FFFFFF)
      {
	fprintf(stderr, "The palette in %s is not the prefered one\n", filename);
      }
  }
  else
    {
      for (i=0; i< (colorsused>0?colorsused:(1<<colors)); i++) 
	{
	  colortable[i] = get4bytes(fp);
	  colortable[i] = ((colortable[i]&0xFF) + ((colortable[i]&0xFF00)>>8) 
			   + ((colortable[i]&0xFF00)>>8))/3;
	  //printf("color %i = %x\n", i, colortable[i]);
	}
    }


  assert(ftell(fp) == dataOffset);

  // Calculate the size of the bitmap in pixels and allocate memory
  bBMP->size = bBMP->width * bBMP->height;
  bBMP->data = (char*) gmalloc(bBMP->size);

  if (colors>1) {
    char *buf = gmalloc(sizeof(char)*(bBMP->width+4));
    int value;
    for (i=0; i<bBMP->height; i++) { 
      /* Read line on 4 byte boundary */
      fread(buf, sizeof(char), (bBMP->width*colors/32 )*4, fp);
      for (j=0; j<bBMP->width; j++) {
	bBMP->data[i*bBMP->height+j] = colortable[getnbits(buf, j*colors, colors)];
      }
    }
    free(buf);
    //printf("ftell: %i, fsize: %i\n", ftell(fp), getFilesize(fp));
    gfclose(fp);
    return bBMP;
  }
  

  // Get the bitmap data
  bytesLeftOnCurrentLine = 0;
  pos = 0;
  for (i = 0; i < datasize; i++)
  {
    if (bytesLeftOnCurrentLine == 0 && (i % 4) == 0)
    {
      bytesLeftOnCurrentLine = bBMP->width;
    }

    ch = fgetc(fp);

    if (bytesLeftOnCurrentLine > 8)
    {
      endPos = pos + 8;
      bytesLeftOnCurrentLine -= 8;
    }
    else
    {
      endPos = pos + bytesLeftOnCurrentLine;
      bytesLeftOnCurrentLine = 0;
    }

    while (pos < endPos)
    {
      bBMP->data[pos] = ((ch >> 7) & 1)*255;
      pos++;
      ch <<= 1;
    }
  }

  gfclose(fp);
  return bBMP;
}

void writeBMPfile(binaryBitmap *bBMP, char *filename)
{
  FILE *fp = gfopen(filename, "wb");

  int filesize, datasize, dataOffset;
  int row, col, n;

  // Set to binary

  for (n=0; n<bBMP->size; n++) {
    bBMP->data[n] = bBMP->data[n] == 1?1:0;
  }

  // Calculate the datasize for binary bitmaps
  datasize = 4 * ((((bBMP->width + 7) / 8) + 3) / 4) * bBMP->height;

  // Dataoffset and filesize
  dataOffset = DATA_OFFSET_FOR_BINARY_BITMAPS;
  filesize = dataOffset + datasize;

  // Put a B and an M at the beginning of the file
  fputc('B', fp);
  fputc('M', fp);

  // The filesize shall be here! Return to this position later
  put4bytes(filesize, fp);

  // Just four dummy bytes
  put4bytes(0, fp);

  // Offset to beginning of bitmap data
  put4bytes(dataOffset, fp);

  // Bitmap header size
  put4bytes(BITMAP_HEADER_SIZE, fp);

  // Horizontal width and vertical height of the bitmap in pixels
  put4bytes(bBMP->width, fp);
  put4bytes(bBMP->height, fp);

  // Number of planes
  put2bytes(1, fp);

  // Bits per pixels (allowed values: 1, 4, 8, 16, 24, 32)
  put2bytes(1, fp);

  // Compression, 0 means no compression
  put4bytes(0, fp);

  // Size of the bitmap data
  put4bytes(datasize, fp);

  // Horizontal and vertical resolution in pixels per meter
  put4bytes(DEFAULT_RESOLUTION, fp);
  put4bytes(DEFAULT_RESOLUTION, fp);

  // Number of colours used, NOT USED
  put4bytes(0, fp);

  // Important colours, NOT USED
  put4bytes(0, fp);

  // Palette info when we have one bit per pixel
  put4bytes(0x00000000, fp);
  put4bytes(0x00FFFFFF, fp);

  for (row = 0; row < bBMP->height; row++)
  {
    n = 0;
    for (col = 0; col < bBMP->width; col++)
    {
      n <<= 1;
      n |= bBMP->data[col + row*bBMP->width];
      if ((col + 1) % 32 == 0)
      {
        fputc((n >> 24) & 0x00FF, fp);
        fputc((n >> 16) & 0x00FF, fp);
        fputc((n >> 8) & 0x00FF, fp);
        fputc(n & 0x00FF, fp);
      }
    }
    if ((col % 32) > 0)
    {
      n <<= (32 - col % 32);
      fputc((n >> 24) & 0x00FF, fp);
      fputc((n >> 16) & 0x00FF, fp);
      fputc((n >> 8) & 0x00FF, fp);
      fputc(n & 0x00FF, fp);
    }
  }

  gfclose(fp);
}

void writegBMPfile(binaryBitmap *bBMP, char *filename)
{
  FILE *fp = gfopen(filename, "wb");

  int filesize, datasize, dataOffset;
  int row, col, n;
  int dataoffsetoffset;
  int filesizeoffset;
  int tmp, i;


  // Calculate the datasize for grey bitmaps
  datasize = 4 * (((bBMP->width) + 3) / 4) * bBMP->height;


  // Dataoffset and filesize
  dataOffset = DATA_OFFSET_FOR_BINARY_BITMAPS;
  filesize = dataOffset + datasize;

  // Put a B and an M at the beginning of the file
  fputc('B', fp);
  fputc('M', fp);

  // The filesize shall be here! Return to this position later
  filesizeoffset = ftell(fp);
  put4bytes(filesize, fp);

  // Just four dummy bytes
  put4bytes(0, fp);

  // Offset to beginning of bitmap data
  dataoffsetoffset =ftell (fp);
  put4bytes(dataOffset, fp);

  // Bitmap header size
  put4bytes(BITMAP_HEADER_SIZE, fp);

  // Horizontal width and vertical height of the bitmap in pixels
  put4bytes(bBMP->width, fp);
  put4bytes(bBMP->height, fp);

  // Number of planes
  put2bytes(1, fp);

  // Bits per pixels (allowed values: 1, 4, 8, 16, 24, 32)
  put2bytes(8, fp);

  // Compression, 0 means no compression
  put4bytes(0, fp);

  // Size of the bitmap data
  put4bytes(datasize, fp);

  // Horizontal and vertical resolution in pixels per meter
  put4bytes(DEFAULT_RESOLUTION, fp);
  put4bytes(DEFAULT_RESOLUTION, fp);

  // Number of colours used, NOT USED
  put4bytes(256, fp);

  // Important colours, NOT USED
  put4bytes(256, fp);

  // Write palatte info

  for (i=0; i<256; i++) {
    tmp = i | (i<<8) | (i<<16);
    put4bytes(tmp, fp);
  }
  
  dataOffset=ftell(fp);

  for (row=0; row<bBMP->height; row++) {
    n=0;
    for (col = 0; col < bBMP->width; col++) {
      fputc(bBMP->data[col + row*bBMP->width], fp);
    }
    while (col++%4) fputc(0, fp);
  }
  filesize=ftell(fp);

  fseek(fp, filesizeoffset, SEEK_SET);
  put4bytes(filesize, fp);
    
  fseek(fp, dataoffsetoffset, SEEK_SET);
  put4bytes(dataOffset, fp);
  

  gfclose(fp);
}


void printASCIIBitmap(binaryBitmap *bBMP, FILE *fp)
{
  int row, col;

  for (row = bBMP->height - 1; row >= 0; row--)
  {
    for (col = 0; col < bBMP->width; col++)
    {
      if (bBMP->data[col + row*bBMP->width] == 1)
      {
        fprintf(fp, "  ");
      }
      else
      {
        fprintf(fp, "@ ");
      }
    }
    fputc('\n', fp);
  }
}

binaryBitmap *constructBinaryBitmap(int width, int height, int size)
{
  binaryBitmap *bBMP;
  if (width * height != size)
  {
    return 0;
  }
  bBMP = (binaryBitmap*) gmalloc(sizeof(binaryBitmap));

  bBMP->width = width;
  bBMP->height = height;
  bBMP->size = size;
  bBMP->data = (char*) gmalloc(size * sizeof(char));

  return bBMP;
}

void disposeBinaryBitmap(binaryBitmap *bBMP)
{
  free(bBMP->data);
  free(bBMP);
}

long getFilesize(FILE *fp)
{
  long currentPosition;
  long filesize;

  currentPosition = ftell(fp);
  fseek(fp, 0, SEEK_END);
  filesize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  fseek(fp, currentPosition, SEEK_CUR);
  return filesize;
}

unsigned int get2bytes(FILE *fp)
{
  unsigned int value;

  value = fgetc(fp);
  value += (fgetc(fp) << 8);
  return value;
}

unsigned int get4bytes(FILE *fp)
{
  unsigned int value;

  value = fgetc(fp);
  value += (fgetc(fp) << 8);
  value += (fgetc(fp) << 16);
  value += (fgetc(fp) << 24);
  return value;
}

void put2bytes(unsigned int value, FILE *fp)
{
  fputc(value & 0x00FF, fp);
  fputc((value >> 8) & 0x00FF, fp);
}

void put4bytes(unsigned int value, FILE *fp)
{
  fputc(value & 0x00FF, fp);
  fputc((value >> 8) & 0x00FF, fp);
  fputc((value >> 16) & 0x00FF, fp);
  fputc((value >> 24) & 0x00FF, fp);
}
