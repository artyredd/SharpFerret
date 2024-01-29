#pragma once

#ifndef _csharp_h_
#define _csharp_h_
#endif // !_csharp_h_

#include <stdbool.h>
#include <intrin.h>
#include "core/exceptions.h"
#include <stdio.h>
#include <stdlib.h>

#define null NULL
#define true 1
#define false 0
#define is ==
#define isnt !=
#define or ||
#define and &&

// expression that provides either left or right, whichever isn't value, if both are value returns value
#define Coalesce(left,right,operator,value) (left operator value ? left : right)

// expression that provides either left or right, whichever isn't null, if both are null returns null
// equivalent to left ?? right
#define NullCoalesce(left,right) Coalesce(left,right,isnt,null) 

// equivalent to left ??= right;
#define NullCoalesceAssign(left,right) if(left is null) {left = right}

#ifdef _WIN32
#define NEWLINE "\r\n"
#elif defined macintosh // OS 9
#define NEWLINE "\r"
#else
#define NEWLINE "\n" // Mac OS X uses \n
#endif

#define THROW_SHOULD_BREAK

#ifdef THROW_SHOULD_BREAK 
#ifdef __INTRIN_H_
#define ThrowEvent(errorCode) __debugbreak();
#else
#define ThrowEvent(errorCode) exit(errorCode);
#endif
#else
#define ThrowEvent(errorCode) exit(errorCode);
#endif

#define throw(errorCode) \
	fprintf(stdout,"%s in %s() %s\tat %s [%li]%s",#errorCode,__func__,NEWLINE,__FILE__,__LINE__,NEWLINE);\
	ThrowEvent(errorCode);

#define warn(errorCode) fprintf(stdout,"[WARNING] %s in %s() %s\tat %s [%li]%s",#errorCode,__func__,NEWLINE,__FILE__,__LINE__,NEWLINE);

// provides the string literal of the provided object
#define nameof(thing) #thing

// Ignores the provided parameter explicitly, avoiding any warnings generated from the compiler
#define ignore_unused( parameter ) (void)parameter;

#define safe_increment( uint ) ( uint = max(uint, (uint + 1) ) )
#define safe_decrement( uint ) ( uint = min(uint, (uint - 1) ) )

#define safe_add( uint, value ) ( max(uint, (uint + value) ) )
#define safe_subtract( uint, value ) ( min(uint, (uint - value) ) )

#define private static inline

#define struct_cast(destinationType) *(destinationType*)& 