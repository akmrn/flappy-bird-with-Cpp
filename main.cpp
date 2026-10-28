#include "Game.hpp"
#include <exception>
#include <iostream>

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        // Also show a message box, since GUI users often have no console
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(), nullptr);
        return 1;
    } catch (...) {
        std::cerr << "Fatal Error: unknown exception\n";
        return 1;
    }
    return 0;
}
