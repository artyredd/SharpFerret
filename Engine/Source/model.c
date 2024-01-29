#include "engine/modeling/model.h"
#include "core/memory.h"

static Model CreateModel(void);
static void DisposeModel(Model model);

const struct _modelMethods Models = {
	.Create = &CreateModel,
	.Dispose = &DisposeModel
};

TYPE_ID(ModelMeshes);
TYPE_ID(Model);

static void DisposeModel(Model model)
{
	if (model isnt null)
	{
		for (size_t i = 0; i < model->Count; i++)
		{
			Mesh tmp = model->Meshes[i];

			Meshes.Dispose(tmp);
		}

		Memory.Free(model->Meshes, ModelMeshesTypeId);

		Memory.Free(model->Name, Memory.String);
	}

	Memory.Free(model, ModelTypeId);
}

static Model CreateModel()
{
	Memory.RegisterTypeName(nameof(Model), &ModelTypeId);
	Memory.RegisterTypeName("ModelMeshes", &ModelMeshesTypeId);

	return Memory.Alloc(sizeof(struct _model), ModelTypeId);
}
