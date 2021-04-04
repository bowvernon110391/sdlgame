#pragma once
#include <vector>
#include <assert.h>

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
	AbstractRenderObject(Mesh*  m, MaterialSet*  ms)
		:m(m), ms(ms)
	{
		assert(ms != nullptr);
		assert(ms != nullptr);
	}
	// pure virtual
	virtual void setupData(Shader* s, RenderPass* rp) = 0;
	virtual void update(float dt) = 0;
	virtual void preRender(float dt) = 0;
	virtual void fillRenderable(std::vector<Renderable> &bucket) = 0;

	// some basic?
	virtual AbstractRenderObject* setFlags(int f) {
		flags = f;
		return this;
	}


	// at the base, it must have at least 
	Mesh* m;
	MaterialSet* ms;	// array of pair of shader + constant(and texture)
	// just a marker
	int flags;
};