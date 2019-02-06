/**
 * GameEngine.h
 * Declares the GameEngine class, the main manager of the
 * game application and environment.
 */

#pragma once

#include <utility>
#include <GLFW/glfw3.h>

#include "Scene.h"

namespace game
{
	//Initial window width
	const int WINDOW_WIDTH = 1920;

	//Initial window height
	const int WINDOW_HEIGHT = 1080;

	//Aspect ratio to enforce
	const std::pair<int, int> ASPECT_RATIO { 16, 9 };

	//Target game rate (Hz)
	const int GAME_RATE = 60;

	//Minimum game rate (Hz)
	const int GAME_RATE_MIN = 10;

	//Application window title
	const std::string WINDOW_TITLE = "Dungeon Crawler";


	class GameEngine
	{
	private:
		//The window in which to render the game
		GLFWwindow *window_;

		//The current Scene containing all entities
		Scene scene_;

		//Are we currently fullscreen?
		bool fullscreen_;

		//Should we use v-sync?
		bool vsync_;

		//Should we draw a default test ground?
		bool ground_;

		//Renders the game
		void draw();

	public:
		//Initialise the game context and settings with the given options
		GameEngine(bool fullscreen, bool vsync, bool ground);

		//Commence the main game loop
		void run();
	};
}
