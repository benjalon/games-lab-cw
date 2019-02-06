/**
 * main.cpp
 * Main entry point for the game application.
 */

#include "GameEngine.h"

#include <iostream>
#include <exception>

int main(int argc, char **argv)
{
	try
	{
		//Instantiate the engine
		game::GameEngine app(false, true, false);

		//Run the main game loop
		app.run();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
