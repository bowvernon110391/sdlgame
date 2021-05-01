#include "ShaderInstanceManager.h"

ShaderInstanceManager::ShaderInstanceManager()
{
    // do nothing?
}

ShaderInstanceManager::~ShaderInstanceManager()
{
    // delete them all
    auto it = shaders.begin();
    while (it != shaders.end()) {
        delete it->second;
        it++;
    }
}

const Shader* ShaderInstanceManager::getShader(const ShaderKey& key)
{
    // first, try to grab existing
    auto it = shaders.find(key);
    if (it != shaders.end()) {
        const Shader* instance = it->second;
#ifdef _DEBUG
        SDL_Log("SHADER ALREADY LOADED: %d @ %X\n\t", key.source->id, (size_t)instance);
#endif
        return instance;
    }

    // not found, instantiate it
#ifdef _DEBUG
    SDL_Log("SHADER NEEDS INSTANTIATING: %d\n\t", key.source->id);
    //key.source->debugPrint();
#endif // _DEBUG

    // make sure source is not null
    SDL_assert(key.source != nullptr);

    Shader* instance = nullptr; // DO LOADING HERE
    shaders.insert(std::make_pair(key, instance));
    return instance;
}

void ShaderInstanceManager::printDebug() const
{
    SDL_Log("SHADER LIBRARY SIZE: %d instances\n", shaders.size());
    auto it = shaders.begin();
    while (it != shaders.end()) {
        const ShaderKey& key = it->first;
        const Shader* shd = it->second;
        SDL_Log("Key[%d] : sourceId(%d) mem(%X)\n", key.computeHash(), key.source->id, (size_t)shd);
        it++;
    }
}
