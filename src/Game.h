#ifndef __GAME_H__
#define __GAME_H__

#include "App.h"

class Game : public App
{
private:
    float angle;
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