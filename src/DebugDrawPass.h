#pragma once
#include <vector>
#include "RenderPass.h"
#include "Mesh.h"

class DebugDrawPass : public RenderPass {
public:
	DebugDrawPass(int priority) :
		RenderPass(priority) 
	{
		debugString = "DEBUG DRAW PASS";
		unitCube = Mesh::createUnitBox()->createBufferObjects();
	}

	virtual ~DebugDrawPass() {
		if (unitCube) delete unitCube;
	}

	// Inherited via RenderPass
	virtual void prepare(float dt) override;
	virtual void draw(float dt) override;
	virtual void processRenderObject(AbstractRenderObject* o) override;
	virtual void generateDebugString() override;

	virtual void clear() {
		// add one more to clear
		RenderPass::clear();
		bboxes.clear();
	}

	std::vector<AABB> bboxes;
	Mesh* unitCube;
};