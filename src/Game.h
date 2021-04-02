#ifndef __GAME_H__
#define __GAME_H__

#include "App.h"
#include <vector>

class Renderer;
class BaseRenderObject;
class Shader;
class Mesh;
class ShaderData;
class Material;
class Texture2D;
class MaterialSet;


// #define GLM_FORCE_MESSAGES

class Game : public App
{
private:
    
public:
    Game(/* args */);
    virtual ~Game();
    
    void onInit();
    void onDestroy();
    void onUpdate(float dt);
    void onRender(float dt);
    void onEvent(SDL_Event *e);

private:
    void initImGui();
    void destroyImGui();
    void beginRenderImGui();
    void endRenderImGui();

    Renderer* m_renderer;

    // will replace with proper manager later...
    std::vector<BaseRenderObject*> renderObjs;
    std::vector<Shader*> shaders;
    std::vector<Mesh*> meshes;
    std::vector<ShaderData*> shaderDatas;
    std::vector<Material*> mats;
    std::vector<Texture2D*> textures;
    std::vector<MaterialSet*> matsets;

    float cam_horzRot, cam_vertRot, cam_dist;
};


#endif