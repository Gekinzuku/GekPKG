#include <string.h>
#include <stdlib.h>
/* Kind of the opposite of strstr(), also doesn't edit the passed in char array.
NOTE: Remember to free the returned value or you'll get a memory leak! 
There is probably a better way to do this.
*/
char * strsub(char * haystack, char needle);
char * strtrim(char * String, char Remove);
char * strsubcpy (char * Dest, char * Src, int Start, int End, short AddTerm, int TermChar);

