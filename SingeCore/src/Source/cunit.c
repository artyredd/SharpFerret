#include "cunit.h"
#include <stdlib.h>
#include "singine/memory.h"

static bool RunTest(Test test, FILE* stream);
static Test CreateTest(char* name, bool(*Method)(FILE*));

static bool RunSuite(TestSuite suite);
static void Append(TestSuite suite, char* name, bool(*method)(FILE*));
static void Dispose(TestSuite suite);

TestSuite CreateSuite(char* name)
{
	if (name is null)
	{
		fprintf(stderr, "parameter: name, must be specified");
		throw(InvalidArgumentException);
	}

	TestSuite suite = SafeAlloc(sizeof(struct testSuite));

	suite->Name = name;
	suite->Count = 0;
	suite->OutputStream = stdout;

	suite->Head = suite->Tail = null;

	suite->Dispose = &Dispose;
	suite->Append = &Append;
	suite->Run = &RunSuite;

	return suite;
}

static bool RunSuite(TestSuite suite)
{
	if (suite is null)
	{
		throw(InvalidArgumentException);
	}

	if (suite->Name is null)
	{
		throw(InvalidArgumentException);
	}

	fprintf(suite->OutputStream, SuiteStartFormat, suite->Name, suite->Count);

	Test head = suite->Head;

	size_t passCount = 0;

	Benchmark(
		while (head != null)
		{
			fprintf(suite->OutputStream, "[%s]\n", head->Name);

			if (RunTest(head, suite->OutputStream))
			{
				++passCount;
			}

			head = head->Next;
		}
	fprintf(suite->OutputStream, SuiteEndFormat, suite->Name, passCount, suite->Count - passCount);

	, suite->OutputStream);

	fprintf(suite->OutputStream, "%c", '\n\n');

	return passCount >= suite->Count;
}

static void Append(TestSuite suite, char* name, bool(*method)(FILE*))
{
	if (suite is null)
	{
		throw(InvalidArgumentException);
	}

	Test newTest = CreateTest(name, method);

	++(suite->Count);

	if (suite->Head is null)
	{
		suite->Head = suite->Tail = newTest;
		return;
	}

	suite->Tail->Next = newTest;
	suite->Tail = newTest;
}

static void Dispose(TestSuite suite)
{
	if (suite is null)
	{
		throw(InvalidArgumentException);
	}

	Test head = suite->Head;

	while (head != null)
	{
		Test tmp = head;

		head = head->Next;

		SafeFree(tmp);
	}

	SafeFree(suite);
}

static bool RunTest(Test test, FILE* stream)
{
	bool pass = false;

	Benchmark(

		pass = test->Method(stream);
	fprintf(stream, "[%s][%s] ", pass ? "PASS" : "FAIL", test->Name);

	, stream);

	fputc('\n', stream);

	return pass;
}

static Test CreateTest(char* name, bool(*Method)(FILE*))
{
	Test newTest = SafeAlloc(sizeof(struct _test));

	newTest->Name = name;
	newTest->Method = Method;
	newTest->Next = null;

	return newTest;
}