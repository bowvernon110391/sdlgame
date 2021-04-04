#pragma once
#include "RenderPass.h"
#include <string>


class ColorDepthPass : public RenderPass {
public:
	ColorDepthPass(int priority) :
		RenderPass(priority)
	{
		debugString = std::string("DEPTH-COLOR PASS!");
	}

	static uint32_t computeRenderableHash(const Renderable& r);
	// Inherited via RenderPass
	virtual void draw(float dt) override;
	virtual void processRenderObject(AbstractRenderObject* o) override;
	// unique
	void generateRenderCommand();


	// Inherited via RenderPass
	virtual void prepare(float dt) override;


	// Inherited via RenderPass
	virtual void generateDebugString() override;

};