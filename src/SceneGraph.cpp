#include "SceneGraph.h"
#include <stack>

SceneObject::SceneObject(ObjectType t, ObjectGroup g): type(t), group(g)
{
	node = nullptr;
}

AABB SceneObject::getBoundingBox() const
{
	return AABB();
}

bool SceneObject::getTransform(glm::mat4* m) const
{
	return false;
}

//=========================================================================================

NullObject::NullObject():
	SceneObject(ObjectType::RENDERABLE, ObjectGroup::STATIC)
{
}

void NullObject::update(float dt)
{
	// do nothing?
}

bool NullObject::getTransform(glm::mat4* m) const
{
	// don't have dynamic transform
	return false;
}

//==========================================================================================
SceneNode::SceneNode(SceneObject* o)
{
	// set relation
	obj = o;
	o->node = this;
	transform = glm::mat4(1.0);	// identity, by default
}

SceneNode::~SceneNode()
{
	// also delete children?
	for (SceneNode* c : children) {
		delete c;
	}
}

void SceneNode::setParent(SceneNode* p)
{
	if (parent)
		parent->removeChild(this);
	p->addChild(this);
}

void SceneNode::addChild(SceneNode* c)
{
	children.push_back(c);
}

void SceneNode::removeChild(SceneNode* c)
{
	// find the child, move to back, and pop?
	for (int i = 0; i < children.size(); i++) {
		if (children[i] == c) {
			SceneNode* tmp = children[children.size() - 1];
			children[children.size() - 1] = c;
			children[i] = tmp;
			children.pop_back();
			return;
		}
	}
}

void SceneNode::update(float dt)
{
	// update transform from child?
	obj->getTransform(&transform);

	// multiply by parent matrix
	if (parent) {
		transform = parent->transform * transform;
	}
}

//=============================================================================================
SceneGraph::SceneGraph()
{
	// initialize root
	root = new SceneNode(new NullObject);
	// initialize tree
	dynamic_tree = new AABBTree();
	static_tree = new AABBTree();
}

SceneGraph::~SceneGraph()
{
	if (root)
		delete root;
}

/// <summary>
/// add object, with parent n
/// </summary>
/// <param name="o">scene object</param>
/// <param name="n">parent node, null by default</param>
/// <returns>newly created node</returns>
SceneNode* SceneGraph::addObject(SceneObject* o, SceneNode* n)
{
	SceneNode* node = new SceneNode(o);
	if (n) {
		// attach to parent?
		node->setParent(n);
	}
	else {
		// attach to root instead
		node->setParent(root);
	}

	// now decide where to put the object in the list
	switch (o->type) {
	case SceneObject::RENDERABLE:
		renderables.push_back(node);
		break;
	case SceneObject::LIGHT:
		lights.push_back(node);
		break;
	case SceneObject::CAMERA:
		cameras.push_back(node);
		break;
	}

	// and the trees
	switch (o->group) {
	case SceneObject::STATIC:
		static_tree->insert(new AABBNode(o));
		break;
	case SceneObject::DYNAMIC:
		dynamic_tree->insert(new AABBNode(o));
		break;
	}

	return node;
}

void SceneGraph::update(float dt)
{
	// use iteration
	std::stack<SceneNode*> s;

	s.push(root);

	while (!s.empty()) {
		SceneNode* n = s.top();
		s.pop();

		// do something, and push children
		n->update(dt); // update is non recursive, but take into account parent node's state

		// add children
		for (SceneNode* c : n->children) {
			s.push(c);
		}
	}

	// gotta update the dynamic tree!
	dynamic_tree->refresh();
}

void SceneGraph::clip(Frustum* f, int clip_flags, std::vector<const SceneObject*> clipped)
{
	std::vector<const IBoundable*> objs;

	if (clip_flags & CLIP_STATIC) {
		static_tree->clip_leaves(f, objs);
	}

	if (clip_flags & CLIP_DYNAMIC) {
		dynamic_tree->clip_leaves(f, objs);
	}

	// now, filter only that is necessary
	for (const IBoundable* obj : objs) {
		const SceneObject* o = static_cast<const SceneObject*>(obj);

		// okay, check if it fills requirement
		if (clip_flags & CLIP_RENDERABLES && o->type == SceneObject::RENDERABLE)
			clipped.push_back(o);
		if (clip_flags & CLIP_LIGHTS && o->type == SceneObject::LIGHT)
			clipped.push_back(o);
		if (clip_flags & CLIP_CAMERAS && o->type == SceneObject::CAMERA)
			clipped.push_back(o);
	}
}
