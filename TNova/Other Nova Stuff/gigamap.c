//Gigamap 1.2	by Gigaquad		Copyright 2007

#include <stdio.h>
#include <stdlib.h>
#include "lgres.c"

typedef struct settings
{
	unsigned char	txt[513][513];			//texture number
	unsigned char	rot[513][513];			//rotation value
	int				planet, skip, resize;	//settings
	char			mode;					//'L'ow, 'H'igh or 'B'oth
	char			mapFile[255];			//filename of mapfile
};

int main(void);
void askUser(struct settings *s);
void getPlanet(struct settings *s);
void readMap(struct settings *s, int size);
unsigned char getTexture(unsigned char byte);
unsigned char getRotation(short int byte);
void writeBat(struct settings *s, int size);

//first ask user for options, then extract texture map accordingly. do it twice if selected "Both"
int main(void)
{
	struct settings s;

	askUser(&s);
	puts("Creating batch file(s)...");
	getPlanet(&s);
	if ((s.mode == 'L') || (s.mode == 'B'))
	{
		readMap(&s, 257);
		writeBat(&s, 257);
	}
	if ((s.mode == 'H') || (s.mode == 'B'))
	{
		readMap(&s, 513);
		writeBat(&s, 513);
	}
	puts("Done. Remember: compiling a new texture map will overwrite existing one!");
	return(0);
}

//always add 1 to skip factor since it is later used to advance loop, and can't be 0 (but starting at 0 is
//easier for user to understand. mode can be entered in either case, so it must be checked
void askUser(struct settings *s)
{
	puts("\nEnter the Terra Nova mapfile you want to open:");
	gets(s->mapFile);

	puts("Skip how many tiles after drawing one? (0...10)");
	scanf("%d", &s->skip);
		s->skip = s->skip+1;
		if (s->skip <= 0) s->skip = 1;
		if (s->skip > 9) s->skip = 10;

	puts("Resize tiles to ___%? (1...100)");
	scanf("%d", &s->resize);
		if (s->resize < 1) s->resize = 1;
		if (s->resize > 100) s->resize = 100;

	puts("Create which texture map?\n[L]ow res\n[H]igh res\n[B]oth");
	do {
		scanf("%c", &s->mode);
		if (s->mode == 'l') s->mode = 'L';
		if (s->mode == 'h') s->mode = 'H';
		if (s->mode == 'b') s->mode = 'B';
	} while ((s->mode != 'L') && (s->mode != 'H') && (s->mode != 'B'));
}

//open file at chunk 8, scan for string "plnt", then read next char which determines the correct texture set
void getPlanet(struct settings *s)
{
	unsigned char byte;
	FILE *Map = fopen(s->mapFile, "rb");

	fseek (Map, chunkOffset(s->mapFile, 8), SEEK_SET);
	do											//both upper and lower case exist
	{
		byte = fgetc(Map);
		if ((byte == 'p') || (byte == 'P'))
		{
			byte = fgetc(Map);
			if ((byte == 'l') || (byte == 'L'))
			{
				byte = fgetc(Map);
				if ((byte == 'n') || (byte == 'N'))
				{
					byte = fgetc(Map);
					if ((byte == 't') || (byte == 'T'))
					{
						byte = fgetc(Map);
						s->planet = atoi(&byte);
						if (s->planet == 4)			//plnt4 has same textures as plnt0
							s->planet = 0;
						break;
					}
				}
			}
		}
	}
	while (feof(Map) == 0);
	fclose(Map);
}

//chunk 4 in mapfile contains the low-res map (257*257 tiles with 3 bytes each)
//chunk 5 in mapfile contains the high-res map (513*513 tiles with 3 bytes each)
//read bytes which contain texture and rotation values and save them in a 2d array
void readMap (struct settings *s, int size)
{
	unsigned char byte;
	int i, j;
	FILE *Map = fopen(s->mapFile, "rb");

	if (size == 257)
		fseek(Map, chunkOffset(s->mapFile, 4), SEEK_SET);			//jump to first low-res tile
	else
		fseek(Map, chunkOffset(s->mapFile, 5), SEEK_SET);			//jump to first high-res tile
	for(i=0; i!=size; i++)
	{
		for(j=0; j!=size; j++)
		{
			fread(&byte, sizeof(byte), 1, Map);			//tile texture
			s->txt[i][j] = getTexture(byte);
			fread(&byte, sizeof(byte), 1, Map);			//rotation
			s->rot[i][j] = getRotation(byte);
			fread(&byte, sizeof(byte), 1, Map);			//don't need third byte
		}
	}
}

//must remove all other information from byte except texture number
unsigned char getTexture(unsigned char byte)
{
	if (byte > 191)				//texture + shadow + blast -> remove blast
		byte = byte - 64;
	if (byte > 127)				//texture + shadow -> remove shadow
		byte = byte - 128;
	if (byte > 63)				//texture + blast -> remove blast
		byte = byte - 64;

	return(byte);
}

//must remove all other information from byte except tile rotation
unsigned char getRotation(short int byte)
{								//remove partial tile height in upper half of byte by ANDing with 00001111
	byte = byte & 15;			//remaining is rotation value + partial shadow value
	if ((byte >= 0) && (byte <= 3))		//0 + (0...3) is 0 deg
		byte = 'a';
	if ((byte >= 4) && (byte <= 7))		//4 + (0...3) is 90 deg
		byte = 'b';
	if ((byte >= 8) && (byte <= 11))	//8 + (0...3) is 180 deg
		byte = 'c';
	if ((byte >= 12) && (byte <= 15))	//12 + (0...3) is 270 deg
		byte = 'd';

	return(byte);
}

//create a batch file that makes imagemagick compile tile textures into a large texture map
//first resize original textures to specified size, then copy three rotated versions of them
//then copy the correct tile textures next to each other, skipping rows and cols accordingly
//when end of line is reached, create an intermediate texture map and copy it to another directory
//finally merge all intermediate texture maps into final texture map
void writeBat(struct settings *s, int size)
{
	int i, j;
	FILE *Bat;

	if (size == 257)
		Bat = fopen("mapLo.bat", "wa");
	else
		Bat = fopen("mapHi.bat", "wa");

	fprintf(Bat, "for %%%%a in (gmap\\p\\*.*) do del %%%%a\n");		//clean up prev session
	fprintf(Bat, "for %%%%a in (gmap\\s\\*.*) do del %%%%a\n");
	fprintf(Bat, "for %%%%a in (gmap\\t\\*.*) do del %%%%a\ncd gmap\\%d\n", s->planet);
	fprintf(Bat, "for %%%%a in (*.*) do ...\\convert -resize %d%%%% %%%%a ..\\p\\a%%%%a\n", s->resize);
	fprintf(Bat, "for %%%%a in (*.*) do ...\\convert -rotate 90 ..\\p\\a%%%%a ..\\p\\b%%%%a\n");
	fprintf(Bat, "for %%%%a in (*.*) do ...\\convert -rotate 180 ..\\p\\a%%%%a ..\\p\\c%%%%a\n");
	fprintf(Bat, "for %%%%a in (*.*) do ...\\convert -rotate 270 ..\\p\\a%%%%a ..\\p\\d%%%%a\ncd...\n");

	for(i=0; i<size; i=i+s->skip)			//rows
	{
		for(j=0; j<size; j=j+s->skip)		//cols
			fprintf(Bat, "copy /y gmap\\p\\%c%d.* gmap\\s\\%d.*\n", s->rot[i][j], s->txt[i][j], j+100);
		fprintf(Bat, "montage -geometry +0+0 -tile x1 gmap\\s\\*.* gmap\\t\\%d.gif\n", i+100);
	}
	fprintf(Bat, "montage -geometry +0+0 -tile 1x gmap\\t\\*.* texmap.gif\n\n");
	fclose(Bat);
}