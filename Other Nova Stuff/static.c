//Static 1.7		by Gigaquad		Copyright 2008

#include "lgres.c"

unsigned short int loHeight[257][257];		//low-resolution map
unsigned short int hiHeight[516][516];		//high-resolution map (513 + overflow)
unsigned short int minHeight, maxHeight;	//lowest and highest points on map

int main(void);
void readFile(chunkdir *chd);
short int bytes2Height(unsigned char bytes[]);
void printSummary(chunkdir *chd);
void writePic(void);
void readPic(void);
void interpol(void);
void height2bytes(unsigned char row[3*513], int rowNr, int reso);
void replaceHeight(chunkdir *chd, int reso);

//ask user for action, then export or import heightmap
int main(void)
{
	char mode;
	chunkdir chd;

	printf("\nEnter the Terra Nova mapfile you want to open: ");
	readChunkdir(gets(chd.file), &chd);
	puts("Select mode:\n[E]xport heightmap from level\n[I]mport heightmap to level");
	scanf("%c", &mode);

	if (tolower(mode) == 'e')			//export:
	{
		readFile(&chd);					//read low-res map from mapfile
		printSummary(&chd);				//print summary of it
		writePic();						//write it to a heightmap file
	}
	if (tolower(mode) == 'i')			//import:
	{
		readPic();						//read low-res map from heightmap file
		interpol();						//create hi-res map from it
		replaceHeight(&chd, 257);		//replace height in low-res map
		replaceHeight(&chd, 513);		//...and high-res
	}

	return(0);
}

//open desired file at chunk 4 (low res, 257*257 tiles). there's 3 bytes per tile,
//2nd and 3rd byte hold first and second half of height respectively (and other stuff, which must be removed)
void readFile(chunkdir *chd)
{
	int i, j;
	unsigned char bytes[3];
	FILE *Map;

	Map = fopen(chd->file, "rb");
	fseek(Map, chunkAdr(chd, 4), SEEK_SET);				//jump to beginning of low-res tiles
	for(i=0; i<257; i++)
		for(j=0; j<257; j++)
		{
			fread(bytes, 3, 1, Map);					//read tile info (3 bytes)
			loHeight[i][j] = bytes2Height(bytes);		//convert the bytes to a height
		}
	fclose(Map);
}

//convert the 2 bytes containing height data to actual height. for some reason 2nd byte goes
//80h...FFh (negative heights), then 00h...7Fh (positive heights), not 00h..FFh
short int bytes2Height(unsigned char bytes[])
{
	short int height;

	bytes[1] = bytes[1] & 0xF0;					//AND with 11110000b: remove shadow+rotation in lower half of byte
	height = bytes[2] * 16 + bytes[1] / 16;

	if (bytes[2] > 0x7F)						//negative height
		height = height - 4096;

	return(height + 2048);						//height is now always 0...4096
}

//print summary of heightmap stats
void printSummary(chunkdir *chd)
{
	int i, j;

	minHeight = maxHeight = loHeight[0][0];		//use first tile as reference...
	for(i=0; i<257; i++)
		for(j=0; j<257; j++)
		{
			if (loHeight[i][j] < minHeight)		//...when scanning all values for min and max height
				minHeight = loHeight[i][j];
			if (loHeight[i][j] > maxHeight)
				maxHeight = loHeight[i][j];
		}

	printf("\n%s:\n", chd->file);
	printf("Height: \t %d ... %d\n", minHeight, maxHeight);
	printf("Peak to peak: \t %d\n", maxHeight - minHeight);
	printf("Height scale: \t %.0f%%\n", (float) maxHeight/4096*100);
	printf("Tiles: \t \t 257 x 257\n");
	printf("Area: \t \t 6000 x 6000");
}

//write heightmap in 16bit raw format, intel byte order. height values are spread evenly in the 0...65536 range
void writePic(void)
{
	int i, j, buf;
	FILE *Pic;

	Pic = fopen("tn.raw", "wb");
	for(i=0; i<257; i++)
		for(j=0; j<257; j++)
		{												//write lowest point as 0, highest as 65535, rest spread evenly
			buf = (loHeight[i][j] - minHeight) * 65535 / (maxHeight - minHeight);
			fwrite(&buf, 2, 1, Pic);					//write 2 bytes of the int (16 bits)
		}												//may divide by 2...16 to see terrain better (for testing)
	fclose(Pic);
}

//get heightmap by reading 2 bytes per tile. user must save raw in intel byte order
void readPic(void)
{
	int i, j, scale;
	unsigned char bytes[2];
	FILE *Pic;

	Pic = fopen("tn.raw", "rb");
	if (Pic == NULL)
	{
		printf("Error: tn.raw was not found.");
		exit(-1);
	}

	puts("Scale to ___%? (1...100, usually 50...75%)");			//heightmap values are evenly spread in a 0...65535 range,
	scanf("%d", &scale);										//therefore we must scale them down by some %,
	if ((scale < 1) || (scale > 100))							//to match the range the map was originally drawn in
		scale = 66;

	for(i=0; i<=257; i++)
		for(j=0; j<257; j++)
		{
			fread(bytes, 2, 1, Pic);								//read 2 bytes for each (16-bit) tile
			loHeight[i][j] = 256 * bytes[1] + bytes[0];				//2nd byte is MSB
			loHeight[i][j] = loHeight[i][j] / 16 * scale / 100;		//turn 0...65535 range to 0...4096 (65535/4096=16)
		}															//scale down height values
	fclose(Pic);
}

//create high-resolution map from the center of low-resolution map, interpolating missing tiles
void interpol(void)
{
	int i, j, i0, j0;
	float x, y;

	for(i=0; i<513; i++)								//257*257 low-res heightmap -> 513*513 high-res heightmap
		for(j=0; j<513; j++)
		{
			x = i % 4 * 0.25;							//x,y are distance (0...1) from current low-res tile to next
			y = j % 4 * 0.25;
			i0 = i / 4 + 64;							//high-res heighmap spans center 50% of 257*257 heightmap,
			j0 = j / 4 + 64;							//-> quadruple resolution, starting at tile (64,64)

			hiHeight[i][j] =							//bilinear interpolation
				loHeight[ i0 ][ j0 ] * (1-x) * (1-y) +
				loHeight[i0+1][ j0 ] *   x   * (1-y) +
				loHeight[ i0 ][j0+1] * (1-x) *   y   +
				loHeight[i0+1][j0+1] *   x   *   y   ;
		}
}

//remove old height data from bytes (preserving other data), put in new height data
void height2bytes(unsigned char row[3*513], int rowNr, int reso)
{
	int i, height;

	for(i=0; i<reso; i++)
	{
		if (reso == 257)
			height = loHeight[rowNr][i] - 2048;			//heightmap has 0...4096 values, should be -2047...2048
		else
			height = hiHeight[rowNr][i] - 2048;

		row[3*i+1] = row[3*i+1] & 0xF;					//replace old height in mapfile bytes with new height
		if (height > 0)									//see my text file re:the reverse engineered
		{												//terra nova map format for explanation
			row[3*i+2] = floor(height / 16);
			row[3*i+1] = row[3*i+1] + 16 * (height - 16 * row[3*i+2]);
		}
		if (height < 0)
		{
			row[3*i+2] = 128 + floor((2048 + height) / 16);
			row[3*i+1] = row[3*i+1] + 16 * ((2048 + height) - (row[3*i+2] - 128) * 16);
		}
	}
}

//replace existing height in mapfile with imported one: read tile contents into variables, then
//replace height data with new new height (preserving other data), go back and replace bytes
void replaceHeight(chunkdir *chd, int reso)
{
	int i;
	unsigned char row[3*513];
	FILE *Map;

	Map = fopen(chd->file, "rb+");
	if(reso == 257)
		fseek(Map, chunkAdr(chd, 4), SEEK_SET);
	else
		fseek(Map, chunkAdr(chd, 5), SEEK_SET);

	for(i=0; i<reso; i++)
	{
		fread(row, 3*reso, 1, Map);					//read entire row
		height2bytes(row, i, reso);					//replace old height values with new ones
		fseek(Map, -3*reso, SEEK_CUR);				//go back to line start
		fflush(Map);
		fwrite(row, 3*reso, 1, Map);				//overwrite with new heights
		fflush(Map);								//flush stream when changing read/write
	}
	fclose(Map);
}