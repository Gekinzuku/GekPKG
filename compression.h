#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "main.h"

/* Compression libraries */
#ifdef USE_GZ
	#include <zlib.h>
#endif
#ifdef USE_BZ2
	#include <bzlib.h>
#endif

#define CHUNK 16384

int ExtractFileFromTar(char * Prefix, int FileID);
int ExtractLoadedTar();
int LoadTarFromFile(char * FileName);
int LoadTarFromChar(char * FileName, int Size);
void FreeUpTar();

char * InfFile2Char(const char * FileName_In, char *dest, int * Size);
char * gzInfFromChar(char * Data, int zSize, int * Size);
int InfFile2File(const char * FileName_In, const char * FileName_Out);

/* Documention on tar format from: http://www.gnu.org/software/automake/manual/tar/Standard.html */
struct RawTarHeader
{                               /* Byte offset - End */
  char name[100];               /*   0 - 99 */
  char mode[8];                 /* 100 - 107 */
  char uid[8];                  /* 108 - 115 */
  char gid[8];                  /* 116 - 123 */
  char size[12];                /* 124 - 135 */
  char mtime[12];               /* 136 - 147 */
  char chksum[8];               /* 148 - 155 */
  char typeflag;                /* 156 - 156 */
  char linkname[100];           /* 157 - 256 */
  char magic[6];                /* 257 - 262 */
  char version[2];              /* 263 - 264 */
  char uname[32];               /* 265 - 296 */
  char gname[32];               /* 297 - 328 */
  char devmajor[8];             /* 329 - 336 */
  char devminor[8];             /* 337 - 344 */
  char prefix[155];             /* 345 - 499 */
  char padding[12];             /* 500 - 512 */
};

/* Values used in typeflag field.  */
#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */


/* We'll use this as an array of the files in the tar */
struct TarFile
{
	char Name[255];
	int Mode;
	int DataLength;
	char Type;
	char * Data;
};

extern struct TarFile * tFile;
extern int TarFileCount; /* Keeps track of how many elements there are */
