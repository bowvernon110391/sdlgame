#pragma once
#include "AbstractRenderObject.h"
#include "AABBTree.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>

/// <summary>
/// pure virtual class
/// </summary>
/// 

class SceneNode;
class SceneObject : public IBoundable, public ITransformable {
public:
	enum ObjectType {
		RENDERABLE,
		LIGHT,
		CAMERA
	};

	enum ObjectGroup {
		STATIC,
		DYNAMIC
	};

	SceneObject(ObjectType t, ObjectGroup g);

	virtual void update(float dt) = 0;

	ObjectType type;
	ObjectGroup group;
	SceneNode* node;

	// Inherited via IBoundable
	virtual AABB getBoundingBox() const override;

	// Inherited via ITransformable
	virtual bool getTransform(glm::mat4* m) const override;
};

class NullObject : public SceneObject {
public:
	NullObject();
	// Inherited via SceneObject
	virtual void update(float dt) override;
	virtual bool getTransform(glm::mat4* m) const override;
};

/// <summary>
/// a representation of scene node
/// </summary>
class SceneNode {
public:
	SceneNode(SceneObject *o);
	virtual ~SceneNode();

	void setParent(SceneNode* p);
	void addChild(SceneNode* c);
	void removeChild(SceneNode* c);

	virtual void update(float dt);

	// the transformation matrix (nope, should be got from object, no?)
	glm::mat4 transform;

	// parent child
	SceneNode* parent;
	std::vector<SceneNode*> children;

	// the object (could be mesh, light, camera, etc)
	SceneObject* obj;
};

/// <summary>
/// list of node
/// </summary>
class SceneGraph {
public:
	enum {
		CLIP_RENDERABLES = (1 << 0),
		CLIP_LIGHTS = (1 << 1),
		CLIP_CAMERAS = (1 << 2),

		CLIP_STATIC = (1 << 3),
		CLIP_DYNAMIC = (1 << 4),

		CLIP_ALL = CLIP_RENDERABLES | CLIP_LIGHTS | CLIP_CAMERAS | CLIP_STATIC | CLIP_DYNAMIC
	};

	SceneGraph();
	~SceneGraph();

	// add a scene object
	SceneNode* addObject(SceneObject* o, SceneNode* n = nullptr);
	// update teh scenegraph
	void update(float dt);
	// get all scene objects
	void clip(Frustum* f, int clip_flags, std::vector<const SceneObject*> clipped);

	// root scene node
	SceneNode* root;

	// the aabb tree?
	AABBTree* dynamic_tree;
	AABBTree* static_tree;

	// based on object type
	std::vector<SceneNode*> renderables;
	std::vector<SceneNode*> lights;
	std::vector<SceneNode*> cameras;
};