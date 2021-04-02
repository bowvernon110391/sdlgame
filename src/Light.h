#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Light {
public:

	Light* setPosition(const glm::vec3& pos) { position = pos; return this; }
	Light* setRadius(const float& r) { radius = r; return this; }
	Light* setDiffuse(const glm::vec4& v) { diffuseColor = v; return this; }
	Light* setSpecular(const glm::vec4& v) { specularColor = v; return this; }
	Light* setAttenuation(const glm::vec4& v) { attenuation = v; return this; }

	float distanceCost(const glm::vec3& pos, float kRadiusMultiplier = 1.0f) const {
		glm::vec3 ab = pos - position;
		// the further, the bigger the cost
		// the bigger the radius, the smaller the cost
		return glm::dot(ab, ab) + 1.0f / (kRadiusMultiplier * radius + 0.1f);
	}

	glm::vec3 position;
	float radius;	// is it used?
	glm::vec4 diffuseColor, specularColor;
	glm::vec4 attenuation;
};
