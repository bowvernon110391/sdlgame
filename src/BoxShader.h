#pragma once

#include "Shader.h"
#include "Light.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

namespace GShader {
	class BoxShader : public ::Shader {
	public:
		typedef enum {
			AMBIENT_COLOR = 0,
			LIGHT0_POS,
			LIGHT1_POS,
			LIGHT0_COLOR,
			LIGHT1_COLOR,
			LIGHT0_ATTENUATION,
			LIGHT1_ATTENUATION,

			MAT_MVP,
			MAT_MODELVIEW,
			SCALE,

			TEXTURE0
		} UniformLoc;

		// store light infos
		//Light lights[2];	// only 2 atm
		glm::vec3 ambientColor;

		virtual void prepareState() {
			glEnable(GL_TEXTURE_2D);
		}

		virtual void setUniformLocs() {
			this->pushUniformLocation("ambientColor", UniformLoc::AMBIENT_COLOR);
			this->pushUniformLocation("lightPos[0]", UniformLoc::LIGHT0_POS);
			this->pushUniformLocation("lightColor[0]", UniformLoc::LIGHT0_COLOR);
			this->pushUniformLocation("lightAttenuation[0]", UniformLoc::LIGHT0_ATTENUATION);this->pushUniformLocation("lightPos[0]", UniformLoc::LIGHT0_POS);
			this->pushUniformLocation("lightPos[1]", UniformLoc::LIGHT0_POS+1);
			this->pushUniformLocation("lightColor[1]", UniformLoc::LIGHT0_COLOR+1);
			this->pushUniformLocation("lightAttenuation[1]", UniformLoc::LIGHT0_ATTENUATION+1);

			this->pushUniformLocation("matMVP", UniformLoc::MAT_MVP);
			this->pushUniformLocation("matModelview", UniformLoc::MAT_MODELVIEW);
			this->pushUniformLocation("scale", UniformLoc::SCALE);

			this->pushUniformLocation("texture0", UniformLoc::TEXTURE0);
		}

		void setAmbientColor() {
			glUniform3f(this->getUniformLocation(UniformLoc::AMBIENT_COLOR), ambientColor.x, ambientColor.y, ambientColor.z);
		}

		// set texture
		void setTextureData(Texture2D* tex) {
			glActiveTexture(GL_TEXTURE0);
			tex->use();
			glUniform1i(this->getUniformLocation(UniformLoc::TEXTURE0), 0);
		}

		// set light data
		void setLightingData(int idx, const Light& l, const glm::mat4& view) {
			// update certain light data
			if (idx < 2) {
				glm::vec4 viewPos = view * glm::vec4(l.pos, 1.0f);

				// directly set that shit
				glUniform3f(this->getUniformLocation(UniformLoc::LIGHT0_POS + idx), viewPos.x, viewPos.y, viewPos.z);
				glUniform3f(this->getUniformLocation(UniformLoc::LIGHT0_COLOR + idx), l.color.x, l.color.y, l.color.z);
				glUniform3f(this->getUniformLocation(UniformLoc::LIGHT0_ATTENUATION + idx), l.attenuation.x, l.attenuation.y, l.attenuation.z);
			}
		}

		// called before drawing to set some data
		void setTransformData(const glm::mat4& mvp, const glm::mat4& modelView, const glm::vec3& scale) {
			// set matrices
			glUniformMatrix4fv(this->getUniformLocation(UniformLoc::MAT_MVP), 1, false, glm::value_ptr(mvp));
			glUniformMatrix4fv(this->getUniformLocation(UniformLoc::MAT_MODELVIEW), 1, false, glm::value_ptr(modelView));
			glUniform3f(this->getUniformLocation(UniformLoc::SCALE), scale.x, scale.y, scale.z);
		}
	};
}