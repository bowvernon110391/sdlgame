#include "Game.h"

#include <math.h>

#include "Helper.h"

Game::Game():
App(10, "Game Test") {
    angle = 0;

	cube = nullptr;
	simple = nullptr;
}

Game::~Game() {
}

void Game::onInit() {
    SDL_Log("Renderer : %s", glGetString(GL_RENDERER));
    SDL_Log("Vendor : %s", glGetString(GL_VENDOR));
    SDL_Log("Version : %s", glGetString(GL_VERSION));
    SDL_Log("GLSL Ver : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// init matrices
	proj = glm::perspective(glm::degrees(45.0f), iWidth/(float)iHeight, .01f, 1000.0f);

	view = glm::translate(0, 0, 2);
	view = glm::inverse(view);

	model = glm::mat4(1.0f);

	// log it?
	float *m;
	
	m = glm::value_ptr(proj);
	SDL_Log("perspective @ %.2f, %.2f:", glm::degrees(45.0f), iWidth / (float)iHeight);
	for (int i=0; i<4; i++) {
		SDL_Log("%.2f %.2f %.2f %.2f", m[0 + i], m[4 + i], m[8 + i], m[12 + i]);
	}

	m = glm::value_ptr(view);
	SDL_Log("view:");
	for (int i=0; i<4; i++) {
		SDL_Log("%.2f %.2f %.2f %.2f", m[0 + i], m[4 + i], m[8 + i], m[12 + i]);
	}

	m = glm::value_ptr(model);
	SDL_Log("model:");
	for (int i=0; i<4; i++) {
		SDL_Log("%.2f %.2f %.2f %.2f", m[0 + i], m[4 + i], m[8 + i], m[12 + i]);
	}

	glm::mat4 mvp = glm::mul( glm::mul(proj, view), model);
	m = glm::value_ptr(mvp);
	SDL_Log("MVP:");
	for (int i=0; i<4; i++) {
		SDL_Log("%.2f %.2f %.2f %.2f", m[0 + i], m[4 + i], m[8 + i], m[12 + i]);
	}

	// create cube
	cube = Mesh::createBox(1.0f);
	if (cube) {
		if (cube->createBufferObjects()) {
			SDL_Log("Buffer ids: %u %u", cube->vbo, cube->ibo);
		}
	}

	simple = Shader::loadShaderFromFile("shader.vert", "shader.frag");

	SDL_assert(simple != NULL);

	if (simple) {
		/*simple->pushUniformLocation("matProj", 0);
		simple->pushUniformLocation("matView", 1);
		simple->pushUniformLocation("matModel", 2);

		simple->pushUniformLocation("tex0", 3);*/

		simple->pushUniformLocation("time", 0);
		simple->pushUniformLocation("matProj", 1);
		simple->pushUniformLocation("matView", 2);
		simple->pushUniformLocation("matModel", 3);
	}

	// set opengl state
	glEnable(GL_DEPTH_TEST);
	glClearColor(.2f, .5f, .3f, .1f);

	glEnableVertexAttribArray(ATTRIB_POS_LOC);
	glEnableVertexAttribArray(ATTRIB_COL_LOC);
}

void Game::onDestroy() {
    // do clean up here rather than at destructor
    // if (simple) delete simple;
	if (cube) delete cube;
	if (simple) delete simple;
}

void Game::onUpdate(float dt) {
    angle += 5.0f * dt;
}

/* Render function */
void Game::onRender(float dt) {
    float newAngle = angle + 5.0f * dt;
    
    float sA = sin(newAngle);
    float cA = cos(newAngle);
    float aA = ((sA + cA) * .5f);

	glm::vec3 axis = glm::vec3(sA, cA, aA);
	

	model = glm::translate(sA * 0.2f, cA * 0.2f, aA * 0.2f);
	model = glm::rotate(model, newAngle, axis);
    // do render here
    glViewport(0, 0, iWidth, iHeight);
    //glClearColor(sA, cA, aA, 1.0f);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// bind shader and set uniforms?
	simple->use();
	
	glUniform1f(simple->getUniformLocation(0), newAngle);

	glUniformMatrix4fv(simple->getUniformLocation(1), 1, GL_FALSE, glm::value_ptr(proj));
	glUniformMatrix4fv(simple->getUniformLocation(2), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(simple->getUniformLocation(3), 1, GL_FALSE, glm::value_ptr(model));

	// use buffer
	cube->use();

	glVertexAttribPointer(ATTRIB_POS_LOC, 3, GL_FLOAT, false, cube->strideLength, (void*)0);
	glVertexAttribPointer(ATTRIB_COL_LOC, 3, GL_FLOAT, false, cube->strideLength, (void*)12);

	for (int i=0; i<cube->subMeshes.size(); i++) {
		Mesh::SubMesh &s = cube->subMeshes[i];

		glDrawElements(GL_TRIANGLES, s.elemCount, GL_UNSIGNED_SHORT, (void*)s.idxBegin);
	}
}

/* handle event */
void Game::onEvent(SDL_Event *e) {
    if (e->type == SDL_QUIT) {
        this->setRunFlag(false);
    } else if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_ESCAPE:
            case SDLK_AC_BACK:
                this->setRunFlag(false);
            break;
        }
    }
}