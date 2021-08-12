/* LGRes.c - C library for Looking Glass Technologies resource files
   Copyright 2009 by Gigaquad - Thanks to TSSHP dev team */

#include "lgres.h"

void readchunkdir(char *file, chunkdir *ch)
{
	char magic[15];
	unsigned int i, offset;
	FILE *in;

	in = fopen(file, "rb");
	if(in == NULL)
	{
		printf("\n%s not found", file);
		exit(-1);
	}
	fgets(magic, 15, in);
	if(strncmp(magic, "LG Res File v2", 15) != 0)
	{
		printf("\n%s isn't a LG .res file", file);
		exit(-1);
	}

	strcpy(ch->file, file);							/* Save filename inside struct */
	fseek(in, 0x7C, SEEK_SET);						/* Chunkdir offset is @ 0x7C */
	fread(&ch->chunkdiroffset, 4, 1, in);
	fseek(in, ch->chunkdiroffset, SEEK_SET);
	fread(&ch->chunks, 2, 1, in);
	if(ch->chunks > MAXCHUNKS)
	{
		printf("\n%s has too many (%d) chunks", file, ch->chunks);
		exit(-1);
	}
	fread(&offset, 4, 1, in);						/* First chunk offset */

	for(i=1; i<=ch->chunks; i++)					/* Read id, size, type, content of each chunk */
	{												/* Array starts at 1, chunk(n) = array[n] */
		fread(&ch->id[i], 2, 1, in);
		fread(&ch->sizeunpack[i], 3, 1, in);		/* Size is 3 bytes, held in 4 byte variable... */
		ch->sizeunpack[i] &= 0x00FFFFFF;			/* ...AND with 0x00FFFFFF removes excess */
		fread(&ch->type[i], 1, 1, in);
		fread(&ch->sizepack[i], 3, 1, in);
		ch->sizepack[i] &= 0x00FFFFFF;
		fread(&ch->content[i], 1, 1, in);

		ch->offset[i] = offset;
		offset = offset + ch->sizepack[i];			/* Offset(n) = 0x80 + size of chunks 1...n-1 */
		if(offset % 4 != 0)							/* Offset must be even 4 bytes */
			offset = 4 * (floor(offset / 4) + 1);
	}
	ch->subdir = ch->blocks = 0;					/* We have not called readBlockheader yet */

	fclose(in);
}

void readblockheader(chunkdir *ch, unsigned short int chunk)
{
	unsigned short int i;
	FILE *in;

	if((chunk < 1) || (chunk > ch->chunks))
	{
		printf("\nChunk %d/%d doesn't exist in %s", chunk, ch->chunks, ch->file);
		exit(-1);
	}
	if(ch->type[chunk] <= 1)
	{
		printf("\nChunk %d in %s isn't a subdir", chunk, ch->file);
		exit(-1);
	}

	ch->subdir = chunk;
	in = fopen(ch->file, "rb");
	fseek(in, ch->offset[chunk], SEEK_SET);
	fread(&ch->blocks, 2, 1, in);
	if(ch->blocks > MAXBLOCKS)
	{
		printf("\n%s has too many (%d) blocks", ch->file, ch->blocks);
		exit(-1);
	}
	fread(&ch->bloffset[1], 4, 1, in);							/* Read 1st offset separately (subtraction below) */
	for(i=1; i<=ch->blocks; i++)								/* Array starts at 1, block(n) = array[n] */
	{
		fread(&ch->bloffset[i+1], 4, 1, in);					/* Read remaining offsets, including last header entry */
		ch->blsize[i] = ch->bloffset[i+1] - ch->bloffset[i];	/* Block size = next - current offset, which works since */
	}															/* the last entry in header is total size of all blocks */

	fclose(in);
}

void dumpres(char *file, char mode[1])
{
	unsigned int i, j, fsize;
	chunkdir ch;
	FILE *in;

	in = fopen(file, "rb");
	if(in == NULL)
	{
		printf("\n%s not found", file);
		return;
	}
	if(mode == NULL)											/* Optional argument unused */
		mode = "0";

	fseek(in, 0, SEEK_END);
	fsize = ftell(in);
	fclose(in);

	readchunkdir(file, &ch);
	printf("\n%s (%d chunks)\n%d bytes, chunkdir @ %X", file, ch.chunks, fsize, ch.chunkdiroffset);
	printf("\n\nChu#      Size    Offset     ID    Cont   Blocks");
	printf("\n%43s Blo#  Size @ Offset\n", "");

	for(i=1; i<=ch.chunks; i++)
	{
		printf("\n%3d %10d %9X %7d %5d", i, ch.sizeunpack[i], ch.offset[i], ch.id[i], ch.content[i]);
		if(ch.sizeunpack[i] != ch.sizepack[i])
			printf("     (Packed)");

		if(ch.type[i] >= 2)										/* Print block info if subdir*/
		{
			readblockheader(&ch, i);
			printf("%8d", ch.blocks);
			if((mode[0] == 'f') || (mode[0] == 'F'))
				for(j=1; j<=ch.blocks; j++)
					printf("\n%47d %6d @ %X", j, ch.blsize[j], ch.bloffset[j]);
		}
	}
}

void unpack(unsigned char *pack, unsigned char *unpack, long unpacksize)
{
	unsigned char *byteptr;
	unsigned char *exptr;
	unsigned long word = 0;
	int offs_token	[16384];
	int len_token	[16384];
	int org_token	[16384];
	int val, i, nbits = 0, ntokens = 0;

	for (i = 0; i < 16384; ++i)
	{
		len_token [i] = 1;
		org_token [i] = -1;
	}
	byteptr	= pack;
	exptr	= unpack;

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

void extractchunk(chunkdir *ch, unsigned short int chunk)
{
	unsigned char *packedchunk, *unpackedchunk;
	char fileout[15];
	FILE *in, *out;

	packedchunk = realloc(NULL, ch->sizepack[chunk]);
	unpackedchunk = realloc(NULL, ch->sizeunpack[chunk]);
	if((packedchunk == NULL) || (unpackedchunk == NULL))
	{
		printf("\nOut of memory extracting chunk %d in %s", chunk, ch->file);
		exit(-1);
	}
	if((chunk < 1) || (chunk > ch->chunks))
	{
		printf("\nChunk %d/%d doesn't exist in %s", chunk, ch->chunks, ch->file);
		exit(-1);
	}

	sprintf(fileout, "chunk_%d.dat", chunk);
	in = fopen(ch->file, "rb");
	out = fopen(fileout, "wb");
	fseek(in, ch->offset[chunk], SEEK_SET);												/* Go to chunk offset */
	fread(packedchunk, ch->sizepack[chunk], 1, in);										/* Copy chunk into array */

	switch(ch->type[chunk])
	{
		case 0:	/* Unpacked chunk */													/* Do nothing, write to file */
		case 2:	/* Unpacked subdir */
			fwrite(packedchunk, ch->sizeunpack[chunk], 1, out);
			break;
		case 1:	/* Packed chunk */														/* Unpack, write to file */
			unpack(packedchunk, unpackedchunk, ch->sizeunpack[chunk]);
			fwrite(unpackedchunk, ch->sizeunpack[chunk], 1, out);
			break;
		case 3:	/* Packed subdir */
			readblockheader(ch, chunk);													/* Find out subdir header size */
			fwrite(packedchunk, ch->bloffset[1], 1, out);								/* Write subdir header to file */
			unpack(packedchunk+ch->bloffset[1], unpackedchunk, ch->sizeunpack[chunk]);	/* Unpack starting after header */
			fwrite(unpackedchunk, ch->sizeunpack[chunk]-ch->bloffset[1], 1, out);		/* Write to file */
			break;
	}
	printf("\nExtracted %s from %s", fileout, ch->file);
	if(ch->type[chunk] >= 2)
		printf(" (subdir)");

	free(packedchunk);
	free(unpackedchunk);
	fclose(in);
	fclose(out);
}

void extractblock(chunkdir *ch, unsigned short int block)
{
	unsigned char *packedblock, *unpackedblock;
	char fileout[15];
	FILE *in, *out;

	packedblock = realloc(NULL, ch->blsize[block]);
	unpackedblock = realloc(NULL, ch->blsize[block]);
	if((packedblock == NULL) || (unpackedblock == NULL))
	{
		printf("\nOut of memory extracting block %d in %s", block, ch->file);
		exit(-1);
	}
	if((ch->subdir == 0) || (ch->type[ch->subdir] < 2))
	{
		printf("\nChunk %d in %s not a subdir, run readblockheader()", ch->subdir, ch->file);
		exit(-1);
	}
	if((block < 1) || (block > ch->blocks))
	{
		printf("\nBlock %d/%d doesn't exist in subdir", block, ch->blocks);
		exit(-1);
	}

	sprintf(fileout, "block_%d.dat", block);
	in = fopen(ch->file, "rb");
	out = fopen(fileout, "wb");
	fseek(in, ch->offset[ch->subdir]+ch->bloffset[block], SEEK_SET);	/* Go to block offset */
	fread(packedblock, ch->blsize[block], 1, in);						/* Read block data */
	if(ch->type[ch->subdir] == 2)										/* Unpacked subdir */
		fwrite(packedblock, ch->blsize[block], 1, out);					/* Write block to file as it is */
	else																/* Packed subdir */
	{
		unpack(packedblock, unpackedblock, ch->blsize[block]);			/* Unpack, then write block */
		fwrite(unpackedblock, ch->blsize[block], 1, out);
	}
	printf("\nExtracted %s from %s", fileout, ch->file);

	free(packedblock);
	free(unpackedblock);
	fclose(in);
	fclose(out);
}

void replacechunk(chunkdir *ch, unsigned short int chunk, char *file)
{
	unsigned int i, size, offset, oldoffset;
	unsigned char *oldres, *newres;
	FILE *in, *out;

	in = fopen(ch->file, "rb");
	oldres = realloc(NULL, ch->chunkdiroffset+6+10*ch->chunks);
	if(oldres)
		fread(oldres, ch->chunkdiroffset+6+10*ch->chunks, 1, in);
	fclose(in);

	in = fopen(file, "rb");
	if(in == NULL)
	{
		printf("\nCan't import, %s not found", file);
		exit(-1);
	}
	fseek(in, 0, SEEK_END);									/* Enough memory for old chunk + new file */
	size = ftell(in);
	newres = realloc(NULL, ch->chunkdiroffset+6+10*ch->chunks+size);
	if((oldres == NULL) || (newres == NULL))
	{
		printf("\nOut of memory importing %s to %s", file, ch->file);
		exit(-1);
	}
	memcpy(newres, oldres, 0x7C);
	offset = 0x80;
	printf("\nChunk %d in %s replaced with %s\n\t%d -> %d bytes",
			chunk, ch->file, file, ch->sizeunpack[chunk], size);

	for(i=1; i<=ch->chunks; i++)
	{
		oldoffset = ch->offset[i];
		ch->offset[i] = offset;

		if(i == chunk)
		{
			fseek(in, 0, SEEK_SET);
			fread(newres+offset, size, 1, in);
			ch->sizepack[i] = ch->sizeunpack[i] = size;		/* Packing isn't supported, */
			if(ch->type[i] % 2 != 0)						/* new chunk becomes unpacked */
				ch->type[i] = ch->type[i] - 1;
		}
		else
			memcpy(newres+offset, oldres+oldoffset, ch->sizepack[i]);

		offset = offset + ch->sizepack[i];
		while(offset % 4 != 0)								/* All chunks start at even boundary */
		{
			*(newres+offset) = 0x00;
			offset++;
		}
	}

	ch->chunkdiroffset = offset;
	memcpy(newres+0x7C, &ch->chunkdiroffset, 4);
	memcpy(newres+offset, &ch->chunks, 2);		offset += 2;
	memcpy(newres+offset, &ch->offset[1], 4);	offset += 4;
	for(i=1; i<=ch->chunks; i++)
	{
		memcpy(newres+offset, &ch->id[i], 2);			offset += 2;
		memcpy(newres+offset, &ch->sizeunpack[i], 3);	offset += 3;
		memcpy(newres+offset, &ch->type[i], 1);			offset += 1;
		memcpy(newres+offset, &ch->sizepack[i], 3);		offset += 3;
		memcpy(newres+offset, &ch->content[i], 1);		offset += 1;
	}

	out = fopen(ch->file, "wb");
	fwrite(newres, ch->chunkdiroffset+6+10*ch->chunks, 1, out);
	fclose(in);
	fclose(out);
	free(oldres);
	free(newres);
}

void replaceblock(chunkdir *ch, unsigned short int block, char *file)
{
	int i;
	unsigned int size, offset, oldoffset;
	unsigned char *oldchunk, *newchunk;
	FILE *in, *out;

	if((ch->subdir == 0) || (ch->type[ch->subdir] < 2))
	{
		printf("\nChunk %d in %s not a subdir, run readblockheader()", ch->subdir, ch->file);
		exit(-1);
	}
	if((block < 1) || (block > ch->blocks))
	{
		printf("\nBlock %d/%d doesn't exist in subdir", block, ch->blocks);
		exit(-1);
	}

	in = fopen(file, "rb");
	if(in == NULL)
	{
		printf("\nCan't import, %s not found", file);
		exit(-1);
	}
	fseek(in, 0, SEEK_END);
	size = ftell(in);
	fclose(in);
	newchunk = realloc(NULL, ch->sizeunpack[ch->subdir]+size);
	oldchunk = realloc(NULL, ch->sizeunpack[ch->subdir]+size);
	if((oldchunk == NULL) || (newchunk == NULL))
	{
		printf("\nOut of memory importing %s to %s", file, ch->file);
		exit(-1);
	}

	in = fopen(ch->file, "rb");
	fseek(in, ch->offset[ch->subdir], SEEK_SET);
	if(ch->type[ch->subdir] == 2)
		fread(oldchunk, ch->sizeunpack[ch->subdir], 1, in);
	if(ch->type[ch->subdir] == 3)
	{
		fread(newchunk, ch->sizeunpack[ch->subdir], 1, in);		/* Read temporarily to newchunk */
		unpack(newchunk, oldchunk, ch->sizeunpack[ch->subdir]);	/* Unpack to oldchunk */
	}
	fclose(in);
	offset = 6+4*ch->blocks;
	printf("\nBlock %d:%d in %s replaced with %s\n\t%d -> %d bytes",
			ch->subdir, block, ch->file, file, ch->blsize[block], size);

	for(i=1; i<=ch->blocks+1; i++)								/* One extra for ch->blocks[i+1] (total size) */
	{
		oldoffset = ch->bloffset[i];
		ch->bloffset[i] = offset;

		if(i == block)
		{
			in = fopen(file, "rb");
			fread(newchunk+offset, size, 1, in);
			ch->blsize[i] = size;
		}
		if((i != block) && (i <= ch->blocks))
			memcpy(newchunk+offset, oldchunk+oldoffset, ch->blsize[i]);

		offset = offset + ch->blsize[i];
	}

	memcpy(newchunk, &ch->blocks, 2);							/* Write header with updated offsets */
	for(i=1; i<=ch->blocks+1; i++)
		memcpy(newchunk+2+4*(i-1), &ch->bloffset[i], 4);
	out = fopen("subdir.dat", "wb");
	fwrite(newchunk, ch->bloffset[ch->blocks+1], 1, out);
	fclose(in);
	fclose(out);
	free(oldchunk);
	free(newchunk);
	replacechunk(ch, ch->subdir, "subdir.dat");
	remove("subdir.dat");
}

void loadpal(chunkdir *ch, unsigned short int chunk, image *img)
{
	unsigned int i;
	FILE *in;

	if((chunk < 1) || (chunk > ch->chunks))
	{
		printf("\nChunk %d/%d doesn't exist in file", chunk, ch->chunks);
		exit(-1);
	}
	if(ch->sizeunpack[chunk] != 768)
	{
		printf("\nChunk %d in %s is not a palette", chunk, ch->file);
		exit(-1);
	}

	in = fopen(ch->file, "rb");
	fseek(in, ch->offset[chunk], SEEK_SET);
	for(i=0; i<256; i++)
	{
		fread(&img->r[i], 1, 1, in);
		fread(&img->g[i], 1, 1, in);
		fread(&img->b[i], 1, 1, in);
		img->a[i] = 0x00;
	}
	fclose(in);
}

void extractbmp(chunkdir *ch, unsigned short int block, image *img)
{
	unsigned int i, compression=0, pixels=0;
	unsigned char byte[4];
	FILE *in;

	if((block < 1) || (block > ch->blocks))
	{
		printf("\nCan't extract bitmap %d/%d from subdir", block, ch->blocks);
		exit(-1);
	}
	if((ch->subdir == 0) || (ch->content[ch->subdir] != 2))
	{
		printf("\nChunk %d in %s is not a bitmap", ch->subdir, ch->file);
		return;
	}

	memset(&img->pix, 0x00, BMPX*BMPY);							/* Clear image */
	in = fopen(ch->file, "rb");
	fseek(in, ch->offset[ch->subdir]+ch->bloffset[block]+4, SEEK_SET);
	fread(&compression, 2, 1, in);								/* Read compression type */
	fseek(in, 2, SEEK_CUR);										/* Skip next 2 unknown bytes */
	fread(&img->x, 2, 1, in);									/* Read image width */
	fread(&img->y, 2, 1, in);									/* Read image height */
	fseek(in, 16, SEEK_CUR);									/* Skip to bitmap data */

	if(compression != 4)
		fread(&img->pix, img->x, img->y, in);					/* No compression, read x*y pixels */

	else
	{
		while(pixels < img->x * img->y - 1)						/* Uncompress until x*y pixels */
		{
			fread(&byte[0], 1, 1, in);

			if(byte[0] == 0)
			{
				fread(&byte[1], 1, 2, in);
				memset(&img->pix[pixels], byte[2], byte[1]);
				pixels = pixels + byte[1];
			}

			if((byte[0] > 0) && (byte[0] < 0x80))
			{
				fread(&img->pix[pixels], 1, byte[0], in);
				pixels = pixels + byte[0];
			}

			if(byte[0] == 0x80)
			{
				fread(&byte[1], 1, 2, in);

				if((byte[1] == 0x00) && (byte[2] == 0x00))
					break;

				if(byte[2] < 0x80)
					pixels = pixels + byte[2] * 256 + byte[1];

				if(byte[2] == 0x80)
				{
					fread(&img->pix[pixels], 1, byte[1], in);
					pixels = pixels + byte[1];
				}

				if((byte[2] > 0x80) && (byte[2] < 0xC0))
				{
					fread(&img->pix[pixels], 1, (byte[2] & 0x3F) * 256 + byte[1], in);
					pixels = pixels + (byte[2] & 0x3F) * 256 + byte[1];
				}

				if(byte[2] > 0xC0)
				{
					fread(&byte[3], 1, 1, in);
					memset(&img->pix[pixels], byte[3], (byte[2] & 0x3F) * 256 + byte[1]);
					pixels = pixels + (byte[2] & 0x3F) * 256 + byte[1];
				}
			}

			if(byte[0] > 0x80)
				pixels = pixels + (byte[0] & 0x7F);
		}
	}

	fclose(in);

	for(i=0; i<strlen(ch->file); i++)							/* resfilename_[chunk#]_[block#] */
		if(ch->file[i] == '.')
			img->file[i] = '\0';
		else
			img->file[i] = ch->file[i];
	sprintf(img->file, "%s_%d_%d.bmp", img->file, ch->subdir, block);
	printf("\n%s extracted, %d * %d", img->file, img->x, img->y);
}

void importbmp(chunkdir *ch, unsigned short int block, image *img)
{
	unsigned int pos, pixels=0;
	unsigned short int header1, header2, header3, temp;
	FILE *out;

	out = fopen(ch->file, "rb");								/* Preserve old values */
	fseek(out, ch->offset[ch->subdir]+ch->bloffset[block]+6, SEEK_SET);
	fread(&header1, 2, 1, out);
	fread(&header2, 2, 1, out);
	fseek(out, 6, SEEK_CUR);
	fread(&header3, 2, 1, out);
	fclose(out);

	out = fopen("bmp.dat", "wb");
	fprintf(out, "%c%c%c%c", 0, 0, 0, 0);						/* ??? Always 0 */
	fprintf(out, "%c%c", 4, 0);									/* Compression */
	fwrite(&header1, 2, 1, out);								/* ??? */
	fprintf(out, "%c%c", img->x, img->x>>8);					/* Width */
	fprintf(out, "%c%c", img->y, img->y>>8);					/* Height */
	fprintf(out, "%c%c", img->x, img->x>>8);					/* Width in bytes */
	fprintf(out, "%c", (int)floor(log(img->x)/log(2)));			/* Log2 width */
	fprintf(out, "%c", (int)floor(log(img->y)/log(2)));			/* Log2 height */
	temp = floor((float)header3 / (float)header2 * img->x);		/* Animation frames */
	fwrite(&temp, 2, 1, out);
	temp = img->y - 1;
	fwrite(&temp, 2, 1, out);
	temp = floor((float)header3 / (float)header2 * img->x) + 1;
	fwrite(&temp, 2, 1, out);
	fwrite(&img->y, 2, 1, out);
	fprintf(out, "%c%c%c%c", 0, 0, 0, 0);						/* ??? Always 0 */

	while(pixels < img->x * img->y)								/* Compress, only 00 nn xx case */
	{
		pos = 1;
		while((img->pix[pixels] == img->pix[pixels+pos]) && (pos < 255) && pixels+pos<img->x*img->y)
			pos++;												/* Adjacent identical colours */
		fprintf(out, "%c", 0x00);
		fprintf(out, "%c", pos);
		fprintf(out, "%c", img->pix[pixels]);
		pixels = pixels + pos;
	}
	fprintf(out, "%c%c%c", 0x80, 0x00, 0x00);

	fclose(out);
	replaceblock(ch, block, "bmp.dat");
	remove("bmp.dat");
	printf("\n%s imported, %d * %d pixels", img->file, img->x, img->y);
}

void savebmp(image *img)
{
	int i, j, offset;
	FILE *out;

	out = fopen(img->file, "wb");
	fprintf(out, "BM");									/* BMP magic number */
	if(img->x % 4 > 0)
		offset = 54 + 1024 + 4 * (floor(img->x / 4) + 1) * img->y;
	else
		offset = 54 + 1024 + img->x * img->y;
	fwrite(&offset, 4, 1, out);							/* BMP file size */
	fprintf(out, "%c%c%c%c", 0x00, 0x00, 0x00, 0x00);	/* Reserved */
	offset = 54 + 1024;
	fwrite(&offset, 4, 1, out);							/* Full header size */
	offset = 40;
	fwrite(&offset, 4, 1, out);							/* Remaining header size */
	fwrite(&img->x, 4, 1, out);							/* Width */
	fwrite(&img->y, 4, 1, out);							/* Height */
	fprintf(out, "%c%c", 0x01, 0x00);					/* Colour planes */
	fprintf(out, "%c%c", 0x08, 0x00);					/* 8 bits / pixel */
	fprintf(out, "%c%c%c%c", 0x00, 0x00, 0x00, 0x00);	/* No compression */
	offset = img->x*img->y;
	fwrite(&offset, 4, 1, out);							/* Image size */
	fprintf(out, "%c%c%c%c", 0x00, 0x00, 0x00, 0x00);	/* Horizontal resolution */
	fprintf(out, "%c%c%c%c", 0x00, 0x00, 0x00, 0x00);	/* Vertical resolution */
	fprintf(out, "%c%c%c%c", 0x00, 0x01, 0x00, 0x00);	/* 256 colours in palette */
	fprintf(out, "%c%c%c%c", 0x00, 0x00, 0x00, 0x00);	/* Important colours */

	for(i=0; i<256; i++)
	{
		fwrite(&img->b[i], 1, 1, out);					/* BMP palette is BGR */
		fwrite(&img->g[i], 1, 1, out);
		fwrite(&img->r[i], 1, 1, out);
		fwrite(&img->a[i], 1, 1, out);
	}
	for(i=img->y-1; i>=0; i--)							/* BMPs are upside down */
	{
		fwrite(&img->pix[img->x*i], img->x, 1, out);
		if(img->x % 4 > 0)								/* Padding */
			for(j=0; j<4*(floor(img->x/4)+1)-img->x; j++)
				fprintf(out, "%c", 0x00);
	}

	fclose(out);
}

void loadbmp(image *img, char *file)
{
	char magic[3];
	unsigned char r, g, b;
	unsigned short int j, k, bpp;
	int i, offset, test=0, nearest, smallestdeviation;
	FILE *in;

	in = fopen(file, "rb");
	if(in == NULL)
	{
		printf("\n%s was not found", file);
		exit(-1);
	}
	fgets(magic, 3, in);
	if(strncmp(magic, "BM", 3) != 0)
	{
		printf("\n%s isn't a .bmp file", file);
		exit(-1);
	}

	fseek(in, 8, SEEK_CUR);
	fread(&offset, 4, 1, in);
	fseek(in, 4, SEEK_CUR);
	fread(&img->x, 4, 1, in);
	fread(&img->y, 4, 1, in);
	if(img->x*img->y > BMPX*BMPY)
	{
		printf("\n%s is too large, %d*%d is max", file, BMPX, BMPY);
		exit(-1);
	}
	fseek(in, 2, SEEK_CUR);
	fread(&bpp, 2, 1, in);
	if((bpp != 8) && (bpp != 24))
	{
		printf("\n%s doesn't have 8 or 24 bits/pixel", file);
		exit(-1);
	}
	fread(&test, 4, 1, in);
	if(test != 0)
	{
		printf("\n%s isn't uncompressed", file);
		exit(-1);
	}

	fseek(in, offset, SEEK_SET);

	if(bpp == 8)
	{
		for(i=img->y-1; i>=0; i--)									/* Read image */
		{
			fread(&img->pix[img->x*img->y-img->x*(img->y-i)], img->x, 1, in);
			if(img->x % 4 > 0)										/* Skip row%4=0 padding */
				fread(&test, 1, 4*(floor(img->x/4)+1)-img->x, in);
		}
	}
	if(bpp == 24)
	{
		printf("\n%s is 24bpp, converting to 8bpp palette", file);
		for(i=img->y-1; i>=0; i--)
		{
			for(j=0; j<img->x; j++)
			{
				fread(&b, 1, 1, in);								/* Read BGR components */
				fread(&g, 1, 1, in);
				fread(&r, 1, 1, in);
				smallestdeviation = 0x30000;						/* 3*256^2, > theoretical max */
				for(k=0; k<256; k++)
				{
					test =	(img->r[k] - r) * (img->r[k] - r) +		/* Sum of B+G+R square deviation */
							(img->g[k] - g) * (img->g[k] - g) +
							(img->b[k] - b) * (img->b[k] - b) ;
					if(test < smallestdeviation)					/* Palette entry with smallest */
					{												/* deviation is closest match */
						smallestdeviation = test;
						nearest = k;
					}
					img->pix[i*img->x+j] = nearest;
				}
				if(img->x * 3 % 4 > 0)								/* Skip row%4=0 padding */
					fread(&test, 1, 4*(floor(img->x*3/4)+1)-img->x*3, in);
			}
		}
	}

	fclose(in);
}

void scale2x(image *img)
{
	unsigned int i, epos;
	unsigned char newimg[4*BMPX*BMPY];
	unsigned char b, d, f, h, e1, e2, e3, e4;

	if(img->x > BMPX/2 || img->y > BMPY/2)
	{
		printf("\n%s (%d*%d) too large for scaling", img->file, img->x, img->y);
		return;
	}

	/*  a b c	Scale2x algorithm
		d e f	e-> e1 e2
		g h i		e3 e4		*/

	for(i=0; i<img->x*img->y; i++)
	{
		b = h = d = f = img->pix[i];		/* Image border: use edge colour */
		if(i >= img->x)						/* Else: use prev/next col+row */
			b = img->pix[i-img->x];
		if(i < img->x*img->y-img->x)
			h = img->pix[i+img->x];
		if(i % img->x != 0)
			d = img->pix[i-1];
		if(i+1 % img->x != 0)
			f = img->pix[i+1];

		e1 = e2 = e3 = e4 = img->pix[i];
		if(b != h && d != f)
		{
			if(d == b)
				e1 = d;
			if(b == f)
				e2 = f;
			if(d == h)
				e3 = d;
			if(h == f)
				e4 = f;
		}

		epos = (int)floor(i/img->x) * 4 * img->x + 2 * i%((int)floor(2*img->x));
		newimg[epos] = e1;					/* New indexes in 2x array */
		newimg[epos+1] = e2;
		newimg[epos+2*img->x] = e3;
		newimg[epos+2*img->x+1] = e4;
	}

	img->x = img->x*2;
	img->y = img->y*2;
	memcpy(img->pix, newimg, img->x*img->y);
}

int main(void)
{
	int i, j;
	chunkdir ch;
	image img;

	readchunkdir("restnobj.res", &ch);
	for(i=6; i<=ch.chunks; i++)		/* 1-5 vegetation (too many -> out of memory) */
		if(ch.type[i] >=2 && ch.content[i] == 2)
		{
			readblockheader(&ch, i);
			for(j=1; j<=ch.blocks; j++)
			{
				extractbmp(&ch, j, &img);
				if(i<=37 && img.x <= 128 && img.y <= 128)	/* explosions -> once */
					scale2x(&img);
				else
					while(img.x <= 128 && img.y <= 128)		/* rest -> until 256 pixels */
						scale2x(&img);
				importbmp(&ch, j, &img);
			}
		}

	return(0);
}
