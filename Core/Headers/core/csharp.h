#pragma once

#ifndef _csharp_h_
#define _csharp_h_
#endif // !_csharp_h_

#include <stdbool.h>
#include <intrin.h>
#include "core/exceptions.h"
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef size_t ulong;
typedef unsigned int uint;

#define null NULL
#define true 1
#define false 0
#define is ==
#define isnt !=
#define or ||
#define and &&
#define not !

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
#define ThrowEvent(errorCode) __debugbreak()
#else
#define ThrowEvent(errorCode) exit(errorCode)
#endif
#else
#define ThrowEvent(errorCode) exit(errorCode)
#endif

#define _FORMAT_COLOR_RED_START "\x1b[31m " // Color Start
#define _FORMAT_COLOR_RED_END " \x1b[0m" // To flush out prev settings
#define _FORMAT_COLOR_GREEN_START "\x1b[32m " // Color Start
#define _FORMAT_COLOR_GREEN_END _FORMAT_COLOR_RED_END // To flush out prev settings
#define _FORMAT_COLOR_BLUE_START "\x1b[34m "
#define _FORMAT_COLOR_BLUE_END _FORMAT_COLOR_RED_END
#define _FORMAT_COLOR_YELLOW_START "\x1b[33m "
#define _FORMAT_COLOR_YELLOW_END _FORMAT_COLOR_RED_END


#define _EXPAND(thing) thing
// fprintf but it outputs red text
#define fprintf_red(file, format, ...) fprintf(file,_FORMAT_COLOR_RED_START format _FORMAT_COLOR_RED_END, __VA_ARGS__)
#define fprintf_blue(file, format, ...) fprintf(file,_FORMAT_COLOR_BLUE_START format _FORMAT_COLOR_BLUE_END, __VA_ARGS__)
#define fprintf_yellow(file, format, ...) fprintf(file,_FORMAT_COLOR_YELLOW_START format _FORMAT_COLOR_YELLOW_END, __VA_ARGS__)
#define fprintf_green(file, format, ...) fprintf(file,_FORMAT_COLOR_GREEN_START format _FORMAT_COLOR_GREEN_END, __VA_ARGS__)

static int BreakDebugger(int errorCode)
{
	ThrowEvent(errorCode);
	return errorCode;
}

#define throw(errorCode) \
	fprintf_red(stdout,"%s in %s() %s\tat %s [%li]%s",#errorCode,__func__,NEWLINE,__FILE__,__LINE__,NEWLINE);\
	ThrowEvent(errorCode)

#define _CAT(left,right) left##right

// use this when you want to throw an error in the middle of an expression like
// example: int x = y < 0 ? throw_in_expression(IndexOutOfRangeException) : y;
#define throw_in_expression(errorCode) \
	fprintf(stdout,#errorCode"\n\r\tat "__FILE__) + BreakDebugger(errorCode)

#define warn(errorCode) fprintf(stdout,"[WARNING] %s in %s() %s\tat %s [%li]%s",#errorCode,__func__,NEWLINE,__FILE__,__LINE__,NEWLINE);

// provides the string literal of the provided object
#define nameof(thing) #thing

// Ignores the provided parameter explicitly, avoiding any warnings generated from the compiler
#define ignore_unused( parameter ) (void)parameter;

#define safe_increment( unsigned_integer ) ( unsigned_integer = max(unsigned_integer, (unsigned_integer + 1) ) )
#define safe_decrement( unsigned_integer ) ( unsigned_integer = min(unsigned_integer, (unsigned_integer - 1) ) )

#define safe_add( unsigned_integer, value ) ( max(unsigned_integer, (unsigned_integer + value) ) )
#define safe_subtract( unsigned_integer, value ) ( min(unsigned_integer, (unsigned_integer - value) ) )

#define private inline static

// DO NOT USE, DANGY, WILL HURT CPU FEELINGS >:(, ONLY USE WHEN YOU KNOW IT WONT HURT CPU FEELING
#define struct_cast(destinationType) *(destinationType*)& 

#define __IsTypeof(value,type) _Generic((value),type:"1",default:"")
#define _IsTypeof(value,type) (sizeof(__IsTypeof(value,type))-1)
#define IsTypeof(value,type) _IsTypeof(value,type)

#define FormatCType(type) _Generic((type), \
    char: "%c", \
    unsigned char: "%c", \
    short: "%hd", \
    unsigned short: "%hu", \
    int: "%d", \
    unsigned int: "%u", \
    long: "%ld", \
    unsigned long: "%lu", \
    long long: "%lld", \
    unsigned long long: "%llu", \
    float: "%f", \
    double: "%lf", \
    long double: "%Lf", \
    const char*: "%s", \
	char*: "%s",\
    default: "UnsupportedType" \
)

static int _intrinsicTypeIds[] = {
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
	__COUNTER__,
};

#define typeof(t) _Generic((t), \
	default: 0\
)