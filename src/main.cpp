#include "Game.h"

int main(int argc, char** argv) {
  Game *g = new Game();

  g->run();

  delete g;

  return 0;
}