#pragma once
#include "BaseRenderObjectData.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//#define _DEBUG_RENDER_OBJECT_DATA

// this is basic render object data, with pos, rot and scale
class RenderObjectData : public BaseRenderObjectData {
public:
	RenderObjectData() {
		scale = glm::vec3(1, 1, 1);
		pos = glm::vec3(0, 0, 0);
		rot = glm::quat(1,0,0,0);
	}

	RenderObjectData* usePosition(const glm::vec3& p) { pos = p; return this; }
	RenderObjectData* useScale(const glm::vec3& s) { scale = s; return this; }
	RenderObjectData* useRotation(const glm::quat& r) { rot = r; return this; }

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


	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
};