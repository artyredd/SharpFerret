#pragma once

#ifndef _csharp_h_
#define _csharp_h_
#endif // !_csharp_h_

#include <stdbool.h>
#include <intrin.h>
#include "exceptions.h"

#define null NULL
#define true 1
#define false 0
#define is ==

#ifdef _WIN32
#define NEWLINE "\r\n"
#elif defined macintosh // OS 9
#define NEWLINE "\r"
#else
#define NEWLINE "\n" // Mac OS X uses \n
#endif

// prints the provided exception to stdout and exits the application
// see https://docs.microsoft.com/en-us/cpp/intrinsics/debugbreak?view=msvc-170
#ifdef __INTRIN_H_
#define throw(errorCode) \
	fprintf(stdout,"%s in %s() %s\tat %s [%li]%s",#errorCode,__func__,NEWLINE,__FILE__,__LINE__,NEWLINE);\
	__debugbreak();
#else

#define throw(errorCode) \
	fprintf(stdout,"%s in %s() %s\tat %s [%li]%s",#errorCode,__func__,NEWLINE,__FILE__,__LINE__,NEWLINE);\
	exit(errorCode);

#endif

#define warn(errorCode) fprintf(stdout,"[WARNING] %s in %s() %s\tat %s [%li]%s",#errorCode,__func__,NEWLINE,__FILE__,__LINE__,NEWLINE);