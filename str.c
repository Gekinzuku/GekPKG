#include "str.h"

char * strsub(char * haystack, char needle)
{
	int i;
	for (i = 0; i < strlen(haystack); ++i) /* Search for the character */
		if (haystack[i] == needle) break;

	char * result = (char*)malloc(i); /* Allocate needed memory, terminate the array, and return it */
	result[i] = '\0';
	for (i -= 1; i >= 0; --i) result[i] = haystack[i];
	return result;
}

char * strtrim(char * String, char Remove)
{
	int i, StartPoint = -1, EndPoint = -1, Offset;
	/* Find the first character that isn't the Remove one */
	for (i = 0; i < strlen(String) && StartPoint == -1 ; ++i) {
		if (String[i] != Remove) StartPoint = i;
	}
	Offset = i-1;
	/* Find the last character that isn't the Remove one */
	for (i = strlen(String)-1; i > StartPoint-1 && i > 0 && EndPoint == -1; --i) {
		if (String[i] != Remove) EndPoint = i+1;
	}

	/* If one or both didn't need to be trimmed, use the end of the extremes of the string instead */
	if (StartPoint == -1) StartPoint = 0;
	if (EndPoint == -1) EndPoint = strlen(String)-1;

	/* Shift the characters over and add a new end point */
	for (i = StartPoint; i < EndPoint; ++i) String[i-Offset] = String[i];
	String[i-Offset] = '\0';

	return String;
}

/* Copies a chunk of a string to another. */
char * strsubcpy (char * Dest, char * Src, int Start, int End, short AddTerm, int TermChar)
{
	int i;
	if (TermChar < 0) {
		for (i = Start; i < End; ++i) Dest[i-Start] = Src[i];
	}
	else {
		for (i = Start; i < End && Src[i] != TermChar; ++i) Dest[i-Start] = Src[i];
	}

	if (AddTerm) Dest[i] = '\0';
	return Dest;
}

