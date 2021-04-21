#ifndef __GAME_H__
#define __GAME_H__

#include "App.h"
#include "ResourceManager.h"
#include <vector>
class Renderer;
class AbstractRenderObject;
class Shader;
class Mesh;
class ShaderData;
class Material;
class Texture2D;
class MaterialSet;
class LargeMesh;
class AABBTree;
class AABBNode;

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

    void spawnRandomObject();
    void clearTrees();
    void debugPrint(AABBNode* n);

    // some of our loaders, not necessarily a file
    static Shader* loadShader(const char* name);
    static ShaderData* loadBasicShaderData(const char* name);
    static Mesh* loadMesh(const char* name);
    static Texture2D* loadTexture(const char* name);
    static Material* loadBasicMaterial(const char* name);
    static MaterialSet* loadMaterialSet(const char* name);
    static LargeMesh* loadLargeMesh(const char* name);

    // THE renderer
    Renderer* m_renderer;

    // will replace with proper manager later...
    ResourceManager<Shader> *shaderMgr;
    ResourceManager<Mesh> *meshMgr;
    ResourceManager<LargeMesh>* largeMeshMgr;
    ResourceManager<ShaderData> *shaderDataMgr;
    ResourceManager<Material> *materialMgr;
    ResourceManager<Texture2D> *textureMgr;
    ResourceManager<MaterialSet> *matsetMgr;

    std::vector<AbstractRenderObject*> renderObjs;

    float cam_horzRot, cam_vertRot, cam_dist;

    AABBTree* tree;
};


#endif