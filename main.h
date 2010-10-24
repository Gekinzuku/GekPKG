/* Options */
#define USE_GZ
//#define USE_BZ2

/* Enable debug messages and whatnot */
#define DEBUG

#define VERSION "GekPkg v0.0.1A. - Copyright James Danielson (GeekyLink)\n\
A simple, minimalist, and fast package manager.\n\
Built with:%s\n\
This program is licensed under the GNU General Public License Version 3.\n\
Visit http://gekinzuku.com for details\n"

#define TRUE 1
#define FALSE 0
/* Convience for C++ coders like myself */
#define true TRUE
#define false FALSE

/* String based defines for errors or help or whatev */
#define HELP_STR "Usage: pkgman <action> [package name]\n\
Operations:\n\
	pkgman add packagename.tar.pkg\n\
	pkgman remove packagename\n\
	pkgman info packagename\n\
Extras:\n\
	pkgman {--version, -v}\n"

#define ERROR_BADFILE_STR "This package appears to either be corrupt or does not exist.\n"
#define ERROR_BADPKG_STR "This package appears to be incorrectly made.\n"
#define ERROR_NOTROOT_STR "Sorry, this action can only be executed as root.\n"
#define ERROR_PKG_UNSUPPORTED_STR "This package is either using an invalid file format or this version of pkgman was not built to support it.\n"

/* Separate defines for modes */
#define MODE_NONE	0
#define MODE_INFO	1
#define MODE_ADD	2
#define MODE_VERSION	3

/* Supported extension types */
#define EXT_INVALID	-1
#define EXT_NONE	0
#define EXT_GZ 		1



/* Important global variables */
extern unsigned short Mode; 

struct PackageData
{
	char * FileName;
	char * Name;
	char * Version;
	char * Summary;
	char * Description;
	int Type;
};

#define PKGTYPE_BIN	0
#define PKGTYPE_SRC	1

extern struct PackageData Package;

