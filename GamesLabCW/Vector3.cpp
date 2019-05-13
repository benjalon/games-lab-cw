/**
 * Vector3.cpp
 * Simple data structure representing a 3-dimensional vector.
*/

#include "Vector3.h"

#include <cmath>

game::Vector3::Vector3() : x(0), y(0), z(0) {};

game::Vector3::Vector3(double x, double y, double z) : x(x), y(y), z(z) {};

game::Vector3::Vector3(glm::vec3 vec) : x(vec.x), y(vec.y), z(vec.z) {};

double game::Vector3::abs() const { return sqrt(x*x + y*y + z*z); }

glm::vec3 game::Vector3::ToGLM() { return glm::vec3(x, y, z); }

game::Vector3 game::Vector3::operator+(const Vector3 &other)
{
	return Vector3(this->x + other.x, this->y + other.y, this->z + other.z);
}

game::Vector3 game::Vector3::operator-(const Vector3 &other)
{
	return Vector3(this->x - other.x, this->y - other.y, this->z - other.z);
}

game::Vector3& game::Vector3::operator+=(const Vector3 &other)
{
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;
	return *this;
}

game::Vector3& game::Vector3::operator-=(const Vector3 &other)
{
	this->x -= other.x;
	this->y -= other.y;
	this->z -= other.z;
	return *this;
}

game::Vector3 game::Vector3::operator*(const double &other)
{
	return Vector3(this->x * other, this->y * other, this->z * other);
}

game::Vector3::operator glm::vec3() const
{
	return glm::vec3(x, y, z);
}

std::ostream &game::operator<<(std::ostream &os, const Vector3 &v)
{
	os << v.x << "," << v.y << "," << v.z;
	return os;
}
