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

	// can we rotate?
	bool canRotate() const {
		if (isLeaf()) return false;
		if (left->isLeaf() && right->isLeaf()) return false;
		return true;
	}

	AABB potentialRotatedBox() const {
		if (!canRotate()) {
			return AABB();
		}

		// compute for real?
		float c1, c2;
		AABBNode* cd1, * cd2;
		c1 = left->potentialReduction(&cd1);
		c2 = right->potentialReduction(&cd2);
		AABB best;
		if (c1 > c2 && c1 > 0.f) {
			return AABB::combined(cd1->bbox, right->bbox);
		}
		else if (c2 > 0.f) {
			return AABB::combined(cd2->bbox, left->bbox);
		}

		return AABB();
	}

	// cost of combination with other node
	float computeCost(const AABBNode* other) const {
		// return computed cost
		return AABB::combined(bbox, other->bbox).area();
	}

	// reduction in cost if we're swapping child with sibling
	float potentialReduction(AABBNode** candidate=nullptr) {
		if (isLeaf()) {
			return 0.f;
		}

		AABBNode* s = sibling();
		// if we have no sibling, cannot reduce (root)
		if (!s) {
			return 0.f;
		}

		// compute the greatest reduction
		float initSA = bbox.area();
		float reducedLeft = initSA - AABB::combined(left->bbox, s->bbox).area();
		reducedLeft = reducedLeft < 0 ? 0 : reducedLeft;
		float reducedRight = initSA - AABB::combined(right->bbox, s->bbox).area();
		reducedRight = reducedRight < 0 ? 0 : reducedRight;

		// which is greater?
		if (reducedLeft > reducedRight) {
			if (candidate)
				*candidate = left;
			return reducedLeft;
		}

		if (candidate)
			*candidate = right;
		return reducedRight;
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
		if (isLeaf()) {
#ifdef _DEBUG
			printf("TREE_ROTATE: Node[%X] is leaf!\n", this);
#endif // _DEBUG
			return false;
		}
		// if both children are leaves, don't do shit
		if (left->isLeaf() && right->isLeaf()) {
#ifdef _DEBUG
			printf("TREE_ROTATE: Node[%X] children are all leaves!\n", this);
#endif // _DEBUG
			return false;
		}
		// at least one of the child is a branch!, so build possibilities
		AABBNode* p1 = nullptr, * p2 = nullptr;
		AABBNode* c1, * c2;
		float redLeft = left->potentialReduction(&c1);
		float redRight = right->potentialReduction(&c2);

		if (redLeft > redRight && redLeft > 0.0f) {
			// do left rotation?
#ifdef _DEBUG
			printf("TREE_ROTATE: Node[%X] Left Rotate with (%X)\n", this, c1);
#endif // _DEBUG
			p1 = left;
			p2 = c1;
		} else if (redRight > 0.0f) {
			// do right rotation?
#ifdef _DEBUG
			printf("TREE_ROTATE: Node[%X] Right Rotate with (%X)\n", this, c2);
#endif // _DEBUG
			p1 = right;
			p2 = c2;
		}

		// do the rotation?
		if (p1 && p2) {
			// p1 always higher than p2
			// p2.grandParent = p1.parent

			// high to low relations first
			if (p1->parent->left == p1) {
				p1->parent->left = p2;
			}
			else {
				p1->parent->right = p2;
			}

			if (p2->parent->left == p2) {
				p2->parent->left = p1;
			}
			else {
				p2->parent->right = p1;
			}

			// now low to high relations...
			AABBNode* tmp = p1->parent;
			p1->parent = p2->parent;
			p2->parent = tmp;
			// update aabb for p1's new parent
			p1->parent->refit();
			p2->parent->refit();
		}
#ifdef _DEBUG
		else {
			printf("TREE_ROTATE: Node[%X] (NO POSSIBLE ROTATION)!\n", this);
		}
#endif // _DEBUG

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
		selected = nullptr;
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

	void setDebugBox(const AABB& b) {
		dbgBest = b;
	}

	void resetDebugBox() {
		dbgBest = AABB();
	}

	// operations
	AABBNode* insert(AABBNode* n);
	void remove(AABBNode* n);

	// members?
	// first, vector of aabb tree?
	std::unordered_map<const AbstractRenderObject*, AABBNode*> objs;
	AABBNode* root;
	AABBNode* selected;

	AABB dbgBest;

protected:
	// return best node
	AABBNode* findBestNode(const AABBNode* contender) const;
};

