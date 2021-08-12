// LGRes.c - A helper library in C for manipulating Looking Glass Technologies resource files
// Copyright 2007 by Gigaquad - Thanks to TSSHP dev team

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct chunkProps
{
	int id;
	int lengthUnpacked;
	int type;
	int lengthPacked;
	int content;
};

unsigned char pal[3][256];			// Palette, R,G,B * 256

int readBytes(char *file, int offset, int n);
	// Reads n bytes from file starting at offset, inverts endian, returns them
unsigned char endian(long int input, int Nth);
	// Inverts endian of input and returns Nth byte (0...3)
int chunkdirOffset(char *file);
	// Returns offset to chunk directory
int chunks(char *file);
	// Returns number of chunks in file
void chunkProperties(char *file, int chunk, struct chunkProps *p);
	// Gives properties of chunk via pointer (see struct chunkProps for variables)
int chunkOffset(char *file, int chunk);
	// Returns offset to beginning of chunk
int blocks(char *file, int chunk);
	// Returns number of blocks in subdir
int blockOffset(char *file, int chunk, int block);
	// Returns offset to beginning of block
int blockSize(char *file, int chunk, int block);
	// Returns size of block
void extractChunk(char *file, int chunk);
	// Extracts unpacked chunk to file.dat
void extractBlock(char *file, int chunk, int block);
	// Extracts unpacked block to file.dat.
void printProps(char *file);
	// Prints ID, type and size of chunks and blocks
void loadPal(char *file, int chunk, int offset);
	// Loads palette in (file, chunk) starting at offset into global unsigned char pal[256][3]
void makeBMPs(char *prefix, int x, int y);
	// Creates System Shock bitmaps from palettized 8-bit images prefix[0...last].raw
void makeSubdir(char *prefix);
	// Creates a subdir consisting of files prefix[0...last].dat