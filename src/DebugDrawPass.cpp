#include "DebugDrawPass.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"

void DebugDrawPass::prepare(float dt)
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void DebugDrawPass::draw(float dt)
{
	if (!renderer->drawDebug)
		return;

	// shared by all
	glm::mat4 view = renderer->getCamera()->getViewMatrix();
	glm::mat4 proj = renderer->getCamera()->getProjectionMatrix();

	glm::mat4 mvp = proj * view;

	// generate commands, and draw them?
	Shader* s = renderer->getDebugShader();
	s->bind();

	// set uniform first
	int u_diffuse = s->getUniformLocation(Shader::UniformLoc::material_diffuse);
	glUniform4fv(u_diffuse, 1, glm::value_ptr(renderer->debugColor));
	int u_mvp = s->getUniformLocation(Shader::UniformLoc::m_model_view_projection);
	glUniformMatrix4fv(u_mvp, 1, false, glm::value_ptr(mvp));

	int vbo = renderer->getDebugVBO();
	int ibo = renderer->getDebugIBO();

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	int a_loc = s->getAttribLocation(Shader::AttribLoc::position);
	glEnableVertexAttribArray(a_loc);
	glVertexAttribPointer(a_loc, 3, GL_FLOAT, GL_FALSE, 12, (void*)0);

	// iterate all over aabb, bind and generate the data
	//unitCube->bind();
	//Renderer::setupVertexArray(s, unitCube);

	//SDL_Log("DEBUGDRAW (%d) aabb...mvp(%d) diffuse(%d) a_pos(%d)\n", bboxes.size(), u_mvp, u_diffuse, a_loc);
	
	for (const AABB &b : bboxes) {
		renderer->generateDebugData(b);

		// draw call?
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, (void*)0);
	}

}

void DebugDrawPass::processRenderObject(AbstractRenderObject* o)
{
	// get its bounding box, pass into renderables?
	o->getDebugAABB(bboxes);
}

void DebugDrawPass::generateDebugString()
{
}
