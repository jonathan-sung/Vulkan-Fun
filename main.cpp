
#include "Game.h"
#include <iostream>
#include <memory>
#include <stdexcept>

int main()
{
    try
    {
        std::unique_ptr<Game> game = std::make_unique<Game>();
        game->run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
