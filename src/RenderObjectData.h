#pragma once
#include "BaseRenderObjectData.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "AABB.h"

//#define _DEBUG_RENDER_OBJECT_DATA

// this is basic render object data, with pos, rot and scale
class RenderObjectData : public BaseRenderObjectData {
public:
	RenderObjectData() {
		scale = glm::vec3(1, 1, 1);
		pos = glm::vec3(0, 0, 0);
		rot = glm::quat(1,0,0,0);
		bbox = nullptr;
	}

	virtual RenderObjectData* setLocalAABB(const AABB* b) {
		this->bbox = (AABB*) b;
		return this;
	}

	RenderObjectData* usePosition(const glm::vec3& p) { pos = p; return this; }
	RenderObjectData* useScale(const glm::vec3& s) { scale = s; return this; }
	RenderObjectData* useRotation(const glm::quat& r) { rot = r; return this; }
	
	virtual RenderObjectData* updateBBox() {
		if (!bbox) return this;
		// get rotated matrix, do computation there?
		glm::mat4 m = translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot);
		
		// okay, generate 8 vertices, and using those, we iterate over to compute extent
		const glm::vec3& min = bbox->min;
		const glm::vec3& max = bbox->max;

		glm::vec3 verts[] = {
			glm::vec3( m * glm::vec4(min, 1.0f)),
			glm::vec3( m * glm::vec4( glm::vec3(max.x, min.y, min.z) , 1.0f)),
			glm::vec3( m * glm::vec4( glm::vec3(min.x, max.y, min.z) , 1.0f)),
			glm::vec3( m * glm::vec4( glm::vec3(max.x, max.y, min.z) , 1.0f)),

			glm::vec3( m * glm::vec4(max, 1.0f)),
			glm::vec3(m * glm::vec4(glm::vec3(max.x, min.y, max.z) , 1.0f)),
			glm::vec3(m * glm::vec4(glm::vec3(min.x, max.y, max.z) , 1.0f)),
			glm::vec3(m * glm::vec4(glm::vec3(min.x, min.y, max.z) , 1.0f)),
		};

		// iterate over those, recording extent
		bool first = true;

		float dp;
		for (auto v : verts) {
			// record lowest
			if (first || v.x < wb.min.x) wb.min.x = v.x;
			if (first || v.y < wb.min.y) wb.min.y = v.y;
			if (first || v.z < wb.min.z) wb.min.z = v.z;

			// record highest
			if (first || v.x > wb.max.x) wb.max.x = v.x;
			if (first || v.y > wb.max.y) wb.max.y = v.y;
			if (first || v.z > wb.max.z) wb.max.z = v.z;

			first = false;
		}

		return this;
	}

	virtual bool getModelMatrix(glm::mat4 &m) const {
		// rotate + translate
		m = glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot);
		//m = glm::mat4(rot) * m;

#ifdef _DEBUG_RENDER_OBJECT_DATA
		const float* p = glm::value_ptr(m);
		SDL_Log("RENDER_OBJECT_DATA_MODEL pos(%.4f, %.4f, %.4f) rot(%.4f, %.4f, %.4f, %.4f):\n", 
			pos.x, pos.y, pos.z,
			rot.x, rot.y, rot.z, rot.w);
		for (int i = 0; i < 4; i++) {
			SDL_Log("\t%.4f %.4f %.4f %.4f\n", p[i], p[i+4], p[i+8], p[i+12]);
		}
#endif // _DEBUG


		return true;
	}

	virtual bool getScale(glm::vec3& s) const {
		s = scale;
		return true;
	}

	virtual bool getBoundingBox(AABB& b) const {
		// now compute a bounding box for this rotated one?
		b = wb;
		return true;
	}

	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
	AABB *bbox, wb;	// the reference bbox and our instantiated bounding box (worldspace)
};