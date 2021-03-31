#pragma once
#include "Shader.h"
#include "Texture2d.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

namespace GShader {
	class LightmapShader : public ::Shader {
	public:
		typedef enum {
			MAT_MVP,
			TEXTURE0,
			TEXTURE1
		} UniformLoc;

		virtual void prepareState() {
			// enable texture 2d on both texture unit
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);

			glActiveTexture(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
		}

		virtual void setUniformLocs() {
			this->pushUniformLocation("texture0", UniformLoc::TEXTURE0);
			this->pushUniformLocation("texture1", UniformLoc::TEXTURE1);
			this->pushUniformLocation("matMVP", UniformLoc::MAT_MVP);
		}

		void setTexture0(Texture2D* tex) {
			glActiveTexture(GL_TEXTURE0);
			tex->use();
			glUniform1i(this->getUniformLocation(UniformLoc::TEXTURE0), 0);
		}

		void setTexture1(Texture2D* tex) {
			glActiveTexture(GL_TEXTURE1);
			tex->use();
			glUniform1i(this->getUniformLocation(UniformLoc::TEXTURE1), 1);
		}

		void setTransformData(const glm::mat4& mvp) {
			// set matrix data
			glUniformMatrix4fv(this->getUniformLocation(UniformLoc::MAT_MVP), 1, false, glm::value_ptr(mvp));
		}
	};
}