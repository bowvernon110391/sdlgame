#pragma once
#include <vector>
#include <assert.h>
#include "AABB.h"
#include "render_enums.h"

class Shader;
class Mesh;
class MaterialSet;
class Renderable;
class RenderPass;

/// <summary>
/// This is a pure abstract render object, with only couple of responsibility, which are:
/// - SETUP SHADER DATA
/// - UPDATE
/// - PRE-RENDER UPDATE
/// - FILL RENDERABLE
/// </summary>
class AbstractRenderObject {
public:
	// pure virtual
	virtual void setupData(Shader* s, RenderPass* rp) = 0;
	virtual void update(float dt) = 0;
	virtual void preRender(float dt) = 0;
	virtual void fillRenderable(std::vector<Renderable> &bucket)  = 0;
	// used by debug pass (fill one or more aabbs)
	virtual void getDebugAABB(std::vector<AABB> &bboxes) {}
	virtual AABB getBoundingBox() const = 0;

	// some basic?
	virtual AbstractRenderObject* setFlags(int f) {
		flags = f;
		return this;
	}
	// just a marker
	int flags;
};