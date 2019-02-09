/**
 * Renderer.cpp
 */

#define GLM_ENABLE_EXPERIMENTAL

#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

//Quick conversion to radians
#define R(x) glm::radians((float)x)

namespace game::renderer
{
	Shader basic_shader;

	//Calculates the projection matrix for a camera
	glm::mat4 proj_matrix(CameraComponent camera)
	{
		return glm::infinitePerspective(
			R(camera.fov),
			(float)ASPECT_RATIO_VAL,
			0.1f
		);
	}

	//Calculates the view matrix for a camera
	glm::mat4 view_matrix(CameraComponent camera)
	{
		//Calculate direction of view from angles
		glm::vec3 dir = camera.orientation.direction_hv();

		//Get a vector perpendicularly upwards
		glm::vec3 right = camera.orientation.direction_hv_right();
		glm::vec3 up = glm::cross(right, dir);

		return glm::lookAt(
			glm::vec3(camera.position),
			glm::vec3(camera.position) + dir,
			up
		);
	}

	void init()
	{
		//Configure OpenGL
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		//Load shaders
		basic_shader.load("basic_shader", "shaders/shader.vert", "shaders/shader.frag");
	}

	void render_model(CameraComponent camera, ModelComponent model, TransformComponent t)
	{
		//For now, use basic shader
		auto shader = basic_shader.handle();

		glUseProgram(shader);

		glm::mat4 p = proj_matrix(camera);
		glm::mat4 v = view_matrix(camera);

		//Calculate model matrix
		glm::mat4 m = glm::translate(glm::vec3(t.position)) *
			glm::rotate(R(t.rotation.x), glm::vec3(1, 0, 0)) *
			glm::rotate(R(t.rotation.z), glm::vec3(0, 0, 1)) *
			glm::rotate(R(t.rotation.y), glm::vec3(0, 1, 0)) *
			glm::scale(glm::vec3(t.scale));

		//Overall MVP matrix
		glm::mat4 mvp = p * v * m;

		glUniformMatrix4fv(
			glGetUniformLocation(shader, "MVPMatrix"),
			1, GL_FALSE, glm::value_ptr(mvp)
		);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ibo);

		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);

		std::cout << "rendering" << std::endl;
	}
}
