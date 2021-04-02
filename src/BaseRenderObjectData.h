#pragma once
#include <glm/glm.hpp>

class Shader;
/// <summary>
/// Base interface for render object data
/// </summary>
class BaseRenderObjectData {
public:
	virtual ~BaseRenderObjectData() {}

	virtual bool getModelMatrix(glm::mat4& m) const { return false;  }
	virtual bool getScale(glm::vec3& s) const { return false;  }
	virtual void setupData(const Shader* sh) const {}
};