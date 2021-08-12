// LGRes.h - A helper library in C for manipulating Looking Glass Technologies resource files
// Copyright 2008 by Gigaquad - Thanks to TSSHP dev team

#include <stdio.h>
#include <stdlib.h>					// exit()
#include <string.h>					// strncmp()
#include <math.h>					// pow()
#include <ctype.h>					// tolower()
#include "gfx.c"					// Image loading/saving

#define MAX_CHUNKS 768				// Room for MAX_CHUNKS in a resfile
#define MAX_BLOCKS 768				// Room for MAX_BLOCKS in a subdir

typedef struct
{
	char file[255];
	long int chunkdirOffset;
	int chunks;
	short int id[MAX_CHUNKS];
	int lengthUnpacked[MAX_CHUNKS];
	unsigned char type[MAX_CHUNKS];
	int lengthPacked[MAX_CHUNKS];
	unsigned char content[MAX_CHUNKS];
} chunkdir;

typedef struct
{
	char file[255];
	int chunk;
	long int chunkOffset;
	int type;
	int blocks;
	int offset[MAX_BLOCKS];			// Offset of blocks 1...n. Offset[n+1] = combined size of blocks
	int size[MAX_BLOCKS];
} blockheader;

long int rEnd(unsigned char bytes[], int n);
	// (Read Endian): Inverts endian of the n first elements in bytes[], returns their sum
unsigned char wEnd(long int input, int Nth);
	// (Write Endian): Inverts endian of input, returns Nth (0...3) byte of result
void readChunkdir(char *file, chunkdir *chd);
	// Open valid LG res file, save info in chunkdir into struct chunkdir
long int chunkAdr(chunkdir *chd, int chunk);
	// Returns offset to beginning of chunk
void readBlockheader(blockheader *blh, chunkdir *chd, int chunk);
	// Read blockheader of subdir, save info into struct blockheader
void unpack(unsigned char *pack, unsigned char *unpack, unsigned long unpacksize);
	// Reads unpacksize bytes from pack[], unpacks them to unpack[]
void extractChunk(chunkdir *chd, int chunk);
	// Extracts chunk # to chunk_[chunk#].dat
void extractBlock(blockheader *blh, int block);
	// Extracts block # to block_[chunk#]_[block#].dat
void dumpRes(char *file, char mode[1]);
	// Describes the .res file (chunks, sizes etc). Optional 2nd parameter "f" (full) includes blocks
void loadPal(char *file, int chunk, int offset, unsigned char *pal);
	// The palette located @ offset inside chunk# in file is copied to the
	// unsigned char[256][3] array that the pointer pal is pointed to (eg. the one in every image struct).
void extractBMP(chunkdir *chd, int chunk, int block, image *img);
	// Extracts BMP located in chunk #, block # from file that *chd points to, into the image struct *img.
	// Read chunkdir of res file and load correct palette first, then call function (+all preceding frames if animation)
void makeBMPs(char *prefix, int x, int y);
	// Creates System Shock bitmaps from palettized 8-bit images prefix[1...last].raw
void makeSubdir(char *prefix);
	// Creates a subdir consisting of files prefix[1...last].dat