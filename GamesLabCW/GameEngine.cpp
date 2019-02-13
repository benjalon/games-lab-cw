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

	glfwMakeContextCurrent(window_);
	glfwSetWindowUserPointer(window_, this);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	//Enable v-sync
	if (vsync) glfwSwapInterval(1);

	//Initialise the render system
	renderer::init();

	//Load entity prototypes
	prototypes::register_prototypes();

	//EXAMPLE Load models
	renderer::load("models/Wolf/wolf.obj");
	renderer::load("models/teapot.obj");
	renderer::finalise();

	//Remove 'loading' from title
	glfwSetWindowTitle(window_, WINDOW_TITLE.c_str());
}

void game::GameEngine::run()
{
	//EXAMPLE Instantiate a camera and models
	scene_.instantiate("Camera", CameraComponent{ {0,2,5} });
	scene_.instantiate("Model", ModelComponent{ "models/Wolf/wolf.obj" });
	TransformComponent t_teapot; t_teapot.position.x = 2; t_teapot.scale = { 0.5, 0.5, 0.5 };
	scene_.instantiate("Model", ModelComponent{ "models/teapot.obj" }, ColourComponent{ {1,0.5,1} }, t_teapot);

	//EXAMPLE Add a 'warm flourescent' ambient light
	scene_.instantiate("AmbientLight", AmbientLightComponent{ {1, 244.0/255, 229.0/255}, 1.0 });

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
