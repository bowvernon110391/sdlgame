
#include "RenderCommand.h"

#include "Renderer.h"
#include "RenderPass.h"
#include "Shader.h"
#include "Mesh.h"
#include "ShaderData.h"
#include "AbstractRenderObject.h"
#include <stdio.h>

void RC_BindShader::operator()()
{
	s->bind();
	r->setupData(s);

	// also setup vertex state, don't forget
	Renderer::setupVertexState(s);
}

void RC_BindShader::debug(char* const str) {
	sprintf(str, "<RC>: In RenderPass %d Bind Shader %d\n", r->priority, s->id);
}

void RC_BindShaderData::operator()()
{
	sd->setupShader(s, r);
}

void RC_BindShaderData::debug(char* const str)
{
	sprintf(str, "<RC>: In RenderPass %d Bind Shader data %d to Shader %d\n", r->priority, sd->id, s->id);
}

void RC_BindShaderDataFromObject::operator()()
{
	o->setupData(s, r);
}

void RC_BindShaderDataFromObject::debug(char* const str)
{
	sprintf(str, "<RC>: In RenderPass %d Bind Object data @ 0x%X to Shader %d\n", r->priority, o, s->id);
}

void RC_BindBufferObject::operator()()
{
	// bind the vbo+ibo
	m->bind();

	// now bind the attrib pointer? state already enabled ofc
	Renderer::setupVertexArray(s, m);
}

void RC_BindBufferObject::debug(char* const str)
{
	sprintf(str, "<RC>: Binding Buffer Object of Mesh %d to suit Shader %d\n", m->id, s->id);
}

void RC_DrawElements::operator()()
{
	// just call gl draw element?
	glDrawElements(primitive, s->elemCount, GL_UNSIGNED_SHORT, (void*)s->idxBegin);
}

void RC_DrawElements::debug(char* const str)
{
	sprintf(str, "<RC>: Drawing Element mode %d, elemcount %d, data offset @ 0x%X\n", primitive, s->elemCount, s->idxBegin);
}