// GFX.c - A helper library in C for reading/writing graphics formats
// Copyright 2008 by Gigaquad

#define RAW8BIT			1				// RAW, 8BPP
#define RAW8BITPAL		2				// RAW, 8BPP using palette

typedef struct
{
	char name[255];						// Filename, for saving
	int x;								// Image width
	int y;								// Image height
	int bpp;							// Bits per pixel (8, 16, 24)
	unsigned char pix[650*550];			// Max resolution 650*550
	unsigned char pal[256][3];			// Palette, 256 * R,G,B
} image;

void saveImage(image *img, int format);
	// Generic function to save *img in #defined format
void loadImage(char *file, image *img, int format);
	// Generic function to load image from file (in #defined format) to *img

// No need to call these functions, use saveImage(format) and loadImage(format)
void save8BitRaw(image *img);
	// Save image, RAW 8 bit
void save8BitPalRaw(image *img);
	// Save image, RAW 8 bit, paletted
void load8BitRaw(char *file, image *img);
	// Load RAW 8 bit image from file into *img

void saveImage(image *img, int format)
{
	if(img->name == "")									// Need something in img->name to avoid crash
		sprintf(img->name, "%s", "image");				// since it will be fopen()ed later

	switch(format)
	{
		case	RAW8BIT:								// Note: Passing on the CONTENT of the pointer,
			save8BitRaw(img);							// ie. the address of the original variable
			break;
		case	RAW8BITPAL:
			save8BitPalRaw(img);
			break;
	}
}

void loadImage(char *file, image *img, int format)
{
	FILE *in;

	in = fopen(file, "rb");
	if(in == NULL)
	{
		fprintf(stderr, "\n%s was not found", file);
		exit(-1);
	}
	fclose(in);
	sprintf(img->name, "image");

	switch(format)
	{
		case	RAW8BIT:								// Note: Passing on the CONTENT of the pointer,
			load8BitRaw(file, img);						// ie. the address of the original variable
			break;
	}

}

void save8BitRaw(image *img)
{
	int i;
	FILE *out;

	sprintf(img->name, "%s.raw", img->name);
	out = fopen(img->name, "wb");
	for(i=0; i<img->x*img->y; i++)
		fwrite(&img->pix[i], 1, 1, out);
	fclose(out);
}

void save8BitPalRaw(image *img)
{
	int i;
	FILE *out;

	sprintf(img->name, "%s.raw", img->name);
	out = fopen(img->name, "wb");
	for(i=0; i<img->x*img->y; i++)						// For every pixel:
		fwrite(&img->pal[img->pix[i]][0], 1, 3, out);	// Write corresponding entry in palette
	fclose(out);
}

void load8BitRaw(char *file, image *img)
{
	int size;
	FILE *in;

	in = fopen(file, "rb");
	fseek(in, 0, SEEK_END);								// There is no header to tell the dimensions,
	size = ftell(in);									// instead read file size and assume image is
	img->x = size;										// [filesize] pixels wide and 1 pixel high
	img->y = 1;

	fseek(in, 0, SEEK_SET);
	fread(&img->pix[0], size, 1, in);
	fclose(in);
}