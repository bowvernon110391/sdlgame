#pragma once
#include "Mesh.h"
#include "BaseRenderObjectData.h"

class MaterialSet;
class AABB;

/// <summary>
/// base render object (mesh + render obj data + matset)
/// </summary>
class BaseRenderObject {
public:
	BaseRenderObject(Mesh* m, BaseRenderObjectData* d, MaterialSet* s) {
		mesh = m;
		data = d;
		mat = s;

		d->setLocalAABB(&m->boundingBox)
			->updateBBox();
	}

	virtual ~BaseRenderObject() {
		// only own the data
		delete data;
	}

	Mesh* mesh;
	BaseRenderObjectData* data;
	MaterialSet* mat;
};