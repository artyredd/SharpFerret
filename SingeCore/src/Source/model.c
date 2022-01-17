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
	if (model->Head != null)
	{
		Mesh head = model->Head;
		while (head != null)
		{
			Mesh tmp = head;

			head = head->Next;

			SafeFree(tmp->Vertices);
			SafeFree(tmp->TextureVertices);
			SafeFree(tmp->Normals);
			SafeFree(tmp->Name);

			SafeFree(tmp);
		}
	}

	SafeFree(model->Name);

	SafeFree(model);
}

static Model CreateModel()
{
	return SafeAlloc(sizeof(struct _model));
}
