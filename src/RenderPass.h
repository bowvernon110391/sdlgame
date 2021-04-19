#pragma once

#include <vector>
#include <string>
#include "Renderable.h"
#include "RenderCommand.h"

class AbstractRenderObject;
class Renderer;
/// <summary>
/// This is a render pass class, which has responsibility to:
/// SETUP DATA (Shader)
/// GENERATE RENDER QUEUE/COMMAND
/// </summary>
class RenderPass {
public:
	RenderPass(int priority = 0) : renderer(nullptr), priority(priority), bucket(0), cmds(0), debugString("DEBUG_STRING_HERE") {}
	virtual ~RenderPass() {}

	virtual void prepare(float dt) = 0;
	virtual void draw(float dt) = 0;
	virtual void processRenderObject(AbstractRenderObject* o) = 0;
	virtual void setupData(const Shader* s);
	
	virtual void generateDebugString() = 0;

	virtual void clear() {
		bucket.clear();
		// clear current commands?
		for (RenderCommand* rc : cmds) {
			delete rc;
		}
		cmds.clear();
	}

	int priority;
	// has a pointer to renderer
	Renderer* renderer;
	std::string debugString;
protected:
	// bucket of renderables
	std::vector<Renderable> bucket;
	std::vector<RenderCommand*> cmds;

};