/**
 * Vector3.h
 * Simple data structure representing a 3-dimensional vector.
 */

#pragma once

#include <glm/glm.hpp>
#include <iostream>

namespace game
{
	struct Vector3
	{
		double x;	//x (i) component
		double y;	//y (j) component
		double z;	//z (k) component

		//Default constructor
		Vector3();

		//Constructs a Vector2 with the given components
		Vector3(double x, double y, double z);

		//Returns the magnitude of this Vector3
		double abs() const;


		//Definitions of algebraic operations

		Vector3 operator+(const Vector3 &other);
		Vector3 operator-(const Vector3 &other);

		Vector3& operator+=(const Vector3 &other);
		Vector3& operator-=(const Vector3 &other);

		Vector3 operator*(const double &other);

		//Allows implicit casting to glm
		operator glm::vec3() const;

		friend std::ostream &operator<<(std::ostream &os, const Vector3 &v);
	};
}
