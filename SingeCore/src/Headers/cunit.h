#pragma once

#include "csharp.h"
#include <stdio.h>
#include <time.h>

typedef struct _test* Test;

struct _test {
	/// <summary>
	/// The next test in the test suite
	/// </summary>
	Test Next;
	/// <summary>
	/// The name of the test
	/// </summary>
	char* Name;
	/// <summary>
	/// The method that should be ran
	/// </summary>
	bool(*Method)();
};

typedef struct testSuite* TestSuite;

struct testSuite {
	char* Name;
	Test Head;
	Test Tail;
	size_t Count;
	FILE* OutputStream;
	void(*Append)(TestSuite suite, char* name, bool(*method)(FILE*));
	void(*Dispose)(TestSuite suite);
	bool(*Run)(TestSuite suite);
};

TestSuite CreateSuite(char* name);

static const char* PassFormat = "\t[PASS] ";
static const char* FailFormat = "\t[FAIL] ";
static const char* FailExpectedFormat = "\t\tExpected: ";
static const char* FailActualFormat = "Got: ";
static const char* BenchmarkStartFormat = "\t[%s] "; // MethodName
static const char* BenchmarkSpeedFormat = "[%lli ticks] "; // Speed(ticks)
static const char* BenchmarkEndFormat = " in %s at %s [%li]" NEWLINE; // __func__, __FILE__, __LINE__
static const char* SuiteStartFormat = "[%s] [STARTING] [COUNT = %lli]" NEWLINE; // SuiteName
static const char* SuiteEndFormat = "[%s] [FINISHED] [PASS = %lli] [FAIL = %lli]"; // SuiteName
static const char* TestFinishedFormat = "[%s]"; // TestName

#define Benchmark(expression,stream) do\
{\
	size_t start = clock(); \
	expression;\
	size_t end = clock() - start; \
	fprintf(stream, BenchmarkSpeedFormat, end); \
} while (false);

#define BenchmarkAssertion(expression,stream) do\
{\
	size_t start = clock(); \
	bool pass = (expression); \
	size_t end = clock() - start; \
	if (pass)\
	{\
		fprintf(stream, PassFormat); \
	}\
	else\
	{\
		fprintf(stream, FailFormat); \
	}\
	fprintf(stream, BenchmarkSpeedFormat, end); \
	fprintf(stream, BenchmarkEndFormat,__func__,__FILE__,__LINE__); \
	if(pass is false)\
	{\
		return false;\
	}\
} while (false);

#define BenchmarkComparison(expected,actual,comparison,format,stream)do\
{\
	size_t start = clock(); \
	bool pass = (expected comparison actual); \
	size_t end = clock() - start; \
	if (pass)\
	{\
		fprintf(stream, PassFormat); \
	}\
	else\
	{\
		fprintf(stream, FailFormat); \
	}\
	fprintf(stream, BenchmarkSpeedFormat, end); \
	fprintf(stream, BenchmarkEndFormat,__func__,__FILE__,__LINE__); \
	if(pass is false)\
	{\
		fprintf(stream, FailExpectedFormat);\
		fprintf(stream,format,expected);\
		fputc(' ',stream);\
		fprintf(stream, FailActualFormat);\
		fprintf(stream,format,actual);\
		fprintf(stream,NEWLINE);\
		return false;\
	}\
} while (false);

#define Assert(expr) BenchmarkAssertion(expr,stream);
#define StandardAssert(expr) Assert(expr);

#define IsNull(expr) StandardAssert(expr == NULL);
#define IsNotNull(expr) StandardAssert(expr != NULL);
#define IsZero(expr) StandardAssert(expr == 0);
#define IsNotZero(expr) StandardAssert(expr != 0);
#define IsFalse(expr) StandardAssert(expr != true);
#define IsTrue(expr) StandardAssert(expr);
#define IsEqual(left,right,format) BenchmarkComparison(left,right,==,format,stream);
#define IsNotEqual(left,right) StandardAssert(left != right);