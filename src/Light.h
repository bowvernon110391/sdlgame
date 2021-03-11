#pragma once

#include "glm/glm.hpp"

class Light {
public:

	Light() {
		pos = glm::vec3();
		color = glm::vec3(1, 1, 1);
		attenuation = glm::vec3(0.5f, 0.0025f, 0.25f);
	}

	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 attenuation;
};
