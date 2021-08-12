#include "res.h"

void readchunkdir(HWND hwnd, RESFILE *res)
{
	HANDLE hFile;
	DWORD dwRead;
	unsigned int i, j, offset;
	char buf[MAX_PATH] = "";

	snprintf(buf, MAX_PATH, "Resolution - %s", res->file);
	SetWindowText(hwnd, buf);
	hFile = CreateFile(res->file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(hFile, CHUNKDIR, NULL, FILE_BEGIN);
		ReadFile(hFile, &offset, 4, &dwRead, NULL);
		SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
		ReadFile(hFile, &res->chunks, 2, &dwRead, NULL);
		if(res->chunks > MAXCHUNKS)
		{
			MessageBox(hwnd, "The file has too many chunks!", "Error", MB_OK | MB_ICONERROR);
			DestroyWindow(hwnd);
		}
		ReadFile(hFile, &offset, 4, &dwRead, NULL);

		for(i=0; i<res->chunks; i++)
		{
			ReadFile(hFile, &res->id[i], 2, &dwRead, NULL);
			ReadFile(hFile, &res->unpack[i], 3, &dwRead, NULL);
			ReadFile(hFile, &res->type[i], 1, &dwRead, NULL);
			ReadFile(hFile, &res->packed[i], 3, &dwRead, NULL);
			ReadFile(hFile, &res->content[i], 1, &dwRead, NULL);
			res->unpack[i] &= 0x00FFFFFF;						/* 3 byte data in 4 byte variable */
			res->packed[i] &= 0x00FFFFFF;
			res->offset[i] = offset;
			offset = offset + res->packed[i];
			if(offset % 4 != 0)									/* Data Structure Padding: 4 bytes */
				offset = 4 * (floor(offset / 4) + 1);
		}
		for(i=0; i<res->chunks; i++)
		{
			snprintf(buf, MAX_PATH, "Chunk %d", i);
			addItemToTree(GetDlgItem(hwnd, IDC_TREE), buf, 1);

			if(res->type[i] >= 2)
			{
				SetFilePointer(hFile, res->offset[i], NULL, FILE_BEGIN);
				ReadFile(hFile, &res->blocks[i], 2, &dwRead, NULL);
				if(res->blocks[i] > MAXBLOCKS)
				{
					MessageBox(hwnd, "The file has too many blocks!", "Error", MB_OK | MB_ICONERROR);
					DestroyWindow(hwnd);
				}
				for(j=0; j<res->blocks[i]; j++)
				{
					snprintf(buf, MAX_PATH, "Block %d", j);
					addItemToTree(GetDlgItem(hwnd, IDC_TREE), buf, 2);
				}
			}
		}
	}
	CloseHandle(hFile);
}

void unpackdata(unsigned char *pack, unsigned char *unpack, long unpacksize)
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

void readChunkOrBlock(RESFILE *res, RESDATA *data)
{
	HANDLE hFile;
	DWORD dwRead;
	unsigned int offset, headersize;
	unsigned char *tmp;

	hFile = CreateFile(res->file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(hFile, res->offset[res->currchunk], NULL, FILE_BEGIN);
		switch(res->type[res->currchunk])
		{
			case 0: /* Flat, unpacked */
				data->size = res->unpack[res->currchunk];
				data->unpack = malloc(data->size);
				if(data->unpack)
					ReadFile(hFile, data->unpack, data->size, &dwRead, NULL);
			break;
			case 1: /* Flat, packed */
				data->size = res->unpack[res->currchunk];
				data->unpack = malloc(data->size);
				data->packed = malloc(res->packed[res->currchunk]);
				if(data->unpack && data->packed)
				{
					ReadFile(hFile, data->packed, res->packed[res->currchunk], &dwRead, NULL);
					unpackdata(data->packed, data->unpack, data->size);
					free(data->packed);
				}
			break;
			case 2: /* Subdir, unpacked */
				SetFilePointer(hFile, 2+4*res->currblock, NULL, FILE_CURRENT);
				ReadFile(hFile, &offset, 4, &dwRead, NULL);
				ReadFile(hFile, &data->size, 4, &dwRead, NULL);
				data->size = data->size - offset;
				SetFilePointer(hFile, res->offset[res->currchunk]+offset, NULL, FILE_BEGIN);
				data->unpack = malloc(data->size);
				if(data->unpack)
					ReadFile(hFile, data->unpack, data->size, &dwRead, NULL);
			break;
			case 3: /* Subdir, packed */
				SetFilePointer(hFile, 2, NULL, FILE_CURRENT);
				ReadFile(hFile, &headersize, 4, &dwRead, NULL);
				SetFilePointer(hFile, res->offset[res->currchunk]+2+4*res->currblock, NULL, FILE_BEGIN);
				ReadFile(hFile, &offset, 4, &dwRead, NULL);
				ReadFile(hFile, &data->size, 4, &dwRead, NULL);
				data->size = data->size - offset;
				data->unpack = malloc(data->size);
				data->packed = malloc(res->packed[res->currchunk]);
				tmp = malloc(res->unpack[res->currchunk]);
				if(data->unpack && data->packed && tmp)
				{
					SetFilePointer(hFile, res->offset[res->currchunk]+headersize, NULL, FILE_BEGIN);
					ReadFile(hFile, data->packed, res->packed[res->currchunk]-headersize, &dwRead, NULL);
					unpackdata(data->packed, tmp, res->unpack[res->currchunk]);
					memcpy(data->unpack, tmp+offset-headersize, data->size);
					free(data->packed);
					free(tmp);
				}
			break;
		}
	}
	CloseHandle(hFile);
}
