#include "ColorDepthPass.h"
#include "Renderer.h"
#include <algorithm>
#include <stdio.h>
#include <glad/glad.h>

uint32_t ColorDepthPass::computeRenderableHash(const Renderable& r) {
	// most important is shader
	// then shader data
	// then buffer

	// use 32 bit, layout like this:
	// 8 bit shader | 10 bit shader data | 10 bit buffer | 6 bit <UNUSED>?	
	uint32_t hash = (r.s->id << 24) | (r.sd->id << 16) | (r.m->id << 6);
	
	return hash;
}

void ColorDepthPass::draw(float dt)
{
	// at this point, we got our renderables mostly, so just generate render commands,
	// and draw
	generateRenderCommand();

	// preapre state
	prepare(dt);

	// now just print them all
	for (RenderCommand* rc : cmds) {
		rc->operator()();
	}
}

/// <summary>
/// tell the render object to fill the bucket
/// </summary>
/// <param name="o"></param>
void ColorDepthPass::processRenderObject(AbstractRenderObject* o)
{
	// tell it to build renderables?
	o->fillRenderable(this->bucket);
}

/// <summary>
/// generate render command from a list of renderables
/// </summary>
void ColorDepthPass::generateRenderCommand()
{
	 //a bunch of tracker?
	Shader* ls = nullptr;
	ShaderData* lsd = nullptr;
	Mesh* lb = nullptr;
	AbstractRenderObject* lro = nullptr;

	// order our renderables?
	std::sort(bucket.begin(), bucket.end(),
		[](const Renderable& a, const Renderable& b) {
			return computeRenderableHash(a) < computeRenderableHash(b);
		});

	// now generate our render command
	for (const Renderable& r : bucket) {
		// make sure it has shader
		assert(r.s != nullptr);

		// is it different shader?
		if (r.s != ls) {
			// setup new shader
			cmds.push_back(new RC_BindShader(r.s, this));
			ls = r.s;	// update last shader

			// since it's a first bind, also set data
			cmds.push_back(new RC_BindShaderData(r.s, r.sd, this));
			lsd = r.sd;

			// rebind buffer too (cause buffer is kinda glued to shader)
			// due to different attribute location
			cmds.push_back(new RC_BindBufferObject(r.m, r.s));
			lb = r.m;

			// also since shader changes, even object shared similar data
			// for different meshes, send the command
			cmds.push_back(new RC_BindShaderDataFromObject(ls, r.ro, this));
			lro = r.ro;
		}

		// maybe the shader is same, but different data?
		if (r.sd != lsd) {
			cmds.push_back(new RC_BindShaderData(ls, r.sd, this));
			lsd = r.sd;
		}

		// maybe it's same shader, same data, but different object?
		if (r.ro != lro) {
			cmds.push_back(new RC_BindShaderDataFromObject(ls, r.ro, this));
			lro = r.ro;
		}

		// could it be different buffer object?
		if (r.m != lb) {
			cmds.push_back(new RC_BindBufferObject(r.m, ls));
			lb = r.m;
		}

		// okay, now that we're sure then just draw call
		cmds.push_back(new RC_DrawElements(r.sm, GL_TRIANGLES));
	}
}

void ColorDepthPass::prepare(float dt)
{
	// setup viewport
	int* v = renderer->getViewport();
	glViewport(v[0], v[1], v[2], v[3]);

	// prepare all state?
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	//glDepthFunc(GL_LEQUAL);

	// disable bledning
	glDisable(GL_BLEND);
}

void ColorDepthPass::generateDebugString()
{
	// just generate debug string
	this->debugString = std::string("DepthColorPass COMMAND BUFFER: \n");
	// generate string for each of them
	char tmp[256];
	for (RenderCommand* rc : cmds) {
		rc->debug(tmp);
		this->debugString += std::string(tmp);
	}
}
