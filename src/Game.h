#ifndef __GAME_H__
#define __GAME_H__

#include "App.h"

// #define GLM_FORCE_MESSAGES
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Shader.h"
#include "Mesh.h"

class Game : public App
{
private:
    float angle;

    glm::mat4 proj, view, model;

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