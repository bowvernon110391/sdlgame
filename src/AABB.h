#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class AABB {
public: 
	AABB() {
		min = glm::vec3(0.0f);
		max = glm::vec3(0.0f);
	}

	AABB(const glm::vec3& bMin, const glm::vec3& bMax)
		: min(bMin), max(bMax) {
		// nothing
	}

	// expand bounded area
	void expand(const AABB& b) {
		min = glm::min(min, b.min);
		max = glm::max(max, b.max);
	}

	// expand by vector
	void expand(const glm::vec3& v) {
		if (v.x > 0) max.x += v.x; else min.x += v.x;
		if (v.y > 0) max.y += v.y; else min.y += v.y;
		if (v.z > 0) max.z += v.z; else min.z += v.z;
	}

	// expand by a constant value (fatten it)
	void expand(float f) {
		min += glm::vec3(-f, -f, -f);
		max += glm::vec3(f, f, f);
	}

	// surface area of AABB
	float area() {
		glm::vec3 d = max - min;
		return 2.0f * (d.x * d.y + d.y * d.z + d.x * d.z);
	}

	// return union of two aabb
	static AABB combined(const AABB& a, const AABB& b) {
		return AABB(
			glm::min(a.min, b.min),
			glm::max(a.max, b.max)
		);
	}

	// boolean check
	static bool intersect(const AABB& a, const AABB& b) {
		return (
			a.min.x < b.max.x && 
			a.min.y < b.max.y && 
			a.min.z < b.max.z
			&&
			b.min.x < a.max.x && 
			b.min.y < a.max.y && 
			b.min.z < a.max.z
			);
	}

	glm::vec3 min, max;
};