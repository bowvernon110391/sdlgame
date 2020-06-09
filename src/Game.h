#ifndef __GAME_H__
#define __GAME_H__

#include "App.h"

// #define GLM_FORCE_MESSAGES
#include <glm/glm.hpp>
#include <glm/ext.hpp>


#define GL_GLEXT_PROTOTYPES 1

#include <SDL_opengles2.h>

#include "Shader.h"
#include "Mesh.h"

class Game : public App
{
private:
    float angle;

    glm::mat4 proj, view, world;

    Shader *simple;
    Mesh *cube;
public:
    Game(/* args */);
    virtual ~Game();
    
    void onInit();
    void onDestroy();
    void onUpdate(float dt);
    void onRender(float dt);
    void onEvent(SDL_Event *e);
};


#endif