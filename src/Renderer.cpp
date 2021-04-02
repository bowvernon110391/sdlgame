#include "Renderer.h"
#include "Camera.h"
#include "SceneData.h"
#include "BaseRenderObject.h"
#include "BaseRenderObjectData.h"
#include "Mesh.h"
#include "Material.h"
#include "Shader.h"
#include "ShaderData.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

//#define DEBUG_VA

Renderer::Renderer() {
	m_camera = 0;
	m_scene = 0;
	lastAttribFlags = 0;
}

Renderer::~Renderer() {
	if (m_camera) {
		delete m_camera;
		m_camera = 0;
	}

	if (m_scene) {
		delete m_scene;
		m_scene = 0;
	}
}

Renderer* Renderer::useCamera(Camera* cam) {
	if (m_camera && cam) {
		delete m_camera;
	}
	m_camera = cam;
	return this;
}

Renderer* Renderer::useSceneData(SceneData* sd) {
	if (m_scene && sd) {
		delete m_scene;
	}
	m_scene = sd;
	return this;
}

Renderer* Renderer::setViewport(int x, int y, int w, int h) {
#ifdef _DEBUG
	SDL_Log("RENDERER_SET_VIEWPORT: %d %d %d %d\n", x, y, w, h);
#endif // _DEBUG

	viewport[0] = x;
	viewport[1] = y;
	viewport[2] = w;
	viewport[3] = h;

	if (m_camera)
		m_camera->setAspect((float)w / (float)h);

	return this;
}

void Renderer::setupVertexState(const Shader* shd) {
	for (int i = Shader::AttribLoc::position; i < Shader::AttribLoc::custom_attribute; i++) {
		if (shd->attributeFlags & (1 << i)) {
			glEnableVertexAttribArray(shd->getAttribLocation(i));
		}
	}
}

void Renderer::setPassData(const Shader* shd) {
	// start from viewport?
	int u_loc = SU_LOC(shd, viewport_dimension);
	if (u_loc >= 0) {
		glUniform4iv(u_loc, 1, &viewport[0]);
	}

	// if we have a camera
	if (m_camera) {
		// set projection
		u_loc = SU_LOC(shd, m_projection);
		if (u_loc >= 0) {
			glm::mat4 proj = m_camera->getProjectionMatrix();
			glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(proj));
		}
		// set view
		u_loc = SU_LOC(shd, m_view);
		if (u_loc >= 0) {
			glm::mat4 view = m_camera->getViewMatrix();
			glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(view));
		}
	}

	// if we have a scene data
	if (m_scene) {
		// set something?
		m_scene->setData(shd);
	}
}

/// <summary>
/// set shader data from a shader data (per material)
/// </summary>
/// <param name="shd"></param>
/// <param name="shdata"></param>
void Renderer::setMaterialData(const Shader* shd, const ShaderData* shdata)
{
	shd->setupData(shdata);
}

/// <summary>
/// set shader data from instance
/// </summary>
/// <param name="shd"></param>
/// <param name="instance"></param>
void Renderer::setInstanceData(const Shader* shd, const BaseRenderObjectData* instance)
{
	// set model and mvp, and scale if available
	glm::mat4 model, view, projection;
	glm::vec3 objScale;

	bool hasModelMatrix = instance->getModelMatrix(model);
	bool hasScale = instance->getScale(objScale);
	int u_loc = -1;

	// anything related to model matrix
	if (hasModelMatrix) {
		// set model matrix
		u_loc = SU_LOC(shd, m_model);
		if (u_loc >= 0) {
			glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(model));
		}
		// modelview + mvp + normal
		if (m_camera) {
			view = m_camera->getViewMatrix();
			projection = m_camera->getProjectionMatrix();

#ifdef DEBUG_INSTANCE_DATA
			SDL_Log("VIEW_MATRIX:\n");
			float* p = glm::value_ptr(view);
			for (int i = 0; i < 4; i++) {
				SDL_Log("\t%.4f %.4f %.4f %.4f\n", p[i], p[i + 4], p[i + 8], p[i + 12]);
			}

			SDL_Log("VIEW_MATRIX:\n");
			float* p = glm::value_ptr(view);
			for (int i = 0; i < 4; i++) {
				SDL_Log("\t%.4f %.4f %.4f %.4f\n", p[i], p[i + 4], p[i + 8], p[i + 12]);
			}
#endif // DEBUG_INSTANCE_DATA


			glm::mat4 modelView = view * model;

			// modelview
			u_loc = SU_LOC(shd, m_model_view);
			if (u_loc >= 0) {
				glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(modelView));
			}

			// normal matrix = 3x3 of modelview
			u_loc = SU_LOC(shd, m_normal);
			if (u_loc >= 0) {
				glm::mat3 normalMatrix = glm::mat3(modelView);
				glUniformMatrix3fv(u_loc, 1, false, glm::value_ptr(normalMatrix));
			}

			// modelviewprojection
			u_loc = SU_LOC(shd, m_model_view_projection);
			if (u_loc >= 0) {
				glm::mat4 mvp = projection * modelView;
				glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(mvp));
			}

		}
	}

	// scale?
	if (hasScale) {
		u_loc = SU_LOC(shd, scale);
		if (u_loc >= 0) {
			glUniform3fv(u_loc, 1, glm::value_ptr(objScale));
		}
	}

	// maybe it has a special setup? could override this one too
	instance->setupData(shd);
}

void Renderer::setupVertexArray(const Shader* shd, const Mesh* m)
{
#ifdef DEBUG_VA
	SDL_Log("VERTEX_ARRAY: shd_attrib_flags(%d), mesh_vtx_format(%d)\n", shd->attributeFlags, m->vertexFormat);
#endif // DEBUG_VA

	// now just call glVertexAttribArray on all possible shiets
	size_t offset = 0;
	int a_loc = -1;

	// gotta follow mesh vertex format
	// position
	a_loc = shd->getAttribLocation(Shader::AttribLoc::position);
	if ((m->vertexFormat & VF_XYZ) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: POSITION @ %d bytes\n", a_loc, offset);
#endif // DEBUG

		glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_XYZ)) * sizeof(float) * 3;

	// normal
	a_loc = shd->getAttribLocation(Shader::AttribLoc::normal);
	if ((m->vertexFormat & VF_NORMAL) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: NORMAL @ %d bytes\n", a_loc, offset);
#endif // DEBUG
		
		glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_NORMAL) >> 1) * sizeof(float) * 3;

	// uv
	a_loc = shd->getAttribLocation(Shader::AttribLoc::uv);
	if ((m->vertexFormat & VF_UV) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: UV @ %d bytes\n", a_loc, offset);
#endif // DEBUG

		glVertexAttribPointer(a_loc, 2, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_UV) >> 2) * sizeof(float) * 2;

	// tangent, bitangent
	if (m->vertexFormat & VF_TANGENT) {
		// tangent first
		a_loc = shd->getAttribLocation(Shader::AttribLoc::tangent);
		if (a_loc >= 0) {
#ifdef DEBUG_VA
			SDL_Log("VERTEX_ARRAY[%d]: TANGENT @ %d bytes\n", a_loc, offset);
#endif // DEBUG
			glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)offset);
		}

		// then bitangent
		a_loc = shd->getAttribLocation(Shader::AttribLoc::bitangent);
		if (a_loc >= 0) {
#ifdef DEBUG_VA
			SDL_Log("VERTEX_ARRAY[%d]: BITANGENT @ %d bytes\n", a_loc, offset+sizeof(float)*3);
#endif // DEBUG
			glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)(offset + sizeof(float) * 3));
		}
	}
	offset += ((m->vertexFormat & VF_TANGENT) >> 3) * sizeof(float) * 6;

	// second uv
	a_loc = shd->getAttribLocation(Shader::AttribLoc::uv2);
	if ((m->vertexFormat & VF_UV2) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: UV2 @ %d bytes\n", a_loc, offset);
#endif // DEBUG
		glVertexAttribPointer(a_loc, 2, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_UV2) >> 4) * sizeof(float) * 2;
}

void Renderer::draw(const std::vector<BaseRenderObject*>& data) {
	// update scene data? and camera too ofc
	if (m_scene && m_camera) {
		m_scene->updateLightsUniformData(m_camera->getPosition());
	}
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	// prepare pass state?
	// first, gotta build a queue, then sort them all?
	// nope, just iterate shit
	auto it = data.begin();
	while (it != data.end()) {
		BaseRenderObject* obj = *it;
		// draw it, for every submesh it has
		this->draw(*obj);

		// increment
		++it;
	}
}

/// <summary>
/// draw a single object?
/// </summary>
/// <param name="ro"></param>
void Renderer::draw(const BaseRenderObject& ro) {
	// for every submeshes draw call it
	const BaseRenderObjectData* rod = ro.data;
	
	
	//SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "GL_ERROR: CODE = %d\n", glGetError());
	//SDL_assert(glGetError() != GL_INVALID_OPERATION);

	for (int i = 0; i < ro.mesh->subMeshes.size(); i++) {
		// setup shader per submeshes
		const Shader* sh = ro.mat->mats[i]->sh;
		const ShaderData* shd = ro.mat->mats[i]->shData;
		const Mesh::SubMesh* mesh = &ro.mesh->subMeshes[i];

		// bind shader
		sh->bind();
		//SDL_assert(glGetError() == GL_NO_ERROR);

		// bind buffer
		ro.mesh->bind();

		// enable vertex attributes based on shader attrib flags
		setupVertexState(sh);
		//SDL_assert(glGetError() == GL_NO_ERROR);
		// set pointer too
		setupVertexArray(sh, ro.mesh);
		//SDL_assert(glGetError() == GL_NO_ERROR);
		
		// set per-pass data
		this->setPassData(sh);
		//SDL_assert(glGetError() == GL_NO_ERROR);
		// TODO: GRAB ALL SCENE AND CAMERA DATA, INJECT SHADER USING THEM
		// set per-material data
		this->setMaterialData(sh, shd);
		//SDL_assert(glGetError() == GL_NO_ERROR);
		// set per-object data
		// TODO: GRAB RENDER OBJECT DATA, INJECT SHADER USING THEM
		this->setInstanceData(sh, rod);
		//SDL_assert(glGetError() == GL_NO_ERROR);

		// draw call
		// TODO: JUST CALL GLDRAWELEMENTS HERE, ALL HARD JOB IS DONE
		glDrawElements(GL_TRIANGLES, mesh->elemCount, GL_UNSIGNED_SHORT, (const void*)mesh->idxBegin);
		//SDL_assert(glGetError() == GL_NO_ERROR);
	}
}