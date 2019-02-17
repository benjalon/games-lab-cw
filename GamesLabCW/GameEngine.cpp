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

	//Enable v-sync
	if (vsync) glfwSwapInterval(1);

	//Initialise the render system
	renderer::init();

	//Load entity prototypes
	prototypes::register_prototypes();

	//EXAMPLE Load models
	renderer::load("models/Wolf/wolf.obj");
	renderer::load("models/teapot.obj");
	renderer::load("models/sphere.obj");
	renderer::load("models/Moon/moon.obj");
	renderer::load("models/torus.obj");
	renderer::load("models/Plane/Plane.obj");
	renderer::finalise();

	//Remove 'loading' from title
	glfwSetWindowTitle(window_, WINDOW_TITLE.c_str());
}

void game::GameEngine::run()
{
	//EXAMPLE Instantiate a camera and models
	scene_.instantiate("Camera", CameraComponent{ {0,2,5} });
	scene_.instantiate("Model", ModelComponent{ "models/Plane/Plane.obj" }, ColourComponent{ {0.2,0.2,0.2} });
	TransformComponent t_wolf; t_wolf.position.x = -5;
	scene_.instantiate("Model", ModelComponent{ "models/Wolf/wolf.obj" }, t_wolf);
	TransformComponent t_teapot; t_teapot.position.y = 1;  t_teapot.position.x = -2; t_teapot.scale = { 0.02, 0.02, 0.02 };
	scene_.instantiate("Model", ModelComponent{ "models/teapot.obj", 8 }, ColourComponent{ {1,0.5,1} }, t_teapot);
	TransformComponent t_sphere; t_sphere.position.y = 5; t_sphere.position.z = -10; t_sphere.scale = { 0.04, 0.04, 0.04 };
	scene_.instantiate("Model", ModelComponent{ "models/Moon/moon.obj", 128 }, t_sphere);
	TransformComponent t_torus; t_torus.position.y = 1; t_torus.position.x = 5; t_torus.scale = { 0.04, 0.04, 0.04 };
	scene_.instantiate("Model", ModelComponent{ "models/torus.obj" }, ColourComponent{ {0,0.2,1} }, t_torus);

	scene_.instantiate("AmbientLight", AmbientLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 0.01 });
	scene_.instantiate("DirectionalLight", DirectionalLightComponent{ {1, 1, 1}, 1, {2,1,1} });
	scene_.instantiate("PointLight", PointLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 1, {0,5,0} });

	//EXAMPLE Instantiate entities by prototypes with given variables
	//scene_.instantiate("NamedEntity");
	//scene_.instantiate("NamedEntity", NameComponent{ "MyName" });
	//scene_.instantiate({ "NamedEntity", "KinematicBody" }, KinematicComponent{ {0,0,0},{1,0,0} }, NameComponent{ "Hello" });

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
	input::cursor_pos = { x, y };
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
