#include "Game.hpp"
#include <iostream>
#include <string>

using namespace std;

int main( int argc, char **argv ) 
{
    SDL_Init( SDL_INIT_EVERYTHING );
	CS488Window::launch(argc, argv, new Game(), 1024, 768, "Legend of Cube");

    SDL_Quit();

	return 0;
}
