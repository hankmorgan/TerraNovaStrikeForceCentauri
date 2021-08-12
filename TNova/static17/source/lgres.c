// LGRes.c - A helper library in C for manipulating Looking Glass Technologies resource files
// Copyright 2008 by Gigaquad - Thanks to TSSHP dev team

#include "lgres.h"

long int rEnd(unsigned char bytes[], int n)
{
	int i;
	long int sum=0;
	for(i=0; i<n; i++)
		sum = sum + pow(256, i) * bytes[i];		// Invert endian
	return(sum);
}

unsigned char wEnd(long int input, int Nth)
{
	unsigned char byte[4];
	byte[0] = (input & 0x000000FF) >> 0;		// Invert endian of input
	byte[1] = (input & 0x0000FF00) >> 8;
	byte[2] = (input & 0x00FF0000) >> 16;
	byte[3] = (input & 0xFF000000) >> 24;
	return(byte[Nth]);							// Return Nth byte of result
}

void readChunkdir(char *file, chunkdir *chd)
{
	char magic[15];
	unsigned char bytes[4];
	int i;
	FILE *in = fopen(file, "rb");

	if(in == NULL)										// Error opening file, exit
	{
		fprintf(stderr, "\n%s was not found", file);
		exit(-1);
	}
	fgets(magic, 15, in);								// Read magic numbers in file
	if(strncmp(magic, "LG Res File v2", 15) != 0)		// Exit if not LG res file
	{
		printf("\n%s is not a LG resource file", file);
		exit(-1);
	}

	strcpy(chd->file, file);							// Save filename inside struct
	fseek(in, 0x7C, SEEK_SET);							// Chunkdir offset stored @ 0x7C
	fread(&bytes, 1, 4, in);							// Read the offset (4 bytes)
	chd->chunkdirOffset = rEnd(bytes, 4);
	fseek(in, chd->chunkdirOffset, SEEK_SET);			// Then go to chunkdir

	fread(&bytes, 1, 2, in);							// Read amount of chunks
	chd->chunks = rEnd(bytes, 2);
	if (chd->chunks > MAX_CHUNKS)						// Exit if MAX_CHUNKS isn't large enough
	{
		printf("\nFile has %d chunks, more than MAX_CHUNKS allocates for", chd->chunks);
		exit(-1);
	}

	fseek(in, 4, SEEK_CUR);								// Skip chunk1 offset, it's always 0x80
	for(i=1; i<=chd->chunks; i++)						// Read id, length(s), type, content of each chunk
	{													// Note: array starts at 1, so chunk(n) = array[n]
		fread(&bytes, 1, 2, in);
			chd->id[i] = rEnd(bytes, 2);
		fread(&bytes, 1, 3, in);
			chd->lengthUnpacked[i] = rEnd(bytes, 3);
		fread(&chd->type[i], 1, 1, in);
		fread(&bytes, 1, 3, in);
			chd->lengthPacked[i] = rEnd(bytes, 3);
		fread(&chd->content[i], 1, 1, in);
	}

	chd->id[0] = 1;										// Need some data in 0th entries
	chd->lengthUnpacked[0] = 1;							// for error handling to work
	chd->type[0] = 1;
	chd->lengthPacked[0] = 1;
	chd->content[0] = 1;

	fclose(in);
}

long int chunkAdr(chunkdir *chd, int chunk)
{
	int i;
	long int offset = 0x80;								// First chunk starts @ 0x80

	if ((chunk < 1) || (chunk > chd->chunks))			// Only valid chunks handled
	{
		printf("\nChunk %d does not exist (%d is max)", chunk, chd->chunks);
		exit(-1);
	}

	for(i=1; i<chunk; i++)
	{
		offset = offset + chd->lengthPacked[i];			// Offset(n) = 0x80 + size of chunks 1...n-1
		if(offset % 4 != 0)								// If offset to chunk start isn't divisible by 4...
			offset = 4 * (floor(offset / 4) + 1);		// ...increase offset until it is
	}

	return(offset);
}

void readBlockheader(blockheader *blh, chunkdir *chd, int chunk)
{
	int i;
	unsigned char bytes[4];
	FILE *in;

	strcpy(blh->file, chd->file);
	blh->chunk = chunk;							// Copy values from parent chunk
	blh->chunkOffset = chunkAdr(chd, chunk);
	blh->type = chd->type[chunk];

	if(chd->type[chunk] <= 1)					// Exit if chunk not a subdir
	{
		printf("\nChunk %d in %s isn't a subdir", chunk, blh->file);
		exit(-1);
	}

	in = fopen(blh->file, "rb");
	fseek(in, chunkAdr(chd, chunk), SEEK_SET);
	fread(&bytes, 1, 2, in);					// Read block amount in chunk
	blh->blocks = rEnd(bytes, 2);

	if (blh->blocks > MAX_BLOCKS)				// Exit if MAX_BLOCKS isn't large enough
	{
		printf("\nChunk has %d blocks, more than MAX_BLOCKS allocates for", blh->blocks);
		exit(-1);
	}

	for(i=1; i<=blh->blocks+1; i++)				// Read offset of blocks 1...n, then total size of blocks (+1)
	{
		fread(&bytes, 1, 4, in);
		blh->offset[i] = rEnd(bytes, 4);		// Note: array starts at 1, so block(n) = array[n]
	}
	for(i=1; i<=blh->blocks; i++)				// Calculate size of every block
		blh->size[i] = blh->offset[i+1] - blh->offset[i];

	blh->offset[0] = 1;							// Need some data in 0th entries
	blh->size[0] = 1;							// for error handling to work

	fclose(in);
}

void unpack(unsigned char *pack, unsigned char *unpack, unsigned long unpacksize)
{
	unsigned char *byteptr;
	unsigned char *exptr;
	unsigned long word = 0;
	int offs_token	[16384];
	int len_token	[16384];
	int org_token	[16384];
	int val, i, nbits=0, ntokens = 0;

	for (i = 0; i < 16384; ++i)
	{
		len_token [i] = 1;
		org_token [i] = -1;
	}
	byteptr = pack;
	exptr   = unpack;

	while (exptr - unpack < unpacksize)
	{
		while (nbits < 14)
		{
			word = (word << 8) + *byteptr++;
			nbits = nbits + 8;
		}
		nbits = nbits - 14;
		val = (word >> nbits) & 0x3FFF;

		if (val == 0x3FFF)
			break;

		if (val == 0x3FFE)
		{
			for (i = 0; i < 16384; ++i)
			{
				len_token [i] = 1;
				org_token [i] = -1;
			}
			ntokens = 0;
			continue;
		}

		if (ntokens < 16384)
		{
			offs_token [ntokens] = exptr - unpack;
			if (val >= 0x100)
				org_token [ntokens] = val - 0x100;
		}
		++ntokens;

		if (val < 0x100)
			*exptr++ = val;
		else
		{
			val = val - 0x100;
			if (len_token [val] == 1)
			{
				if (org_token [val] != -1)
					len_token [val] += len_token [org_token [val]];
				else
					len_token [val] += 1;
			}
			for (i = 0; i < len_token [val]; ++i)
				*exptr++ = unpack [i + offs_token [val]];
		}
	}
}

void extractChunk(chunkdir *chd, int chunk)
{
	unsigned char rawChunk[chd->lengthPacked[chunk]];					// Array for chunk data
	unsigned char unpackedChunk[chd->lengthUnpacked[chunk]];			// Array for unpacked chunk
	blockheader blh;
	char fileout[15] = "chunk_";
	FILE *in;
	FILE *out;

	sprintf(fileout, "%s%d%s", fileout, chunk, ".dat");					// chunk_[chunk#].dat
	in = fopen(chd->file, "rb");
	out = fopen(fileout, "wb");
	fseek(in, chunkAdr(chd, chunk), SEEK_SET);							// Go to chunk offset
	fread(&rawChunk, chd->lengthPacked[chunk], 1, in);					// Copy chunk into array

	switch(chd->type[chunk])
	{
		case 0:	// Unpacked chunk										// Do nothing, write to file as is
		case 2:	// Unpacked subdir
			fwrite(&rawChunk, chd->lengthUnpacked[chunk], 1, out);
			break;

		case 1:	// Packed chunk											// Unpack, then write data to file
			unpack(rawChunk, unpackedChunk, chd->lengthUnpacked[chunk]);
			fwrite(&unpackedChunk, chd->lengthUnpacked[chunk], 1, out);
			break;

		case 3:	// Packed subdir
			readBlockheader(&blh, chd, chunk);							// Find out subdir header size
			fwrite(&rawChunk, blh.offset[1], 1, out);					// Write subdir header to file
			unpack(rawChunk+blh.offset[1], unpackedChunk, chd->lengthUnpacked[chunk]);	// Unpack starting after header
			fwrite(&unpackedChunk, chd->lengthUnpacked[chunk]-blh.offset[1], 1, out);	// Write data to file
			break;
	}

	if ((chd->type[chunk] == 2) || (chd->type[chunk] == 3))
		printf("\nFYI: chunk %d is actually a subdir", chunk);
	fclose(in);
	fclose(out);
}

void extractBlock(blockheader *blh, int block)
{
	unsigned char rawBlock[blh->size[block]];			// Array for block data
	unsigned char unpackedBlock[blh->size[block]];		// Array for unpacked block
	char fileout[15] = "block_";
	FILE *in;
	FILE *out;

	if ((block < 1) || (block > blh->blocks))								// Invalid block
		printf("\nBlock %d does not exist in subdir (%d is max)", block, blh->blocks);

	else
	{
		sprintf(fileout, "%s%d_%d%s", fileout, blh->chunk, block, ".dat");	// block_[chunk#]_[block#].dat
		in = fopen(blh->file, "rb");
		out = fopen(fileout, "wb");
		fseek(in, blh->chunkOffset+blh->offset[block], SEEK_SET);			// Go to block offset
		fread(&rawBlock, blh->size[block], 1, in);							// Read block data

		if (blh->type == 2)		// Unpacked subdir
			fwrite(&rawBlock, blh->size[block], 1, out);					// Write block to file as it is
		if (blh->type == 3)		// Packed subdir
		{
			unpack(rawBlock, unpackedBlock, blh->size[block]);				// Unpack, then write block
			fwrite(&unpackedBlock, blh->size[block], 1, out);
		}

		fclose(in);
		fclose(out);
	}
}

void dumpRes(char *file, char mode[1])
{
	int i, j;
	chunkdir chd;
	blockheader blh;

	if (file == NULL)			// Fixing lack of arguments
		exit(-1);
	if (mode == NULL)
		mode = "0";

	readChunkdir(file, &chd);
	printf("\n%s (%d chunks)\nChunk directory @\t0x%X", file, chd.chunks, chd.chunkdirOffset);
	printf("\n\nChu#\tSize\t\tOffset\t\tID\tContent", file);
	printf("\n   Blo#\t   Size\t\t   Offset\n", file);

	for(i=1; i<=chd.chunks; i++)
	{
		printf("\n%d:\t%d\t\t0x%X    \t%d\t%d", i, chd.lengthUnpacked[i], chunkAdr(&chd, i), chd.id[i], chd.content[i]);
		if (chd.lengthUnpacked[i] != chd.lengthPacked[i])						// Compressed chunk
			printf("\tPACKED");

		if ((chd.type[i] >= 2) && (tolower(mode[0]) == 'f'))					// If subdir, print block info
		{
			readBlockheader(&blh, &chd, i);
			for(j=1; j<=blh.blocks; j++)
				printf("\n   %d:\t   %d\t\t   0x%X", j, blh.size[j], blh.offset[j]);
		}
	}
}

void loadPal(char *file, int chunk, int offset, unsigned char *pal)
{
	FILE *in;
	chunkdir chd;

	readChunkdir(file, &chd);
	in = fopen(chd.file, "rb");								// Open file...
	fseek(in, chunkAdr(&chd, chunk)+offset, SEEK_SET);		// ... where palette begins
	fread(pal, 3, 256, in);									// Read palette into array, not pointer
	fclose(in);
}

void extractBMP(chunkdir *chd, int chunk, int block, image *img)
{
	int i, compression, pixels=0;
	unsigned char byte[4];
	blockheader blh;
	FILE *in;

	readBlockheader(&blh, chd, chunk);
	if ((block < 1) || (block > blh.blocks))
	{
		printf("\nBlock %d does not exist in subdir (%d is max)", block, blh.blocks);
		exit(-1);
	}

	in = fopen(chd->file, "rb");
	fseek(in, blh.chunkOffset+blh.offset[block]+4, SEEK_SET);	// Go to BMP header, skip first 4 unknown bytes
	fread(&byte, 1, 2, in);										// Read compression type
		compression = rEnd(byte, 2);
	fseek(in, 2, SEEK_CUR);										// Skip next 2 unknown bytes
	fread(&byte, 1, 2, in);										// Read image width
		img->x = rEnd(byte, 2);
	fread(&byte, 1, 2, in);										// Read image height
		img->y = rEnd(byte, 2);
	fseek(in, 16, SEEK_CUR);									// Skip rest of header, not needed

	if(compression != 4)						// No compression:
		fread(&img->pix, img->x, img->y, in);					// Read x*y pixels, save bitmap in image struct

	else										// Compressed BMP:
	{
		while(pixels < img->x * img->y - 1)						// Keep uncompressing until x*y pixels
		{
			fread(&byte[0], 1, 1, in);

			if  (byte[0] == 0)									// 00 nn mm			write mm nn times
			{
				fread(&byte[1], 1, 1, in);
				fread(&byte[2], 1, 1, in);
				memset(&img->pix[pixels], byte[2], byte[1]);
				pixels = pixels + byte[1];
			}

			if(byte[0] < 0x80)									// mm<80			write next mm bytes
			{
				fread(&img->pix[pixels], 1, byte[0], in);
				pixels = pixels + byte[0];
			}

			if(byte[0] > 0x80)									// mm>80			skip next mm bytes
				pixels = pixels + (byte[0] & 0x7F);

			if(byte[0] == 0x80)
			{
				fread(&byte[1], 1, 1, in);
				fread(&byte[2], 1, 1, in);

				if((byte[1] == 0x00) && (byte[2] == 0x00))		// 80 00 00			end of bitmap
					break;

				if(byte[2] < 0x80)								// 80 mm nn<80		skip next nn*256+mm bytes
					pixels = pixels + byte[2] * 256 + byte[1];

				if(byte[2] == 0x80)								// 80 nn 80			write next nn bytes
				{
					fread(&img->pix[pixels], 1, byte[1], in);
					pixels = pixels + byte[1];
				}

				if((byte[2] > 0x80) && (byte[2] < 0xC0))		// 80 mm 80<nn<C0	write next (nn&0x3F)*256+mm bytes
				{
					fread(&img->pix[pixels], 1, (byte[2] & 0x3F) * 256 + byte[1], in);
					pixels = pixels + (byte[2] & 0x3F) * 256 + byte[1];
				}

				if(byte[2] > 0xC0)								// 80 mm nn<C0 xx	write xx (nn&0x3F)*256+mm times
				{
					fread(&byte[3], 1, 1, in);
					memset(&img->pix[pixels], byte[3], (byte[2] & 0x3F) * 256 + byte[1]);
					pixels = pixels + (byte[2] & 0x3F) * 256 + byte[1];
				}
			}
		}
	}

	fclose(in);

	for(i=0; i<strlen(chd->file); i++)							// Image name becomes: resfilename_[chunk#]_[block#]
		if(chd->file[i] == '.')
			img->name[i] = '\0';
		else
			img->name[i] = chd->file[i];
	sprintf(img->name, "%s_%d_%d", img->name, chunk, block);
}

void makeBMPs(char *prefix, int x, int y)
{
	unsigned char buf[256];
	char *file = "";
	int blocks, i=1;
	FILE *in;
	FILE *out;

	do	// Store amount of input files and check their size
	{
		sprintf(file, "%s%d%s", prefix, i, ".raw");		// Blocks must be named prefix1.raw ... prefixn.raw
		in = fopen(file, "rb");
		if (in == NULL)									// Stop looping when reached last file
		{
			blocks = i;									// Store amount of blocks
			i = -1;
			break;
		}

		fseek(in, 0, SEEK_END);							//check if filesize matches dimensions
		if (ftell(in) != x * y)
		{
			printf("\nFile %d has different size than dimensions.", i);
			exit(-1);
		}

		fclose(in);
		i++;
	}
	while(i != -1);

	for(i=1; i<blocks; i++)								// Go through all block files
	{
		sprintf(file, "%s%d%s", prefix, i, ".raw");		// Blocks must be named prefix1.raw ... prefixn.raw
		in = fopen(file, "rb");
		sprintf(file, "%s%d%s", prefix, i, ".dat");		// Files will be named prefix1.dat ... prefixn.dat
		out = fopen(file, "wb");

		// "Compressed" bitmap header:
		fprintf(out, "%c%c%c%c", 0, 0, 0, 0);			// ??? Always 0
		fprintf(out, "%c%c", 4, 0);						// Compression
		fprintf(out, "%c%c", 0, 0);						// ???
		fprintf(out, "%c%c", wEnd(x, 0), wEnd(x, 1));	// Width
		fprintf(out, "%c%c", wEnd(y, 0), wEnd(y, 1));	// Height
		fprintf(out, "%c%c", wEnd(x, 0), wEnd(x, 1));	// Width
		fprintf(out, "%c", (int)floor(log(x)/log(2)));	// Log2 width
		fprintf(out, "%c", (int)floor(log(y)/log(2)));	// Log2 height
		fprintf(out, "%c%c%c%c", 0, 0, 0, 0);			// Animation frames (next lines are for vidmails, not BMPs)
		fprintf(out, "%c%c", wEnd(x, 0), wEnd(x, 1));	// Width
		fprintf(out, "%c%c", wEnd(y, 0), wEnd(y, 1));	// Height
		fprintf(out, "%c%c%c%c", 0, 0, 0, 0);			// Always 0

		// Image data:
		for(int j=0; j<(int) floor(x * y / 255); j++)						// Image data in 255 byte chunks
		{
			fprintf(out, "%c%c%c", 0x80, 0xFF, 0x80);
			fread(&buf, 255, 1, in);
			fwrite(&buf, 255, 1, out);
		}
		fprintf(out, "%c", 0x80);											// Remainder of image data
		fprintf(out, "%c", (int) (x * y - floor((x * y) / 255) * 255));
		fprintf(out, "%c", 0x80);
		fread(&buf, (int) (x * y - floor((x * y) / 255) * 255), 1, in);
		fwrite(&buf, (int) (x * y - floor((x * y) / 255) * 255), 1, out);
		fprintf(out, "%c%c%c", 0x80, 0x00, 0x00);							// End of image data

		fclose(in);
		fclose(out);
	}
}

void makeSubdir(char *prefix)
{
	int blocks, offset, i=1, j;
	int blockSize[MAX_BLOCKS];							// Room for MAX_BLOCKS in subdir
	char *file = "";
	unsigned char byte;
	FILE *in;
	FILE *out = fopen("subdir.dat", "wb");

	// Store amount of input files and their size
	do
	{
		sprintf(file, "%s%d%s", prefix, i, ".dat");		// Files must be named prefix1.dat ... prefixn.dat
		in = fopen(file, "rb");

		if (in == NULL)									// Stop looping when reached last file
		{
			blocks = i;									// Store amount of blocks
			i = -1;
			break;
		}
		fseek(in, 0, SEEK_END);
		blockSize[i] = ftell(in);						// Store block sizes in array
		fclose(in);
		i++;
	}
	while(i != -1);

	// Write chunkdir header
	fprintf(out, "%c%c", wEnd(blocks, 0), wEnd(blocks, 1));		// Blocks in subdir
	offset = 2 + 4 * blocks + 4;								// Size of header
	for(i=1; i<=blocks+1; i++)
	{
		fprintf(out, "%c%c%c%c", wEnd(offset, 0), wEnd(offset, 1), wEnd(offset, 2), wEnd(offset, 3));
		offset = offset + blockSize[i];							// Block offset = prev offset + prev blocksize
	}

	// Merge blocks into subdir
	for(i=1; i<blocks; i++)										// Go through all block files
	{
		sprintf(file, "%s%d%s", prefix, i, ".dat");
		in = fopen(file, "rb");
		for(j=0; j<blockSize[i]; j++)
		{
			byte = fgetc(in);									// Read 1 byte of block
			fputc(byte, out);									// Save it in subdir
		}
		printf("\nBlock %d done, %d bytes", i, blockSize[i]);
		fclose(in);
	}

	while(ftell(out) % 4 != 0)
		fputc(0x00, out);										// Add zeroes until size is divisible by 4

	fclose(out);
}