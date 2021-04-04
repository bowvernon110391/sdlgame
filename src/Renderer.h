#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <glm/glm.hpp>
#include "RenderPass.h"

class AABB;
class MaterialSet;	// array of Material
class Camera;	// view + projection?
class SceneData;	// lights and stuffs
class Shader;
class Mesh;
class ShaderData;
class AbstractRenderObject;
class RenderPass;
/// <summary>
/// This is our renderer class
/// </summary>
/// 
class Renderer {
public:
	Renderer();
	virtual ~Renderer();

	Renderer* useCamera(Camera* c);
	Renderer* useSceneData(SceneData* sd);
	Renderer* setViewport(int x, int y, int w, int h);
	int* getViewport() { return &viewport[0]; }

	Camera* getCamera() const { return m_camera;  }
	SceneData* getSceneData() const { return m_scene; }

	RenderPass* addPass(const std::string& name, RenderPass* p);
	RenderPass* getPass(const std::string& name) { return passes[name]; }

	void createPasses();

	// usable by everyone?
	static void setupVertexState(const Shader* shd);
	static void setupVertexArray(const Shader* shd, const Mesh* m);

	// our draw function?
	void draw(std::vector<AbstractRenderObject*>& objs, float dt);
	
	bool drawDebug;
	glm::vec4 debugColor;
protected:
	friend class RenderPass;

	std::unordered_map<std::string, RenderPass*> passes;
	std::vector<RenderPass*> orderedPasses;

	Camera *m_camera;
	SceneData *m_scene;

	int viewport[4];
	int lastAttribFlags;

	// for debugging?
	uint32_t vboDebug, iboDebug;
	std::vector<float> vbDebug;
	std::vector<unsigned short> ibDebug;
	Shader* debugShader;

	void initDebugData();
	void generateDebugData(const AABB& b);
	void destroyDebugData();
};