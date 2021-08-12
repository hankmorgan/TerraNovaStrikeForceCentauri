/* LGRes.h - C library for Looking Glass Technologies resource files
   Copyright 2009 by Gigaquad - Thanks to TSSHP dev team */

#include <stdio.h>
#include <stdlib.h>						/* exit() */
#include <string.h>						/* strncmp() */
#include <math.h>						/* floor() */

#define MAXCHUNKS 836					/* ARCHIVE.DAT: 834 */
#define MAXBLOCKS 1708					/* OBJART.RES[1]: 1706 */
#define PATHLENGTH 255					/* Length of pathname for files */
#define BMPX 640						/* Width of bitmap */
#define BMPY 480						/* Height of bitmap */

typedef struct
{
	char file[PATHLENGTH];
	unsigned int chunkdiroffset;
	unsigned short int chunks;
	unsigned short int id[MAXCHUNKS];
	unsigned int sizeunpack[MAXCHUNKS];
	unsigned char type[MAXCHUNKS];
	unsigned int sizepack[MAXCHUNKS];
	unsigned char content[MAXCHUNKS];
	unsigned int offset[MAXCHUNKS];
	unsigned short int subdir;
	unsigned short int blocks;
	unsigned int bloffset[MAXBLOCKS];	/* bloffset[chunkdir.blocks+1] = size of all blocks */
	unsigned int blsize[MAXBLOCKS];
} chunkdir;

typedef struct
{
	char file[PATHLENGTH];
	unsigned int x;
	unsigned int y;
	unsigned char pix[BMPX*BMPY];
	unsigned char r[256];
	unsigned char g[256];
	unsigned char b[256];
	unsigned char a[256];
} image;

void readchunkdir(char *file, chunkdir *ch);
/* Open LG res file, save header info to struct. Always call this first */
void readblockheader(chunkdir *ch, unsigned short int chunk);
/* Read header of subdir, save info to struct. Sets chunkdir->subdir to chunk # */
void dumpres(char *file, char mode[1]);
/* Describes the res file (chunks, sizes etc). Optional 2nd parameter "f" (full) includes blocks */
void unpack(unsigned char *pack, unsigned char *unpack, long unpacksize);
/* Reads unpacksize bytes from pack[], unpacks them to unpack[] */
void extractchunk(chunkdir *ch, unsigned short int chunk);
/* Extracts chunk # to chunk_#.dat */
void extractblock(chunkdir *ch, unsigned short int block);
/* Extracts block # to block_#.dat */
void replacechunk(chunkdir *ch, unsigned short int chunk, char *file);
/* Replaces chunk # with contents of file. New chunk preserves ID and type, but is unpacked */
void replaceblock(chunkdir *ch, unsigned short int block, char *file);
/* Replaces block # in active subdir with file, then calls replaceChunk */
void loadpal(chunkdir *ch, unsigned short int chunk, image *img);
/* The palette from chunk # is copied to img->pal */
void extractbmp(chunkdir *ch, unsigned short int block, image *img);
/* Extracts BMP located in active subdir's block # into the image struct *img. */
void importbmp(chunkdir *ch, unsigned short int block, image *img);
/* Image in *img is saved to block# in active subdir */
void savebmp(image *img);
/* Saves image as BMP with the filename defined in img->file */
void loadbmp(image *img, char *file);
/* Loads BMP file into image struct */
