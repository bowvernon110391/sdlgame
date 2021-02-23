#include "Mesh.h"

#include <glad/glad.h>
#include <SDL_log.h>

Mesh::~Mesh() {
	if (vbo) glDeleteBuffers(1, &vbo);
	if (ibo) glDeleteBuffers(1, &ibo);
}

void Mesh::use() {
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

bool Mesh::createBufferObjects() {
	// does it make sense?
	if (!vertBuffer.size() || !idxBuffer.size()) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot initialize empty vertex buffer!");
		return false;
	}

	// create vbo first
	glGenBuffers(1, &vbo);

	if (!vbo) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed creating vbo");
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertBuffer.size() * sizeof(vertBuffer[0]), &vertBuffer[0], GL_STATIC_DRAW);

	// create ibo next
	glGenBuffers(1, &ibo);

	if (!ibo) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed creating ibo");
		glDeleteBuffers(1, &vbo);

		return false;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxBuffer.size() * sizeof(idxBuffer[0]), &idxBuffer[0], GL_STATIC_DRAW);

	return true;
}