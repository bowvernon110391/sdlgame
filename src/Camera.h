#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera {
public:
	Camera() {
		up = glm::vec3(0, 1, 0);
		target = eye + glm::vec3(0, 0, -1);
		fov = 80.0f;	// default to 80 deg
		aspect = 1.0f;
		isPerspective = true;
		nearPlane = 0.2f;
		farPlane = 100.0f;
	}

	virtual glm::mat4 getViewMatrix() {
		glm::mat4 view;
		view = glm::lookAt(eye, target, up);
		return view;
	}

	virtual glm::mat4 getProjectionMatrix() {
		glm::mat4 proj;
		if (isPerspective) {
			proj = glm::perspectiveFov(glm::radians(fov), aspect, 1.0f, nearPlane, farPlane);
		}
		else {
			proj = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
		}
		return proj;
	}

	Camera* setAspect(float a) { aspect = a; return this; }
	Camera* usePerspective(bool u) { isPerspective = u; return this; }
	Camera* setPosition(const glm::vec3 v) { eye = v; return this; }
	Camera* setTarget(const glm::vec3 v) { target = v; return this; }
	Camera* setUp(const glm::vec3 v) { up = v; return this; }
	Camera* setFov(const float& f) { fov = f; return this;  }
	Camera* setClipDistance(float n, float f) {
		nearPlane = n; 
		farPlane = f;
		return this;
	}

	const glm::vec3& getPosition() const { return eye;  }
	const glm::vec3& getTarget() const { return target;  }
	const glm::vec3& getUp() const { return up;  }
	glm::vec3 getDir() const {
		return target - eye;
	}

protected:
	// what does a camera have?
	glm::vec3 eye, target, up;
	// is it perspective camera?
	bool isPerspective;
	// for perspective
	float fov;
	float aspect;
	float nearPlane, farPlane;
	// for orthogonal
	float left, right, top, bottom;
};