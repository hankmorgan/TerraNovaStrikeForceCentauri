#include "gui.h"

#ifndef RES_H
#define RES_H

#include <windows.h>
#include <stdio.h>			/* sprintf() */
#include <math.h>			/* floor() */

#define CHUNKDIR	0x7C
#define MAXCHUNKS	838		/* sshock savgam##.dat */
#define MAXBLOCKS	1706	/* sshock objart.res[0] */

typedef struct
{
	char				file[MAX_PATH];
	unsigned short int	chunks;
	unsigned short int	blocks[MAXCHUNKS];
	unsigned short int	id[MAXCHUNKS];
	unsigned int		unpack[MAXCHUNKS];
	unsigned char		type[MAXCHUNKS];
	unsigned int		packed[MAXCHUNKS];
	unsigned char		content[MAXCHUNKS];
	unsigned int		offset[MAXCHUNKS];
	short int			currchunk;
	short int			currblock;
} RESFILE;

typedef struct
{
	unsigned char *unpack, *packed, *hex, *txt;
	unsigned int size, fontheight;
} RESDATA;

void readchunkdir(HWND hwnd, RESFILE *res);
void unpackdata(unsigned char *pack, unsigned char *unpack, long unpacksize);
void readChunkOrBlock(RESFILE *res, RESDATA *data);

#endif
