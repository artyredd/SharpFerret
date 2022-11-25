#include "modeling/model.h"
#include "singine/memory.h"

static Model CreateModel(void);
static void DisposeModel(Model model);

const struct _modelMethods Models = {
	.Create = &CreateModel,
	.Dispose = &DisposeModel
};

static void DisposeModel(Model model)
{
	if (model isnt null)
	{
		for (size_t i = 0; i < model->Count; i++)
		{
			Mesh tmp = model->Meshes[i];

			Meshes.Dispose(tmp);
		}

		Memory.Free(model->Meshes);

		Memory.Free(model->Name);
	}

	Memory.Free(model);
}

static Model CreateModel()
{
	return Memory.Alloc(sizeof(struct _model));
}
