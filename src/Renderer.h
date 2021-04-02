#pragma once

#include <vector>

class RenderQueueItem;
class BaseRenderObject;
class MaterialSet;	// array of Material
class BaseRenderObjectData;	// anything with transform mat4
class Camera;	// view + projection?
class SceneData;	// lights and stuffs
class Shader;
class Mesh;
class ShaderData;
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
	void draw(const std::vector<BaseRenderObject*>& objs);

	Camera* getCamera() const { return m_camera;  }
	SceneData* getSceneData() const { return m_scene; }
protected:
	Camera *m_camera;
	SceneData *m_scene;

	int viewport[4];
	int lastAttribFlags;

	// tracker
	Shader* lastShader;
	ShaderData* lastShaderData;

	void draw(const BaseRenderObject& ro);
	void setupVertexState(const Shader* shd);
	void setPassData(const Shader* shd);
	void setMaterialData(const Shader* shd, const ShaderData* shdata);
	void setInstanceData(const Shader* shd, const BaseRenderObjectData* instance);
	void setupVertexArray(const Shader* shd, const Mesh* m);
};