#ifndef __DEBUG_LIGHT_SHADER__
#define __DEBUG_LIGHT_SHADER__

#include "Shader.h"
#include "Light.h"
#include "../glm/glm/ext.hpp"

namespace GShader {
	class DebugLightShader : public ::Shader {
	public:
		typedef enum {
			LIGHT_COLOR,
			MAT_MVP,
			LIGHT_RADIUS
		} UniformLoc;

		virtual void prepareState() {
			// bind texture and set states, that kind of stuffs
			glDisable(GL_TEXTURE_2D);
		}

		virtual void setUniformLocs() {
			// called after loaded
			this->pushUniformLocation("lightColor", UniformLoc::LIGHT_COLOR);
			this->pushUniformLocation("matMVP", UniformLoc::MAT_MVP);
			this->pushUniformLocation("lightRadius", UniformLoc::LIGHT_RADIUS);
		}

		void setDrawData(const Light& l, const glm::mat4& view, const glm::mat4& proj) {
			// construct mvp too
			glm::mat4 model = glm::translate(glm::mat4(1.0f), l.pos);
			glm::mat4 mvp = proj * view * model;

			/*SDL_Log("Model Matrix data:");
			float* m = glm::value_ptr(model);
			for (int i = 0; i < 4; i++) {
				SDL_Log("%.2f %.2f %.2f %.2f", m[0 + i], m[4 + i], m[8 + i], m[12 + i]);
			}*/

			static int x = 0;
			if (x++ < 1) {
				SDL_Log("LightShader info: %d %d",
					this->getUniformLocation(UniformLoc::MAT_MVP),
					this->getUniformLocation(UniformLoc::LIGHT_COLOR)
				);
			}

			// set mvp
			glUniformMatrix4fv(this->getUniformLocation(UniformLoc::MAT_MVP), 1, false, glm::value_ptr(mvp));

			// set radius?
			// nope, just color
			glUniform3f(this->getUniformLocation(UniformLoc::LIGHT_COLOR), l.color.r, l.color.g, l.color.b);
		}
	};
}

#endif