#include "MeshObject.h"
#include "RenderPass.h"
#include "Camera.h"
#include "Renderable.h"
#include "Material.h"
#include "Renderer.h"

//#define _DEBUG_DRAW

void MeshObject::setupData(Shader* s, RenderPass* rp)
{
	// grab viewprojection, and generate mvp
	Camera* cam = rp->renderer->getCamera();
	int u_loc;

	// for now just set matrices and scale data
	assert(cam != nullptr);

	if (cam) {
#ifdef _DEBUG_DRAW
		SDL_Log("MESHOBJECT: SETUP DATA!! \n");
#endif
		// grab our model matrix
		glm::mat4 model = glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot);
		glm::mat4 view = cam->getViewMatrix();
		glm::mat4 projection = cam->getProjectionMatrix();

		// does it have model?
		u_loc = SU_LOC(s, m_model);
		if (u_loc >= 0) {
#ifdef _DEBUG_DRAW
			SDL_Log("MESHOBJECT: SETUP Model Matrix!! \n");
			float* ptr = glm::value_ptr(model);
			for (int i = 0; i < 4; i++) {
				SDL_Log("\t%.4f %.4f %.4f %.4f\n", ptr[i], ptr[i + 4], ptr[i + 8], ptr[12]);
			}
#endif
			glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(model));
		}

		// maybe model view?
		glm::mat4 modelView = view * model;
		u_loc = SU_LOC(s, m_model_view);
		if (u_loc >= 0) {
			glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(modelView));
		}

		// maybe normal matrix?
		glm::mat3 m_normal = glm::mat3(modelView);
		u_loc = SU_LOC(s, m_normal);
		if (u_loc >= 0) {
			glUniformMatrix3fv(u_loc, 1, false, glm::value_ptr(m_normal));
		}

		// does it have mvp?
		u_loc = SU_LOC(s, m_model_view_projection);
		if (u_loc >= 0) {
			glm::mat4 modelViewProjection = projection * modelView;
			// compute mvp
			glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(modelViewProjection));
		}

		// does it have scale?
		u_loc = SU_LOC(s, scale);
		if (u_loc >= 0) {
			glUniform3fv(u_loc, 1, glm::value_ptr(scale));
		}
	}
}

void MeshObject::update(float dt)
{
	// do what here?
}

void MeshObject::preRender(float dt)
{
	// and do what here?
}

void MeshObject::fillRenderable(std::vector<Renderable>& bucket)
{
	// make sure material set and submeshes are adequate, otherwise, throw error?
	assert(m->subMeshes.size() <= ms->mats.size());

	// just fill the bucket with renderables, consist of some submesh combinations
	for (int i = 0; i < m->subMeshes.size(); i++) {
		Material* mat = ms->mats[i];

		assert(mat != nullptr);
		assert(mat->sh != nullptr);
		assert(mat->shData != nullptr);

		Renderable r;

		r.m = m;
		r.sm = &m->subMeshes[i];
		r.s = mat->sh;
		r.sd = mat->shData;
		r.ro = this;

		// push
		bucket.push_back(r);
	}
}
