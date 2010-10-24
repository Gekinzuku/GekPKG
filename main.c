#include "compression.h"
#include "str.h"
/* Set this to true and the package manager will actually add things to / 
 * Don't enable for alpha unless you know what you are doing.
 * Don't say I didn't warn you! */
int DangerMode = false; 

unsigned short Mode = MODE_NONE;
struct PackageData Package;

/* Functions */
int GetFlags(int argc, char * argv[]); /* Converts command line flags to variables and settings */
int ParsePackage(char * data); /* Load the package info into the Package struct */
int LoadPackage();

int main(int argc, char * argv[])
{
	if (!GetFlags(argc, argv)) return 1;

	/* Find info on a package in the current directory */
	if (Mode == MODE_ADD) {
		//if (geteuid() != 0) {
		//	printf(ERROR_NOTROOT_STR);
		//	return 1;
		//}
		int Result = LoadPackage();
		if (Result != 0) return Result;

		int i;
		if (Package.Type == PKGTYPE_BIN) { 
			ExtractLoadedTar("");
		}
		else if (Package.Type == PKGTYPE_SRC) {

		}
	}
	if (Mode == MODE_INFO) {
		int Result = LoadPackage();
		if (Result != 0) return Result;

		printf("%d: %s (%d)\n", TarFileCount, tFile[2].Name, tFile[2].Mode);
		printf("%s: (%s) %s\n%s\n%s\n", Package.Name, (Package.Type == PKGTYPE_BIN) ? "Binary" : "Source", Package.Version, Package.Summary, Package.Description);

		/* Clean up */
		free(Package.Name);
		free(Package.Version);
		free(Package.Summary);
		free(Package.Description);
		FreeUpTar();
	} /* Prints some details about the package manager. */
	else if (Mode == MODE_VERSION) {
		char * Libs = NULL;
	
		/* Gets each library version */
		#ifdef USE_GZ
			Libs = realloc(Libs, strlen(" zlib: ") + strlen(zlibVersion())); 
			strcat(Libs, " zlib: ");
			strcat(Libs, zlibVersion());
		#endif
		#ifdef USE_BZ2
			Libs = realloc(Libs, strlen(" bzlib: ") + strlen(BZ2_bzlibVersion())); 
			strcat(Libs, " bzlib: ");
			strcat(Libs, BZ2_bzlibVersion());
		#endif

		/* If no libraries are being used then simply say so */
		if (Libs == NULL) {
			#define NO_LIBS " No external libraries."
			Libs = malloc(strlen(NO_LIBS));
			strcat(Libs, NO_LIBS);
			#undef NO_LIBS
		}
		
		printf(VERSION, Libs);
		//free(Libs);
	}

	return 0;
}

int GetFlags(int argc, char * argv[])
{
	short Success = true;
	if (argc < 2) {
		printf(HELP_STR);
		Success = false;
	}
	else
	{
		if (strcmp(argv[1], "help") == 0) {
			printf(HELP_STR);
			Success = false;
		}
		else if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) Mode = MODE_VERSION;
		if (strcmp(argv[1], "info") == 0 && argc > 2) {
			Mode = MODE_INFO;
			Package.FileName = (char*)malloc(strlen(argv[2]));
			strcpy(Package.FileName,argv[2]);	
		}
		if (strcmp(argv[1], "add") == 0 && argc > 2) {
			Mode = MODE_ADD;
			Package.FileName = (char*)malloc(strlen(argv[2]));
			strcpy(Package.FileName,argv[2]);	
		}
	}
	return Success;
}

int ParsePackage(char * data)
{
	char * tmp = strtok(data,"\n"); /* Go line by line through the file */
	int Works = true;
	while (tmp != NULL)
	{
		/* We first trim and get the current line's info type, then use it to determine where to save the data */
		strtrim(tmp, ' ');
		char * Type = strsub(tmp,':');
		if (strcmp(Type, "Package") == 0) { /* Save the package name */
			char * Info = strstr(tmp, " ");
			Package.Name = (char*)malloc(strlen(Info));
			int i; /* Copy the info into the correct field, but ignore the first space */
			strtrim(Info, ' ');
			for (i = 0; i < strlen(Info); ++i) Package.Name[i] = Info[i];
			Package.Name[i] = '\0';
		}
		else if (strcmp(Type, "Version") == 0) { /* Save the package version */
			char * Info = strstr(tmp, " ");
			Package.Version = (char*)malloc(strlen(Info));
			int i; /* Copy the info into the correct field, but ignore the first space */
			strtrim(Info, ' ');
			for (i = 0; i < strlen(Info); ++i) Package.Version[i] = Info[i];
			Package.Version[i] = '\0';
		}
		else if (strcmp(Type, "Summary") == 0) { /* Save the package summary */
			char * Info = strstr(tmp, " ");
			Package.Summary = (char*)malloc(strlen(Info));
			int i; /* Copy the info into the correct field, but ignore the first space */
			strtrim(Info, ' ');
			for (i = 0; i < strlen(Info); ++i) Package.Summary[i] = Info[i];
			Package.Summary[i] = '\0';
		}
		else if (strcmp(Type, "Description") == 0) { /* Save the package summary */
			char * Info = strstr(tmp, " ");
			Package.Description = (char*)malloc(strlen(Info));
			int i; /* Copy the info into the correct field, but ignore the first space */ 
			strtrim(Info, ' ');
			for (i = 0; i < strlen(Info); ++i) Package.Description[i] = Info[i];
			Package.Description[i] = '\0';
		}
		else if (strcmp(Type, "Type") == 0) { /* Save the package summary */
			char * Info = strstr(tmp, " ");
			strtrim(Info, ' ');
			if (strcmp(Info, "Binary") == 0) Package.Type = PKGTYPE_BIN;
			else if (strcmp(Info, "Source") == 0) Package.Type = PKGTYPE_SRC;
			else {
				Works = false;
				break;
			}
		}
		else {
			free(Type);
			Works = false;
			break;
		}
		tmp = strtok (NULL, "\n");
		free(Type); /* We have to free up mem from our strsub() call */
	}
	/* Free up no longer needed mem */
	return Works;
}

int LoadPackage()
{
	char * Data;
	int Size, i, ExtType = EXT_INVALID;

	/* Load the gz file */
	Data = InfFile2Char(Package.FileName, Data, &Size);
	if (Data == NULL) {
		printf(ERROR_BADFILE_STR);
		return 1;
	}
	/* Now, let's load the tar, free the Data, then use that to load the info on the package */
	LoadTarFromChar(Data, Size); 
	free(Data);

	if (TarFileCount != 2) {
		printf(ERROR_BADPKG_STR);
		return 1;
	}

	ExtType = EXT_INVALID;
	for (i = 0; i < TarFileCount; ++i) {
		if (strcmp(tFile[i].Name, "info") == 0) { ExtType = EXT_NONE; break; }
		if (strcmp(tFile[i].Name, "info.gz") == 0) { 
		#ifdef USE_GZ
			ExtType = EXT_GZ; 
		#endif
			break; 
		}
	}
	if (i == TarFileCount) {
		printf(ERROR_BADPKG_STR);
		return 1;
	}

	if (ExtType == EXT_GZ) Data = gzInfFromChar(tFile[i].Data, tFile[i].DataLength, &Size);
	else if (ExtType == EXT_NONE) {
		Data = malloc(tFile[i].DataLength);
		strsubcpy(Data, tFile[i].Data, 0, tFile[i].DataLength, true, -1); 
	}
	else {
		printf(ERROR_PKG_UNSUPPORTED_STR);
		return 1;
	}

	/* Let's parse the package now, make sure it went OK. */
	if (ParsePackage(Data) == false) {
		printf(ERROR_BADPKG_STR);
		return 1;
	}

	/* After freeing up Data, let's load the actual files into it */
	free(Data);
	ExtType = EXT_INVALID;
	for (i = 0; i < TarFileCount; ++i) {
		if (strcmp(tFile[i].Name, "data.tar") == 0) { ExtType = EXT_NONE; break; }
		if (strcmp(tFile[i].Name, "data.tar.gz") == 0) { 
		#ifdef USE_GZ
			ExtType = EXT_GZ; 
		#endif
			break; 
		}
	}

	if (ExtType == EXT_GZ) Data = gzInfFromChar(tFile[i].Data, tFile[i].DataLength, &Size); 
	else if (ExtType == EXT_NONE) { 
		Data = malloc(tFile[i].DataLength);
		strsubcpy(Data, tFile[i].Data, 0, tFile[i].DataLength, false, -1); 
		Size = tFile[i].DataLength;
	} 
	else {
		printf(ERROR_PKG_UNSUPPORTED_STR);
		return 1;
	}
	/* Load the actual files from the tar, remove the old tar data though first cause we don't need it. */
	FreeUpTar();
	LoadTarFromChar(Data, Size);
	free(Data);
	return 0;
}

