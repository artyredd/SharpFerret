#include "core/memory.h"
#include "core/array.h"
#include "sourceParser.h"

private array(string) GetFiles(char* strs[], int count)
{
	array(string) result = Arrays(string).Create(count);

	for (size_t i = 0; i < count; i++)
	{
		string path = strings.Create(strlen(strs[i]));
		strings.AppendCArray(path, strs[i], path->Capacity);

		Arrays(string).Append(result, path);
	}

	return result;
}

void main(int argc, char* argv[])
{
	RunTests();
	return;

	array(string) paths = GetFiles(argv, argc);

	for (size_t i = 1; i < paths->Count; i++)
	{
		fprintf(stdout, "%s\n\r", paths->Values[i]->Values);
		array(string) tokens = ReadTokens(paths->Values[i]);
	}


	printf("");
}
