#include "core/memory.h"
#include "core/array.h"
#include "sourceParser.h"
#include "GenericExpansion.h"
#include "core/runtime.h"
#include "core/file.h"

private array(string) GetFiles(byte* strs[], int count)
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

void main(int argc, byte* argv[])
{
	RunOnStartMethods();

	array(string) paths = GetFiles(argv, argc);

	Assembly assembly = CreateAssembly();


	for (size_t i = 1; i < paths->Count; i++)
	{
		const string path = at(paths, i);
		fprintf(stdout, "%s\n\r", path->Values);

		array(location) locations = GetGenericLocations(path);

		string data = Files.ReadAll(path);

		CompileUnit unit = CreateCompileUnit(assembly, data);

		ExpandGenerics(data, locations, unit);
	}

	for (int i = 0; i < assembly->CompileUnits->Count; i++)
	{
		CompileUnit unit = at(assembly->CompileUnits, i);

		//string path = strings.Clone();

		Files.WriteAll(at(paths, i), unit->Data);
	}

	printf("");
}
