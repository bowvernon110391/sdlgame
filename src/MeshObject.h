#pragma once
#include "AbstractRenderObject.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class MeshObject : public AbstractRenderObject {
public:
	// init base data
	MeshObject(Mesh * m, MaterialSet * ms): 
		AbstractRenderObject(m, ms) {

		assert(m != nullptr);
		assert(ms != nullptr);

		pos = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
		rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	}
	// Inherited via AbstractRenderObject
	virtual void setupData(Shader* s, RenderPass* rp) override;
	virtual void update(float dt) override;
	virtual void preRender(float dt) override;
	virtual void fillRenderable(std::vector<Renderable>& bucket) override;

	MeshObject* setPosition(const glm::vec3& p) { pos = p; return this; }
	MeshObject* setRotation(const glm::quat& q) { rot = q; return this; }
	MeshObject* setScale(const glm::vec3& s) { scale = s; return this; }
protected:
	// has position, rotation and scale
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
};
