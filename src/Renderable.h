#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "ShaderData.h"
#include "AbstractRenderObject.h"

class Renderable {
public:
	Renderable() {
		m = nullptr;
		sm = nullptr;
		s = nullptr;
		sd = nullptr;
		ro = nullptr;
	}

	Mesh* m;
	SubMesh* sm;
	Shader* s;
	ShaderData* sd;
	AbstractRenderObject* ro;
};
