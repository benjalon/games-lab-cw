#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"

std::unique_ptr<Camera> camera;
std::unique_ptr<Shader> shader;
std::unique_ptr<Mesh> mesh;

const float WINDOW_WIDTH = 1280.0f;
const float WINDOW_HEIGHT = 1024.0f;

static void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	camera->Render(WINDOW_WIDTH, WINDOW_HEIGHT);
	mesh->Render(shader->GetShaderID(), camera->GetProjectionMatrix(), camera->GetViewMatrix());

	glutSwapBuffers();
}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(Render);
	glutIdleFunc(Render);
}

int main(int argc, char** argv)
{
	// Initialize glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Dungeon Crawler");
	InitializeGlutCallbacks();

	// Initialize glew
	GLenum response = glewInit();
	assert(response == GLEW_OK && glewGetErrorString(response));

	// Create scene objects
	camera = std::make_unique<Camera>();
	shader = std::make_unique<Shader>();
	mesh = std::make_unique<Mesh>("bench.obj");

	// Render setup
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// Render
	glutMainLoop();

	return 0;
}