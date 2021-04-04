#include "ShaderData.h"
#include "Shader.h"
#include "Renderer.h"

void ShaderData::setupShader(Shader* s, RenderPass *r)
{
	// do shit here
	// set texture data (if needed && possible)
	int uLoc;
	int nTexture = texture.size();
	for (int i = 0; i < glm::min(nTexture, 4); i++) {
		uLoc = s->getUniformLocation(Shader::UniformLoc::texture0 + i);
		if (uLoc >= 0 && nTexture > i) {
			glActiveTexture(GL_TEXTURE0 + i);
			//glEnable(GL_TEXTURE_2D);
			texture[i]->bind();
			glUniform1i(uLoc, i);
		}
	}

	// set color data (if possible)
	uLoc = SU_LOC(s, material_diffuse);
	if (uLoc >= 0) {
		glUniform4fv(uLoc, 1, glm::value_ptr(diffuseColor));
	}

	uLoc = SU_LOC(s, material_specular);
	if (uLoc >= 0) {
		glUniform4fv(uLoc, 1, glm::value_ptr(specularColor));
	}

	uLoc = SU_LOC(s, material_emission);
	if (uLoc >= 0) {
		glUniform4fv(uLoc, 1, glm::value_ptr(emissionColor));
	}

	uLoc = SU_LOC(s, material_shininess);
	if (uLoc >= 0) {
		glUniform1f(uLoc, shininess);
	}
}
