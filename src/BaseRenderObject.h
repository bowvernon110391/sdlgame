#pragma once

class Mesh;
class BaseRenderObjectData;
class MaterialSet;

/// <summary>
/// base render object (mesh + render obj data + matset)
/// </summary>
class BaseRenderObject {
public:
	BaseRenderObject(Mesh* m, BaseRenderObjectData* d, MaterialSet* s) {
		mesh = m;
		data = d;
		mat = s;
	}

	virtual ~BaseRenderObject() {
		// only own the data
		delete data;
	}

	Mesh* mesh;
	BaseRenderObjectData* data;
	MaterialSet* mat;
};