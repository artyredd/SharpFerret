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

		SafeFree(model->Meshes);

		SafeFree(model->Name);
	}

	SafeFree(model);
}

static Model CreateModel()
{
	return SafeAlloc(sizeof(struct _model));
}
