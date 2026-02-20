#include "Game.h"
#include <SDL.h>

int main(int argc, char* argv[]) {
    Game game;

    if (!game.initialize()) {
        SDL_Log("Failed to initialize game!");
        return 1;
    }

    game.run();

    return 0;
}