#include "Game.hpp"
#include <iostream>
#include <string>

using namespace std;

int main( int argc, char **argv ) 
{
#ifndef NOSOUND
    SDL_Init( SDL_INIT_EVERYTHING );
#endif
	CS488Window::launch(argc, argv, new Game(), 1024, 768, "Legend of Cube");

#ifndef NOSOUND
    SDL_Quit();
#endif

	return 0;
}
