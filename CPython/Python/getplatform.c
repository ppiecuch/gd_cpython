
#include "Python.h"

#ifndef PLATFORM
#if defined(__ANDROID__)
#define PLATFORM "Android"
#elif defined(__APPLE__)
#define PLATFORM "Darwin (Apple)"
#elif defined(__linux__)
#define PLATFORM "Linux"
#elif defined(_WIN32) || defined(WIN32)
#define PLATFORM "Windows"
#else
#define PLATFORM "Unknown"
#endif
#endif

const char *
Py_GetPlatform(void)
{
	return PLATFORM;
}
