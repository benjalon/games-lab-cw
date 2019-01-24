#include <GL/freeglut.h>

#include "GamesLabCW.h"

int main(int argc, char** argv)
{
	// Setup and create a cross-platform GLUT window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Tutorial 01");

	// Pass buffer clear function
	glutDisplayFunc(RenderSceneCB);

	// Render objects
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glutMainLoop();

	return 0;
}

/*
	Clear back buffer, render onto it and then swap it in
*/
static void RenderSceneCB()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
}