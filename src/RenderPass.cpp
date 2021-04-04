#include "RenderPass.h"
#include "Shader.h"
#include "Renderer.h"
#include "Camera.h"
#include "SceneData.h"

void RenderPass::setupData(const Shader* s)
{
	// start from viewport?
	int u_loc = SU_LOC(s, viewport_dimension);
	if (u_loc >= 0) {
		glUniform4iv(u_loc, 1, &renderer->viewport[0]);
	}

	// if we have a camera
	Camera* cam = renderer->m_camera;

	assert(cam != nullptr);

	if (cam) {
		// set projection
		u_loc = SU_LOC(s, m_projection);
		if (u_loc >= 0) {
			glm::mat4 proj = cam->getProjectionMatrix();
			glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(proj));
		}
		// set view
		u_loc = SU_LOC(s, m_view);
		if (u_loc >= 0) {
			glm::mat4 view = cam->getViewMatrix();
			glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(view));
		}
	}

	// if we have a scene data
	SceneData* sce = renderer->m_scene;
	if (sce) {
		// set scene data into shader
		sce->setupData(s, this);
	}
}
