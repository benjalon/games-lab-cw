/**
 * GameEngine.h
 * Declares the GameEngine class, the main manager of the
 * game application and environment.
 */

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <utility>
#include <unordered_map>

#include "Scene.h"
#include "Events.h"

namespace game
{
	//Initial window width
	constexpr int WINDOW_WIDTH = 1920;

	//Initial window height
	constexpr int WINDOW_HEIGHT = 1080;

	//Aspect ratio to enforce
	constexpr std::pair<int, int> ASPECT_RATIO { 16, 9 };

	//Aspect ratio to enforce
	constexpr double ASPECT_RATIO_VAL = (double)ASPECT_RATIO.first / ASPECT_RATIO.second;

	//Target game rate (Hz)
	constexpr int GAME_RATE = 60;

	//Minimum game rate (Hz)
	constexpr int GAME_RATE_MIN = 10;

	//Maximium render distance
	constexpr double RENDER_DISTANCE = 300.0;

	//Cull more distant point lights
	constexpr bool CULL_POINT_LIGHTS = true;

	//Allow free-flying movement through solids
	constexpr bool NOCLIP = false;

	//Application window title
	const std::string WINDOW_TITLE = "DunCraw: The Third Realm";


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


		//Handler to quit the game
		void quit(const events::QuitGame &);

		//Handler to toggle fullscreen
		void toggle_fullscreen(const events::ToggleFullscreen &);


		//Key input callback
		static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

		//Cursor position callback
		static void cursor_position_callback(GLFWwindow *window, double x, double y);

		//Mouse button callback
		static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

		//Window size callback
		static void window_size_callback(GLFWwindow *window, int width, int height);
	};
}
