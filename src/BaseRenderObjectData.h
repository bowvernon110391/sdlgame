#pragma once
#include <glm/glm.hpp>

class Shader;
class AABB;
/// <summary>
/// Base interface for render object data
/// </summary>
class BaseRenderObjectData {
public:
	virtual ~BaseRenderObjectData() {}

	virtual bool getModelMatrix(glm::mat4& m) const { return false;  }
	virtual bool getScale(glm::vec3& s) const { return false;  }
	virtual bool getBoundingBox(AABB& bbox) const { return false; }
	virtual void setupData(const Shader* sh) const {}
	virtual BaseRenderObjectData* setLocalAABB(const AABB* bbox) { return this;  }
	virtual BaseRenderObjectData* updateBBox() { return this;  }
};