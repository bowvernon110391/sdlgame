#include "Game.h"
#include <SDL_opengl.h>
// #include <GL/gl.h>
#include <math.h>

Game::Game():
App(10, "Game Test") {
    angle = 0;
}

Game::~Game() {

}

void Game::onInit() {
    // SDL_Log("Renderer : %s", glGetString("RENDERER"));
}

void Game::onDestroy() {

}

void Game::onUpdate(float dt) {
    angle += dt;
}

/* Render function */
void Game::onRender(float dt) {
    float newAngle = angle + dt;
    
    float sA = sin(newAngle) * 2.f - 1.f;
    float cA = cos(newAngle) * 2.f - 1.f;
    float aA = 1.f - ((sA + cA) * .5f);
    // do render here
    glViewport(0, 0, iWidth, iHeight);
    glClearColor(sA, cA, aA, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

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