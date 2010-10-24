#include "compression.h"
#include "str.h"

struct TarFile * tFile;
int TarFileCount = 0; /* Keeps track of how many elements there are */

/* Convert Octal to Decimal */
int Oct2Dec(int Octal)
{
	int i = 0, Result = 0;
	for (; Octal != 0; ++i)
	{
		Result += (Octal%10)*(int)pow(8, i);
		Octal /= 10;
	}
	return Result;
}

int ExtractFileFromTar(char * Prefix, int FileID)
{
	char * FileName = malloc(strlen(tFile[FileID].Name)+1);
	strcpy(FileName, tFile[FileID].Name);

	if (tFile[FileID].Type == DIRTYPE) { /* Directory */
		char buffer[262];
		sprintf(buffer, "mkdir %s", FileName);
		system(buffer); /* Whee, bad way of doing shit but it works and it IS an alpha */
	}
	else if (tFile[FileID].Type == REGTYPE || tFile[FileID].Type == AREGTYPE) { /* Normal files */
		/* Write the file */
		FILE * file = fopen(FileName, "wb");
		fwrite(tFile[FileID].Data, 1, tFile[FileID].DataLength, file);
		fclose(file);
		/* Set up the premissions */
		char buffer[267];
		sprintf(buffer, "chmod %d %s", tFile[FileID].Mode, FileName);
		system(buffer);
	}
}

int ExtractLoadedTar(char * Prefix)
{
	int i, Result = 0;
	for (i = 0; i < TarFileCount && Result == 0; ++i) {
		Result = ExtractFileFromTar(Prefix, i);
	}

	return Result;
}

int LoadTarFromFile(char * FileName)
{
	FILE * TarFile = fopen(FileName, "rb");	
	/* Infinite loop, we rely on the if (feof(tar)) break; to escape */
	int Count = 0, TotalSize = 0;
	for(;;)
	{
		/* We use realloc so that this function can possibly be called more than once without leakage */
		struct RawTarHeader RawFile;
		fread(&RawFile, 1, sizeof(RawFile), TarFile);
		TotalSize += sizeof(struct TarFile);
		tFile = realloc(tFile, TotalSize);

		int size = Oct2Dec(atoi(RawFile.size));
		tFile[Count].Data = realloc(tFile[Count].Data, size+1);
		fread(tFile[Count].Data, size, 1, TarFile);
		tFile[Count].Data[size+1] = '\0';
		tFile[Count].DataLength = size;
		tFile[Count].Type = RawFile.typeflag;
		tFile[Count].Mode = atoi(RawFile.mode);
		strcpy(tFile[Count].Name, RawFile.name);
		Count++;

//		printf("%s: %d,%d\n%s\n", RawFile.size, size, tFile[Count-1].DataLength, tFile[Count-1].Data);
		while (fgetc(TarFile) == 0x00) { } /* Discard padding */
		if (feof(TarFile)) break;
		fseek(TarFile, -1, SEEK_CUR);
	}
	TarFileCount = Count;
	fclose(TarFile);
}

int LoadTarFromChar(char * Data, int Size)
{
	/* Infinite loop, we rely on the if (feof(tar)) break; to escape */
	int Count = 0, TotalSize = 0, fOffset = 0;
	for(;;)
	{
		int Offset = 0;
		char Buffer[156]; /* Use this for loading crap that needs to be turned into ints */
		/* We use realloc so that this function can possibly be called more than once without leakage */
		TotalSize += sizeof(struct TarFile);
		if (tFile == NULL)
			tFile = malloc(TotalSize);
		else
			tFile = realloc(tFile, TotalSize);
		memset(tFile[Count].Name, 0, 100);
		strsubcpy(tFile[Count].Name, Data, fOffset, fOffset+100, true, 0x00);

		strsubcpy(Buffer, Data, fOffset+124, fOffset+136, false, -1);
		tFile[Count].DataLength = Oct2Dec(atoi(Buffer));
		tFile[Count].Data = malloc(tFile[Count].DataLength);
		strsubcpy(tFile[Count].Data, Data, fOffset+512, fOffset+512+tFile[Count].DataLength, false, -1);

		tFile[Count].Type = Data[fOffset+156];

		/* Load in the mode */
		memset(Buffer, 0, 156);
		strsubcpy(Buffer, Data, fOffset+100, fOffset+108, false, -1);
		tFile[Count].Mode = atoi(Buffer);

//		printf("%s: %d, %d::%d bytes\n", tFile[Count].Name, tFile[Count].DataLength, fOffset, Size);
	
		for (Offset = fOffset+512+tFile[Count].DataLength; Data[Offset] == 0x00; Offset++) { }

		fOffset = Offset;
		Count++;
		if (fOffset >= Size) break;
	}
	TarFileCount = Count;
	#ifdef DEBUG
		printf("LoadTarFromChar(); completed. Files in tar: %d\n", TarFileCount);
	#endif
}

/* Frees the memory for the tar stored in memory */
void FreeUpTar()
{
	int i;
	for (i = 0; i < TarFileCount; ++i) free(tFile[i].Data);
	free(tFile);
	tFile = NULL;
}


/***********************************************************************************/
/* Gzip functions */
char * InfFile2Char(const char * FileName_In, char *dest, int * Size)
{
#ifdef USE_GZ
	int ret;
	short FirstRun = 1; 
	unsigned long TotalSize = 0;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	FILE * source;
	if (!(source = fopen(FileName_In,"rb"))) return NULL;

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, 16+MAX_WBITS);
	if (ret != Z_OK) return NULL;

	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
	        if (ferror(source)) {
			printf("Error: Could not open file\n");
        	    (void)inflateEnd(&strm);
	            return NULL;
	        }
        	if (strm.avail_in == 0)
	            break;
        	strm.next_in = in;

	        /* run inflate() on input until output buffer not full */
        	do {
	       		strm.avail_out = CHUNK;
	        	strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
	            	switch (ret) {
	            	case Z_NEED_DICT:
	                	ret = Z_DATA_ERROR;     /* and fall through */
	            	case Z_DATA_ERROR:
	        	case Z_MEM_ERROR:
		                (void)inflateEnd(&strm);
	                	return NULL;
	            	}
			have = CHUNK - strm.avail_out;
			if (FirstRun == 1) {
				dest = malloc(have);
				if (dest == NULL) {
					printf("Error: Out of memory\n");
					return NULL;
				}
				int i;
				for (i = 0; i < have; ++i) dest[i] = out[i];
				FirstRun = 0;
				TotalSize = have;
			}
			else {
				TotalSize += have;
				dest = realloc(dest, TotalSize);
				if (dest == NULL) {
					printf("Error: Out of memory\n");
					return NULL;
				}
				int i;
				for (i = 0; i < have; ++i) dest[TotalSize-have+i] = out[i];
			}
			int i;
		} while (strm.avail_out == 0);
	/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);
	*Size = TotalSize;

	fclose(source);
	/* clean up and return */
	(void)inflateEnd(&strm);
	return dest;
#else
	printf("This version of GekPkg cannot use zlib.\n");
	return NULL;
#endif
}

char * gzInfFromChar(char * Data, int zSize, int * Size)
{
#ifdef USE_GZ
	int ret;
	short FirstRun = true; 
	unsigned long TotalSize = 0;
	unsigned have;
	z_stream stream;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];
	char * RetData;

	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = 0;
	stream.next_in = Z_NULL;
	ret = inflateInit2(&stream, 16+MAX_WBITS);
	if (ret != Z_OK) return NULL;

//	stream.avail_in = strsubcpy(in, Data, 0, CHUNK, 
	int i, zOffset = 0, Offset = 0;
	do {
		for (i = 0; i < CHUNK && i < zSize; ++i) in[i] = Data[i+zOffset];
//		in[i+1] = '\0';
		stream.avail_in = i;
		stream.next_in = in;
		zOffset += i;
		do {
			stream.avail_out = CHUNK;
			stream.next_out = out;
			ret = inflate(&stream, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);
			have = CHUNK - stream.avail_out;

			if (FirstRun) {
				RetData = malloc(Offset+have);
				FirstRun = false;
			}
			else RetData = realloc(RetData, Offset+have);
			for (i = 0; i < have; ++i) RetData[i+Offset] = out[i];
			Offset += have;
		} while (stream.avail_out == 0);
	} while (ret != Z_STREAM_END); 

	*Size = Offset;
	RetData[Offset] = '\0';
	(void)inflateEnd(&stream);
	return RetData;
#else
	printf("This version of GekPkg cannot use zlib.\n");
	return NULL;
#endif
}

/* Code happily stolen from http://www.zlib.net/zlib_how.html 
 * with only a few minor edits. Mainly to add filename support. */
int InfFile2File(const char * FileName_In, const char * FileName_Out)
{
#ifdef USE_GZ
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* Open the files */
    FILE * source, * dest;
    if (!(source = fopen(FileName_In, "rb"))) return -1;
    dest	= fopen(FileName_Out, "wb");


    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, 16+MAX_WBITS);
    if (ret != Z_OK) {
	fclose(source);
	fclose(dest);
        return ret;
    }

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
	    fclose(source);
	    fclose(dest);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
		fclose(source);
		fclose(dest);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
		fclose(source);
		fclose(dest);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    fclose(source);
    fclose(dest);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
#else
	printf("This version of gekpkg cannot use zlib.\n");
	return -1;
#endif
}

