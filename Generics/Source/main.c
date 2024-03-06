#include "core/memory.h"
#include "core/array.h"
#include "sourceParser.h"
#include "GenericExpansion.h"
#include "core/runtime.h"

private array(string) GetFiles(char* strs[], int count)
{
	array(string) result = arrays(string).Create(count);

	for (size_t i = 0; i < count; i++)
	{
		string path = strings.Create(strlen(strs[i]));
		strings.AppendCArray(path, strs[i], path->Capacity);

		arrays(string).Append(result, path);
	}

	return result;
}

void main(int argc, char* argv[])
{
	RunOnStartMethods();
	return;

	array(string) paths = GetFiles(argv, argc);

	for (size_t i = 1; i < paths->Count; i++)
	{
		fprintf(stdout, "%s\n\r", at(paths, i)->Values);
		array(location) tokens = GetGenericLocations(at(paths, i));
	}


	printf("");
}
