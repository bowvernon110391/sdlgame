#pragma once
#include "AABB.h"
#include "AbstractRenderObject.h"
#include <stdio.h>
#include <queue>
#include <unordered_map>
#include <stack>

class Renderer;

class AABBNode {
public:
	AABBNode(const AbstractRenderObject* obj, float fatten=0.2f):
		obj(obj), parent(nullptr), left(nullptr), right(nullptr)
	{
		if (obj) {
			bbox = obj->getBoundingBox();
			bbox.expand(fatten);
		}
	}

	bool isLeaf() const {
		return !(left || right);
	}

	bool isRoot() const {
		return !parent;
	}

	AABBNode* sibling() const {
		// only valid if we have parent
		if (!parent) 
			return nullptr;
		return parent->left == this ? parent->right : parent->left;
	}

	// cost of combination with other node
	float computeCost(const AABBNode* other) const {
		// return computed cost
		return AABB::combined(bbox, other->bbox).area();
	}

	// returns true if bbox changed
	bool refit() {
		// refit bounding box, only makes sense for branch node
		if (isLeaf()) {
			return false;
		}

		// okay, we have left and right, so compute new bbox
		float SA = bbox.area();
		bbox = AABB::combined(left->bbox, right->bbox);

		return bbox.area() != SA;
	}

	// do some kind of rotation? return true if we do rotation
	bool rotate() {
		// if only two rotation possible, sibling vs left, sibling vs right
		// and only possible if we have parent and we have children (not leaf)
		if (!parent || isLeaf())
			return false;
		// initial cost = our SA
		float currentCost = bbox.area();
		AABBNode* s = sibling();
		AABBNode* candidate = nullptr;
		bool rotated = false;
		// 1st case, sibling vs left
		float cost = s->computeCost(left);
		// rotate it?
		// measure left
		if (cost < currentCost) {
			currentCost = cost;
			candidate = left;
		}
		// measure right
		cost = s->computeCost(right);
		if (cost < currentCost) {
			currentCost = cost;
			candidate = right;
		}

		// do we rotate?
		if (candidate) {
			// perform rotation
			// left rotation
			if (s->parent->left == s) {
				// s is left node
				s->parent->left = candidate;
			}
			else {
				// s is right node
				s->parent->right = candidate;
			}
			candidate->parent = s->parent;

			if (candidate == left) {	
				left = s;
			}
			else {
				// right rotation
				right = s;
			}
			s->parent = this;
			return true;
		}
		return false;
	}

	void debugPrint(int lvl = 0) {
		// print our self, and our child?
		char tabs[32] = { 0 };
		for (int i = 0; i < lvl; i++) {
			tabs[i] = '=';
		}
		tabs[lvl] = 0;

		printf("%s> (%X), root(%s), leaf(%s), bbox(%.2f %.2f %.2f | %.2f %.2f %.2f) parent(%X)\n",
			tabs, (size_t) this, isRoot()?"Y":"N", isLeaf()?"Y":"N", 
			bbox.min.x, bbox.min.y, bbox.min.z, bbox.max.x, bbox.max.y, bbox.max.z,
			(size_t) parent);
		if (!isLeaf()) {
			// got child, recurse
			printf("%s> (left):\n", tabs);
			left->debugPrint(lvl+1);
			
			printf("%s> (right):\n", tabs);
			right->debugPrint(lvl+1);
		}
	}

	AABB bbox;
	const AbstractRenderObject* obj;
	// relations
	AABBNode* parent, * left, * right;
};

class AABBTree {
public:
	AABBTree() {
		objs.clear();
		root = nullptr;
	}
	~AABBTree() {
		// do we own the render objects? nope. so just clear containers
		objs.clear();
		
		if (root) {
			// delete recursively?
			std::stack<AABBNode*> s;
			s.push(root);

			while (!s.empty()) {
				AABBNode* n = s.top();
				s.pop();

				if (!n->isLeaf()) {
					s.push(n->left);
					s.push(n->right);
				}

				// delete here
				delete n;
			}
		}
	}

	AABBNode* findNode(const AbstractRenderObject* obj) {
		if (objs.find(obj) == objs.end())
			return nullptr;
		return objs.at(obj);
	}

	void debugDraw(Renderer* r);
	void debugPrint() {
		if (root) {
			root->debugPrint();
		}
		else {
			printf("NO_ROOT_ERROR!!\n");
		}
	}

	// operations
	AABBNode* insert(AABBNode* n);
	void remove(AABBNode* n);

	// members?
	// first, vector of aabb tree?
	std::unordered_map<const AbstractRenderObject*, AABBNode*> objs;
	AABBNode* root;

protected:
	// return best node
	AABBNode* findBestNode(const AABBNode* contender) const;
};

