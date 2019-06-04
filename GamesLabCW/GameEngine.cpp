/**
 * GameEngine.cpp
 * Implements the GameEngine class, the main manager of the
 * game application and environment.
 */

#include "GameEngine.h"

#include <exception>

#include "Components.h"
#include "Prototypes.h"
#include "renderer/Renderer.h"
#include "Input.h"
#include "renderer/Texture.h"

#include "procedural_generation/procedural_generation.h"

game::GameEngine::GameEngine(bool fullscreen, bool vsync, bool ground) :
	fullscreen_(fullscreen), vsync_(vsync), ground_(ground)
{
	//Initialise the GLFW window
	if (!glfwInit())
		throw std::exception("GLFW could not be initialised");

	//Create the render window
	window_ = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, (WINDOW_TITLE + " - LOADING").c_str(),
		(fullscreen_ ? glfwGetPrimaryMonitor() : nullptr), nullptr);
	if (!window_)
		throw std::exception("Could not create game window");

	glfwSetWindowAspectRatio(window_, ASPECT_RATIO.first, ASPECT_RATIO.second);

	glfwMakeContextCurrent(window_);
	glfwSetWindowUserPointer(window_, this);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	//Set callback functions
	glfwSetKeyCallback(window_, key_callback);
	glfwSetCursorPosCallback(window_, cursor_position_callback);
	glfwSetMouseButtonCallback(window_, mouse_button_callback);
	glfwSetWindowSizeCallback(window_, window_size_callback);

	//Set event handlers
	events::dispatcher.sink<events::QuitGame>().connect<&GameEngine::quit>(this);
	events::dispatcher.sink<events::ToggleFullscreen>().connect<&GameEngine::toggle_fullscreen>(this);
	for (auto &r : events::responses)
		r->log();

	//Hide and initialise the cursor
	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	input::cursor_centre = true;

	//Enable v-sync
	if (vsync) glfwSwapInterval(1);

	//Initialise the render system
	renderer::init();

	//Load entity prototypes
	prototypes::register_prototypes();

	//Load models
	renderer::load_model("models/Room/room.obj");
	renderer::load_model("models/Procedural/type1.obj");
	renderer::load_model("models/Procedural/type2.obj");
	renderer::load_model("models/Procedural/type3.obj");
	renderer::load_model("models/Procedural/type4.obj");
	renderer::load_model("models/Procedural/type5.obj");
	renderer::load_model("models/Water/water.obj");
	renderer::load_model("models/Skybox/skybox.obj");
	renderer::load_model("models/Torch/torch.obj");
	renderer::load_model("models/Key/Key_B_02.obj");
	renderer::load_model("models/Fireball/fireball.obj");
	renderer::load_model("models/Minotaur/Minotaur@Idle.fbx");
	renderer::load_model("models/Minotaur/Minotaur@Walk.fbx");
	renderer::load_model("models/Minotaur/Minotaur@Get_Hit.fbx");
	renderer::load_model("models/Minotaur/Minotaur@Attack.fbx");

	renderer::load_particle_effect("models/Particles/star.png", 200, 0.2, 0.5);
	renderer::load_particle_effect("models/Particles/fire.png", 30, 0.08, 0.3);
	renderer::load_particle_effect("models/Particles/fire2.png", 80, 0.15, 1);

	// The four HP levels (hearts-3 = full HP, hearts-0 = dead)
	renderer::load_overlay("models/UI/hearts-3.png", Vector2(0, 0));
	renderer::load_overlay("models/UI/hearts-2.png", Vector2(0, 0));
	renderer::load_overlay("models/UI/hearts-1.png", Vector2(0, 0));
	renderer::load_overlay("models/UI/hearts-0.png", Vector2(0, 0));

	// The four MP levels (mana-3 = full, mana-0 = empty)
	renderer::load_overlay("models/UI/mana-3.png", Vector2(0, 0));
	renderer::load_overlay("models/UI/mana-2.png", Vector2(0, 0));
	renderer::load_overlay("models/UI/mana-1.png", Vector2(0, 0));
	renderer::load_overlay("models/UI/mana-0.png", Vector2(0, 0));

	// Misc overlays
	renderer::load_overlay("models/UI/crosshair.png", Vector2(0, 0));
	renderer::load_overlay("models/UI/win.png", Vector2(0, 0));
	renderer::load_overlay("models/UI/lose.png", Vector2(0, 0));
	renderer::load_overlay("models/UI/start.png", Vector2(0, 0));

	std::string paths[6] = {
		"models/Skybox/hw_ruins/ruins_lf.tga",
		"models/Skybox/hw_ruins/ruins_rt.tga",
		"models/Skybox/hw_ruins/ruins_up.tga",
		"models/Skybox/hw_ruins/ruins_dn.tga",
		"models/Skybox/hw_ruins/ruins_ft.tga",
		"models/Skybox/hw_ruins/ruins_bk.tga" };
	renderer::load_external_map(paths, "models/Skybox/skybox.obj", TextureType::CUBE, true);
	renderer::load_external_map(paths, "models/Water/water.obj", TextureType::CUBE, false);

	//Start game at hub
	procgen::load_hub(scene_);

	//Remove 'loading' from title
	glfwSetWindowTitle(window_, WINDOW_TITLE.c_str());
}

void game::GameEngine::run()
{
	//Time of next update
	double t_next = glfwGetTime();

	//Time of last update
	double t_last = 0;

	//Main game loop
	while (!glfwWindowShouldClose(window_))
	{
		//Get the current time
		double t_now = glfwGetTime();

		//Update if ready
		if (t_now >= t_next)
		{
			//Set the next update time based on target rate
			t_next += 1.0 / GAME_RATE;

			//Tick game logic
			input::pressed.clear();
			input::released.clear();
			glfwPollEvents();
			double dt = t_now - t_last;
			if (dt > 1.0 / GAME_RATE_MIN)
				dt = 1.0 / GAME_RATE_MIN;
			scene_.tick(dt);

			//Update graphics, but skip if running slowly
			if (t_now < t_next)
				draw();

			//Mark this as the latest update
			t_last = t_now;
		}
	}

	//Terminate when exiting game loop
	glfwTerminate();
}

void game::GameEngine::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene_.draw();
	glfwSwapBuffers(window_);
}

void game::GameEngine::quit(const events::QuitGame &)
{
	glfwSetWindowShouldClose(window_, true);
}

void game::GameEngine::toggle_fullscreen(const events::ToggleFullscreen &)
{
	fullscreen_ = !fullscreen_;
	glfwSetWindowMonitor(window_, (fullscreen_ ? glfwGetPrimaryMonitor() : nullptr),
		0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GLFW_DONT_CARE);
	if (vsync_) glfwSwapInterval(1);
}

void game::GameEngine::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key != GLFW_KEY_UNKNOWN)
	{
		if (action == GLFW_PRESS)
		{
			input::pressed.emplace(key);
			input::held.emplace(key);
		}
		else if (action == GLFW_RELEASE)
		{
			input::released.emplace(key);
			input::held.erase(key);
		}
	}
}

void game::GameEngine::cursor_position_callback(GLFWwindow *window, double x, double y)
{
	//Get current window size
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	//Rescales the given xy coordinate to expected window dimensions
	auto xy = [&](Vector2 &v)
	{
		v.x /= width; v.x *= WINDOW_WIDTH;
		v.y /= height; v.y *= WINDOW_HEIGHT;
	};

	//Get current cursor position
	Vector2 pos = { x, y };
	xy(pos);

	if (input::cursor_centre)
	{
		//Set cursor to middle
		glfwSetCursorPos(window, width / 2.0, height / 2.0);

		//Get offset from middle
		Vector2 pos_mid;
		glfwGetCursorPos(window, &pos_mid.x, &pos_mid.y);
		xy(pos_mid);
		input::cursor_pos = pos_mid - pos;
	}
	else
		input::cursor_pos = pos;
}

void game::GameEngine::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		input::pressed.emplace(button);
		input::held.emplace(button);
	}
	else if (action == GLFW_RELEASE)
	{
		input::released.emplace(button);
		input::held.erase(button);
	}
}

void game::GameEngine::window_size_callback(GLFWwindow *window, int width, int height)
{
	//Ensure the viewport scales to the new window size
	glViewport(0, 0, width, height);
}
