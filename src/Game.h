#ifndef __GAME_H__
#define __GAME_H__

#include "App.h"

// #define GLM_FORCE_MESSAGES
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Shader.h"
#include "Mesh.h"
#include "Texture2d.h"

class Game : public App
{
private:
    float angle;
    float speed, lightSpeed;
    bool animate;
    float bgColor[4];

    int projectionType;
    float perspectiveFOV;
    float orthoRange;

    glm::mat4 proj, view, model;

    Mesh *cube;
    Texture2D* tex;
public:
    Game(/* args */);
    virtual ~Game();
    
    void onInit();
    void onDestroy();
    void onUpdate(float dt);
    void onRender(float dt);
    void onEvent(SDL_Event *e);

private:
    void computeProjection();
    void computeCameraMatrix();
    void initImGui();
    void destroyImGui();
    void beginRenderImGui();
    void endRenderImGui();
};


#endif