#include "Game.h"
#include <SDL_opengles2.h>
// #include <GL/gl.h>
#include <math.h>

#include "Mesh.h"

Game::Game():
App(10, "Game Test") {
    angle = 0;
}

Game::~Game() {

}

void Game::onInit() {
    SDL_Log("Renderer : %s", glGetString(GL_RENDERER));
    SDL_Log("Vendor : %s", glGetString(GL_VENDOR));
    SDL_Log("Version : %s", glGetString(GL_VERSION));
    SDL_Log("GLSL Ver : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    Mesh *m = Mesh::createBox(1.0f);

    std::vector<float> &v = m->vertBuffer;
    SDL_Log("Logging vertex buffer...");
    for (int i=0; i<m->vertBuffer.size(); i+=6) {
        SDL_Log("v[%d]: %.2f %.2f %.2f | %.2f %.2f %.2f", i/6, v[i+0], v[i+1], v[i+2], v[i+3], v[i+4], v[i+5]);
    }

    std::vector<uint16_t> &idx = m->idxBuffer;
    SDL_Log("Logging index buffer...");
    for (int i=0; i<m->idxBuffer.size(); i+= 3) {
        SDL_Log("face[%d]: %u %u %u", i/3, idx[i+0], idx[i+1], idx[i+2]);
    }

    SDL_Log("Logging submeshes info...");
    for (int i=0; i<m->subMeshes.size(); i++) {
        const Mesh::SubMesh_t &s = m->subMeshes.at(i);

        SDL_Log("submesh[%d]: begin=%u, count=%u", i, s.idxBegin, s.elemCount);
    }

    delete m;
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