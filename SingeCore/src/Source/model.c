#include "modeling/model.h"
#include "singine/memory.h"

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

	SafeFree(model);
}

Model CreateModel()
{
	Model model = SafeAlloc(sizeof(struct _model));

	model->Head = model->Tail = null;
	model->Name = null;
	model->Dispose = &DisposeModel;
	model->Count = 0;

	return model;
}