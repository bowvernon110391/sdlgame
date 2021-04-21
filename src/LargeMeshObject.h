#pragma once

#include "AbstractRenderObject.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class LargeMesh;
class MaterialSet;

class LargeMeshObject : public AbstractRenderObject {
public:
	LargeMeshObject(LargeMesh* m, MaterialSet* ms);
	virtual ~LargeMeshObject();

	// Inherited via AbstractRenderObject
	virtual void setupData(Shader* s, RenderPass* rp) override;
	virtual void update(float dt) override;
	virtual void preRender(float dt) override;
	virtual void fillRenderable(std::vector<Renderable>& bucket) override;
	virtual void getDebugAABB(std::vector<AABB>& bboxes) override;
	// Inherited via AbstractRenderObject
	virtual AABB getBoundingBox() const override;

	// some data
	LargeMesh* lm;
	MaterialSet* ms;

	int active_mesh;
	bool debug_draw;

protected:
	glm::vec3 pos;
	glm::quat rot;
};