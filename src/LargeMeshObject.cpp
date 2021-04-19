#include "LargeMeshObject.h"
#include "LargeMesh.h"
#include "Material.h"
#include "Shader.h"
#include "Renderer.h"
#include "Camera.h"
#include "Renderable.h"

LargeMeshObject::LargeMeshObject(LargeMesh* m, MaterialSet* ms): lm(m), ms(ms)
{
	active_mesh = 0;
	pos = glm::vec3(0.0f);
	rot = glm::quat();
	debug_draw = false;
}

LargeMeshObject::~LargeMeshObject()
{
}

void LargeMeshObject::setupData(Shader* s, RenderPass* rp)
{
	// grab viewprojection, and generate mvp
	Camera* cam = rp->renderer->getCamera();
	int u_loc;

	// for now just set matrices and scale data
	assert(cam != nullptr);

	if (cam) {
#ifdef _DEBUG_DRAW
		SDL_Log("LARGEMESHOBJECT: SETUP DATA!! \n");
#endif
		// grab our model matrix
		glm::mat4 model = glm::mat4(1.0f); //glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot);
		glm::mat4 view = cam->getViewMatrix();
		glm::mat4 projection = cam->getProjectionMatrix();

		// does it have model?
		u_loc = SU_LOC(s, m_model);
		if (u_loc >= 0) {
#ifdef _DEBUG_DRAW
			SDL_Log("LARGEMESHOBJECT: SETUP Model Matrix!! \n");
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
	}
}

void LargeMeshObject::update(float dt)
{
}

void LargeMeshObject::preRender(float dt)
{
	// clamp in between
}

void LargeMeshObject::fillRenderable(std::vector<Renderable>& bucket)
{
	// make sure the amount of materials >= submeshes
	assert(ms->mats.size() >= lm->submesh_per_mesh);

	// just draw active mesh? only in debug mode
	if (debug_draw) {

		Renderable r;

		Mesh* m = lm->meshes[active_mesh];
		for (int i = 0; i < m->subMeshes.size(); i++) {
			Renderable r;
			r.m = m;
			r.sm = &m->subMeshes[i];
			r.ro = this;
			r.s = ms->mats[i]->sh;
			r.sd = ms->mats[i]->shData;

			bucket.push_back(r);
		}
	}
	else {
		// just iterate all over the meshes, and push them to the bucket
		for (Mesh* m : lm->meshes) {
			// push all submeshes too?
			for (int i = 0; i < m->subMeshes.size(); i++) {
				Renderable r;

				r.m = m;
				r.sm = &m->subMeshes[i];
				r.s = ms->mats[i]->sh;
				r.sd = ms->mats[i]->shData;
				r.ro = this;

				bucket.push_back(r);
			}
		}
	}	
}
