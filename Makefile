
CFLAGS = -g
SRC = bmp.c fft.c gfunctions.c hopfield.c transf.c kohonen.c nnet.c
OBJ = $(addsuffix .o, $(basename $(SRC)))

all:  wnnet

wnnet.o: wnnet.cpp
	gcc $(CFLAGS) -c `wx-config --cflags` wnnet.cpp

wnnet: $(OBJ) wnnet.o
	gcc -o wnnet $(OBJ) wnnet.o `wx-config --libs` -lm -static

nnet:  $(OBJ)
	gcc -o nnet $(OBJ) -lm


srcdist: 
	zip wnnet_src.zip Makefile makefile.vc $(SRC) wnnet.cpp wnnetdia.wxr wnnet.res wnnet.wxr *.h wnnet.rc
