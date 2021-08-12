// LGRes.c - A helper library in C for manipulating Looking Glass Technologies resource files
// Copyright 2007 by Gigaquad - Thanks to TSSHP dev team

#include "lgres.h"

int readBytes(char *file, int offset, int n)
{
	unsigned char byte;
	int sum = 0;

	FILE *resFile = fopen(file, "rb");
	fseek(resFile, offset, SEEK_SET);			// Open file, go to offset

	for(int i=0; i<n; i++)						// Read n bytes...
	{
		fread(&byte, 1, 1, resFile);
		sum = sum + pow(256, i) * byte;			// ...and invert the endian
	}

	fclose(resFile);
	return(sum);
}

unsigned char endian(long int input, int Nth)
{
	unsigned char byte[4];
	byte[0] = (input & 0x000000FF) >> 0;		// Invert endian of all bytes
	byte[1] = (input & 0x0000FF00) >> 8;
	byte[2] = (input & 0x00FF0000) >> 16;
	byte[3] = (input & 0xFF000000) >> 24;
	return(byte[Nth]);							// Return one of them
}

int chunkdirOffset(char *file)
{
	FILE *resFile = fopen(file, "rb");
	if(resFile == NULL)
	{
		printf("\n%s was not found.", file);
		exit(-1);
	}

	char testHeader[15];
	fgets(testHeader, 15, resFile);							// Read first bytes in file...
	fclose(resFile);
	if(strncmp(testHeader, "LG Res File v2", 15) != 0)		// ...hopefully matching header
	{
		printf("\n%s is not a resource file.", file);
		exit(-1);
	}

	return(readBytes(file, 0x7C, 4));						// Chunk dir offset is stored at 0x7Ch
}

int chunks(char *file)
{
	return(readBytes(file, chunkdirOffset(file), 2));		// 2 first bytes in chunk dir = amount of chunks
}

void chunkProperties(char *file, int chunk, struct chunkProps *p)
{
	if ((chunk < 1) || (chunk > chunks(file)))
	{
		printf("\nChunk %d does not exist in %s (%d is max).", chunk, file, chunks(file));
		exit(-1);
	}

	int offset = chunkdirOffset(file)+6+10*chunk-10;		// 6. byte in chunk header + 10 bytes/chunk
	p->id				= readBytes(file, offset+0, 2);
	p->lengthUnpacked	= readBytes(file, offset+2, 3);
	p->type				= readBytes(file, offset+5, 1);
	p->lengthPacked		= readBytes(file, offset+6, 3);
	p->content			= readBytes(file, offset+9, 1);
}

int chunkOffset(char *file, int chunk)
{
	struct chunkProps c;
	int offset = readBytes(file, chunkdirOffset(file)+2, 4);	// First chunk starts here

	for(int i=1; i<chunk; i++)
	{
		chunkProperties(file, i, &c);
		offset = offset + c.lengthUnpacked;
		if(offset % 4 != 0)										// If offset to chunk beginning isn't divisible by 4...
			offset = 4*(floor(offset/4)+1);						// ...increase offset until it is
	}

	return(offset);
}

int blocks(char *file, int chunk)
{
	struct chunkProps c;
	chunkProperties(file, chunk, &c);
	if (c.type <= 1)									// If chunk is actually a subdir...
	{
		printf("\nChunk %d is not a subdir.", chunk);
		exit(-1);
	}

	return(readBytes(file, chunkOffset(file, chunk), 2));	// ...2 first bytes = amount of blocks in it
}

int blockOffset(char *file, int chunk, int block)
{
	int offset;

	if((block < 1) || (block > (blocks(file, chunk))))
	{
		printf("\nBlock %d does not exist in subdir.", block);
		exit(-1);
	}

	offset = chunkOffset(file, chunk)+2+(4*block)-4;				// 2 bytes (amount) + 4 bytes/block
	return(chunkOffset(file, chunk)+readBytes(file, offset, 4));
}

int blockSize(char *file, int chunk, int block)
{
	int size;

	if (block == blocks(file, chunk))								// If last block in subdir
	{
		struct chunkProps c;
		chunkProperties(file, chunk, &c);
		size = c.lengthUnpacked-blockOffset(file, chunk, block);
	}
	else
		size = blockOffset(file, chunk, block+1) - blockOffset(file, chunk, block);

	return(size);
}

void extractChunk(char *file, int chunk)
{
	unsigned char byte;
	struct chunkProps c;
	chunkProperties(file, chunk, &c);

	FILE *fileOut = fopen("file.dat", "wb");
	FILE *resFile = fopen(file, "rb");
	fseek(resFile, chunkOffset(file, chunk), SEEK_SET);

	for(int i=0; i<c.lengthUnpacked; i++)				// Packed chunks (rare) are NOT extracted properly
	{
		fread(&byte, 1, 1, resFile);
		fputc(byte, fileOut);
	}

	fclose(fileOut);
	fclose(resFile);
}

void extractBlock(char *file, int chunk, int block)
{
	unsigned char byte;
	int end = blockSize(file, chunk, block);

	FILE *fileOut = fopen("file.dat", "wb");
	FILE *resFile = fopen(file, "rb");
	fseek(resFile, blockOffset(file, chunk, block), SEEK_SET);

	for(int i=0; i<end; i++)							// Packed blocks (rare) are NOT extracted properly
	{
		fread(&byte, 1, 1, resFile);
		fputc(byte, fileOut);
	}

	fclose(fileOut);
	fclose(resFile);
}

void printProps(char *file)
{
	int i,j;
	struct chunkProps c;

	printf("%s", file);
	printf("\nChu#\tID\tType\tSize", file);
	printf("\n  Blo#\t\t\t  Size\n", file);
	for(i=1; i<=chunks(file); i++)							// Going through all chunks is very slow
	{
		chunkProperties(file, i, &c);
		printf("\n%d:\t%d\t%d\t%d", i, c.id, c.content, c.lengthUnpacked);

		if (c.type >= 2)									// If subdir, print block info
			for(j=1; j<=blocks(file, i); j++)
				printf("\n  %d:\t\t\t  %d", j, blockSize(file, i, j));
	}
}

void loadPal(char *file, int chunk, int offset)
{
	FILE *resFile = fopen(file, "rb");							// Open file...
	fseek(resFile, chunkOffset(file, chunk)+offset, SEEK_SET);	// ... where palette begins
	fread(&pal, 3, 256, resFile);								// read R,G,B 256 times, store in global array pal
	fclose(resFile);
}

void makeBMPs(char *prefix, int x, int y)
{
	unsigned char buf[256];
	char *file = "";
	int blocks, i=0;
	FILE *in;
	FILE *out;

	// Store amount of input files and check their size
	do
	{
		sprintf(file, "%s%d%s", prefix, i, ".raw");	// Blocks must be named prefix0.raw ... prefixn.raw

		in = fopen(file, "rb");
		if (in == NULL)								// Stop looping when reached last file
		{
			blocks = i;								// Store amount of blocks
			i = -1;
			break;
		}

		fseek(in, 0, SEEK_END);					//check if filesize matches dimensions
		if (ftell(in) != x * y)
		{
			printf("\nFile %d has different size than dimensions.", i);
			exit(-1);
		}

		fclose(in);
		i++;
	}
	while(i != -1);

	for(i=0; i<blocks; i++)								// Go through all block files
	{
		sprintf(file, "%s%d%s", prefix, i, ".raw");		// Blocks must be named prefix0.raw ... prefixn.raw
		in = fopen(file, "rb");
		sprintf(file, "%s%d%s", prefix, i, ".dat");		// Files will be named prefix0.dat ... prefixn.dat
		out = fopen(file, "wb");

		// "Compressed" bitmap header:
		fprintf(out, "%c%c%c%c", 0, 0, 0, 0);						// Always 0
		fprintf(out, "%c%c%c%c", 4, 0, 0, 0);						// Compression
		fprintf(out, "%c%c", endian(x, 0), endian(x, 1));			// Width
		fprintf(out, "%c%c", endian(y, 0), endian(y, 1));			// Height
		fprintf(out, "%c%c", endian(x, 0), endian(x, 1));			// Always same as width
		fprintf(out, "%c", (int)floor(log(x)/log(2)));				// Log2 width
		fprintf(out, "%c", (int)floor(log(y)/log(2)));				// Log2 height
		fprintf(out, "%c%c%c%c", 0, 0, 0, 0);						// Animation frames
		fprintf(out, "%c%c", endian(x, 0), endian(x, 1));			// Width
		fprintf(out, "%c%c", endian(y, 0), endian(y, 1));			// Height
		fprintf(out, "%c%c%c%c", 0, 0, 0, 0);						// Always 0

		// Image data:
		for(int j=0; j<(int) floor(x * y / 255); j++)					// Image data in 255 byte chunks
		{
			fprintf(out, "%c%c%c", 0x80, 0xFF, 0x80);
			fread(&buf, 255, 1, in);
			fwrite(&buf, 255, 1, out);
		}
		fprintf(out, "%c", 0x80);									// Remainder of image data
		fprintf(out, "%c", (int) (x * y -floor((x * y) / 255) * 255));
		fprintf(out, "%c", 0x80);
		fread(&buf, (int) (x * y -floor((x * y) / 255) * 255), 1, in);
		fwrite(&buf, (int) (x * y -floor((x * y) / 255) * 255), 1, out);
		fprintf(out, "%c%c%c", 0x80, 0x00, 0x00);					// End of image data
		fclose(in);
		fclose(out);
	}
}

void makeSubdir(char *prefix)
{
	int blocks, offset, i=0;
	int blockSize[128];						// Room for 128 blocks in subdir
	char *file = "";
	unsigned char byte;
	FILE *in;
	FILE *out = fopen("subdir.dat", "wb");

	// Store amount of input files and their size
	do
	{
		sprintf(file, "%s%d%s", prefix, i, ".dat");	// Files must be named prefix0.dat ... prefixn.dat
		in = fopen(file, "rb");

		if (in == NULL)								// Stop looping when reached last file
		{
			blocks = i;								// Store amount of blocks
			i = -1;
			break;
		}
		fseek(in, 0, SEEK_END);
		blockSize[i] = ftell(in);					// Store block sizes in array
		fclose(in);
		i++;
	}
	while(i != -1);

	// Write chunkdir header
	fprintf(out, "%c%c", endian(blocks, 0), endian(blocks, 1));		// Blocks in subdir
	offset = 2 + 4 * blocks + 4;									// Size of header
	for(i=0; i<=blocks; i++)
	{
		fprintf(out, "%c%c%c%c", endian(offset, 0), endian(offset, 1), endian(offset, 2), endian(offset, 3));
		offset = offset + blockSize[i];					// Block offset = previous offset + size of previous block
	}

	// Merge blocks into subdir
	for(i=0; i<blocks; i++)								// Go through all block files
	{
		sprintf(file, "%s%d%s", prefix, i, ".dat");
		in = fopen(file, "rb");

		for(int j=0; j<blockSize[i]; j++)
		{
			fread(&byte, 1, 1, in);						// Read 1 byte of block
			fputc(byte, out);							// Save it in subdir
		}
		printf("\nBlock %d done, %d bytes", i+1, blockSize[i]);

		fclose(in);
	}
	fclose(out);
}