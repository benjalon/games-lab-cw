/**
 * Vector2.h
 * Simple data structure representing a 2-dimensional vector.
 */

#pragma once

#include <glm/glm.hpp>
#include <iostream>

namespace game
{
	struct Vector3;
	struct Vector2
	{
		double x;	//x (i) component
		double y;	//y (j) component

		//Default constructor
		Vector2();

		//Constructs a Vector2 with the given components
		Vector2(double x, double y);

		Vector2(glm::vec2 vec) : x(vec.x), y(vec.y) {};

		//Returns the magnitude of this Vector2
		double abs() const;

		glm::vec2 game::Vector2::ToGLM();

		//Returns the direction of this Vector2 in radians
		double direction_r() const;

		//Returns the direction of this Vector2 in degrees
		double direction_d() const;

		//Sets this Vector2 to the given magnitude and direction
		void set(double magnitude, double direction);

		//Returns a normal to this Vector2
		Vector2 normal() const;

		//Returns the dot product of this and the given Vector2
		double dot(const Vector2 &other) const;

		//Returns the 3D direction vector of this horizontal&vertical orientation (assumes degrees)
		Vector3 direction_hv() const;

		//Returns the 3D direction vector perpendicularly right to directionHV()
		Vector3 direction_hv_right() const;


		//Definitions of algebraic operations

		Vector2 operator+(const Vector2 &other);
		Vector2 operator-(const Vector2 &other);

		Vector2& operator+=(const Vector2 &other);
		Vector2& operator-=(const Vector2 &other);

		Vector2 operator*(const double &other);

		//Allows implicit casting to glm
		operator glm::vec2() const;

		friend std::ostream &operator<<(std::ostream &os, const Vector2 &v);
	};
}
