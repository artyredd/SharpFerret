#pragma once

#include "core/csharp.h"
#include <stdio.h>
#include <time.h>
#include "core/array.h"

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
	void(*Method)();
};

typedef struct testSuite* TestSuite;

struct testSuite {
	char* Name;
	Test Head;
	Test Tail;
	ulong Count;
	FILE* OutputStream;
	void(*Append)(TestSuite suite, char* name, bool(*method)(FILE*));
	void(*Dispose)(TestSuite suite);
	void(*Run)(TestSuite suite);
};

TestSuite CreateSuite(char* name);

static const char* PassFormat = "\t[PASS] ";
static const char* FailFormat = "\t[FAIL] ";
static const char* FailExpectedFormat = "\t\tExpected: ";
static const char* FailActualFormat = "Got: ";
static const char* BenchmarkStartFormat = "\t[%s] "; // MethodName
static const char* BenchmarkSpeedFormat = "[%lli ticks] "; // Speed(ticks)
static const char* BenchmarkEndFormat = " in %s at %s [%li]"NEWLINE; // __func__, __FILE__, __LINE__
static const char* SuiteStartFormat = "[%s] [STARTING] [COUNT = %lli]"NEWLINE; // SuiteName
static const char* SuiteEndFormat = "[%s] [FINISHED] "; // SuiteName
static const char* TestFinishedFormat = "[%s]"; // TestName

#define Benchmark(expression,stream) do\
{\
	ulong start = clock(); \
	expression;\
	ulong end = clock() - start; \
	fprintf(stream, BenchmarkSpeedFormat, end); \
} while (false);

#define BenchmarkAssertion(expression,stream) do\
{\
	ulong start = clock(); \
	bool pass = (expression); \
	ulong end = clock() - start; \
	if (pass)\
	{\
		fprintf(stream,_FORMAT_COLOR_GREEN_START"%s"_FORMAT_COLOR_GREEN_END, PassFormat); \
	}\
	else\
	{\
		fprintf(stream, _FORMAT_COLOR_RED_START"%s"_FORMAT_COLOR_RED_END, FailFormat); \
	}\
	fprintf(stream, BenchmarkSpeedFormat, end); \
	fprintf(stream, BenchmarkEndFormat,__func__,__FILE__,__LINE__); \
} while (false);

#define BenchmarkComparison(expected,actual,comparison,format,stream)do\
{\
	ulong start = clock(); \
	bool pass = (expected comparison actual); \
	ulong end = clock() - start; \
	if (pass)\
	{\
		fprintf(stream,_FORMAT_COLOR_GREEN_START"%s"_FORMAT_COLOR_GREEN_END, PassFormat); \
	}\
	else\
	{\
		fprintf(stream, _FORMAT_COLOR_RED_START"%s"_FORMAT_COLOR_RED_END, FailFormat); \
	}\
	fprintf(stream, BenchmarkSpeedFormat, end); \
	fprintf(stream, BenchmarkEndFormat,__func__,__FILE__,__LINE__); \
	if(!pass)\
	{\
		fprintf(stream,"\t\t"#expected" "#comparison" "#actual NEWLINE);\
		fprintf(stream, _FORMAT_COLOR_RED_START"%s"_FORMAT_COLOR_RED_END, FailExpectedFormat);\
		fprintf(stream,format,expected);\
		fputc(' ',stream);\
		fprintf(stream, _FORMAT_COLOR_RED_START"%s"_FORMAT_COLOR_RED_END, FailActualFormat);\
		fprintf(stream,format,actual);\
		fprintf(stream,NEWLINE);\
	}\
} while (false);

#define TEST(testname) private bool Test_##testname(FILE* __test_stream)
#define APPEND_TEST(testname) suite->Append(suite, #testname, &Test_##testname);
#define TEST_SUITE(suiteName,tests)private void suiteName(void)\
{\
	TestSuite suite = CreateSuite(#suiteName);\
	tests\
	suite->Run(suite);\
	suite->Dispose(suite);\
}

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

#define __IsTypeof(value,type) _Generic((value),type:"1",default:"")
#define IsTypeof(value,type) (sizeof(__IsTypeof(value,type))-1)

#define Assert(expr) BenchmarkAssertion(expr,__test_stream);
#define StandardAssert(expr) Assert(expr,stdout);

#define IsNull(expr) BenchmarkAssertion(expr == NULL,__test_stream);
#define NotNull(expr) BenchmarkAssertion(expr != NULL,__test_stream);
#define IsZero(expr) BenchmarkAssertion(expr == 0,__test_stream);
#define IsNotZero(expr) BenchmarkAssertion(expr != 0,__test_stream);
#define IsFalse(expr) BenchmarkAssertion(expr != true,__test_stream);
#define IsTrue(expr) BenchmarkAssertion(expr,__test_stream);
#define IsEqual(expected,actual) BenchmarkComparison(expected,actual,==,FormatCType(expected),__test_stream);
#define IsNotEqual(left,right) BenchmarkAssertion(left != right,__test_stream);

#define IsApproximate(left,right) BenchmarkComparison(((left < 0 ? (-left) : (left)) - (right < 0 ? (-right) : (right))),1e-15,<=,FormatCType(left),__test_stream);

private int _IndexWhereDifferent(string left, string right)
{
	for (int i = 0; i < min(left->Count, right->Count); i++)
	{
		const byte leftC = at(left, i);
		const byte rightC = at(right, i);
		if (leftC isnt rightC)
		{
			return i;
		}
	}
	return min(left->Count, right->Count);
}

private bool _IsStringEqual(void* stream, string left, string right)
{
	bool equal = strings.Equals(left, right);
	if (equal is false)
	{
		if (left isnt null and right isnt null)
		{
			const int differentIndex = _IndexWhereDifferent(left, right);
			string arrowLine = dynamic_array(byte, differentIndex + 1);
			arrowLine->Count = differentIndex + 1;
			strings.Fill(arrowLine, '-');
			at(arrowLine, differentIndex) = '|';

			fprintf_red(stream, " Expected: %s"NEWLINE, left->Values);
			fprintf_red(stream, "          %s"NEWLINE, arrowLine->Values);
			fprintf_red(stream, "Actual:   %s"NEWLINE, right->Values);
		}
		else
		{
			fprintf_red(stream, "Expected was%s", left ? "nt null" : " null"NEWLINE);
			fprintf_red(stream, "Actual   was%s", right ? "nt null" : " null"NEWLINE);
		}
	}
	return true;
}

#define IsStringEqual(expected,actual) IsTrue(_IsStringEqual(__test_stream, expected,actual))

#define IsTupleEqual(leftTuple,rightTuple) IsEqual(leftTuple.First,rightTuple.First); IsEqual(leftTuple.Second,rightTuple.Second)