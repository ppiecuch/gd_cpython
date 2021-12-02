
#include "Python.h"
#include "osdefs.h"
#include <sys/types.h>
#include <string.h>

extern const char *Py_GetProgramName(void);

static int pathCalculated = 0; 
static char progPath[MAXPATHLEN+1];
static char modulePathes[1024+1];
static char execPrefixPath[MAXPATHLEN+1];

#ifndef HAVE_STPCPY
#ifdef _MSC_VER
# define __restrict__
#endif
char *stpcpy(char *__restrict__ dest, const char *__restrict__ src)
{
	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return --dest;
}
#endif

static void calcPathes()
{
	if(pathCalculated) return;

	char* p = stpcpy(progPath, Py_GetProgramName());
	while(--p > progPath) {
		if(*p == '/') {
			*p = 0;
			break;
		}
	}

	snprintf(modulePathes, 1024, "%s%c/pylib/lib%cpylib.zip%cres://pylib.zip%cres://%c.", progPath, DELIM, DELIM, DELIM, DELIM, DELIM);
	snprintf(execPrefixPath, MAXPATHLEN, "%s/pylib/exec", progPath);
	pathCalculated = 1;
}

/* External interface */

char *
Py_GetPath(void)
{
	calcPathes();
	return modulePathes;
}

char *
Py_GetPrefix(void)
{
	calcPathes();
	return "/pylib";
}

char *
Py_GetExecPrefix(void)
{
	calcPathes();
	return execPrefixPath;
}

char *
Py_GetProgramFullPath(void)
{
	calcPathes();
	return progPath;
}
