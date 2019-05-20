/**
 * Vector2.cpp
 * Simple data structure representing a 2-dimensional vector.
 */

#include "Vector2.h"
#include "Vector3.h"

#include <cmath>

//Quick conversion to radians
constexpr double PI = 3.14159265358979323846;
#define R(X) ((X/180)*PI)

game::Vector2::Vector2() : x(0), y(0) {};

game::Vector2::Vector2(double x, double y) : x(x), y(y) {};

game::Vector2::Vector2(glm::vec2 vec) : x(vec.x), y(vec.y) {};

double game::Vector2::abs() const { return sqrt(x*x + y * y); }

glm::vec2 game::Vector2::ToGLM() { return glm::vec2(x, y); }

double game::Vector2::direction_r() const { return atan2(y, x); }

double game::Vector2::direction_d() const { return direction_r() * 180 / PI; }

void game::Vector2::set(double magnitude, double direction)
{
	x = magnitude * cos(direction);
	y = magnitude * sin(direction);
}

game::Vector2 game::Vector2::normal() const { return Vector2(y, -x); }

double game::Vector2::dot(const Vector2 &other) const { return this->x * other.x + this->y * other.y; }

game::Vector3 game::Vector2::direction_hv() const
{
	return Vector3(
		cos(R(y)) * sin(R(x)),
		sin(R(y)),
		cos(R(y)) * cos(R(x))
	);
}

game::Vector3 game::Vector2::direction_hv_right() const
{
	return Vector3(
		sin(R(x) - (PI/2)),
		0,
		cos(R(x) - (PI/2))
	);
}


game::Vector2 game::Vector2::operator+(const Vector2 &other)
{
	return Vector2(this->x + other.x, this->y + other.y);
}

game::Vector2 game::Vector2::operator-(const Vector2 &other)
{
	return Vector2(this->x - other.x, this->y - other.y);
}

game::Vector2& game::Vector2::operator+=(const Vector2 &other)
{
	this->x += other.x;
	this->y += other.y;
	return *this;
}

game::Vector2& game::Vector2::operator-=(const Vector2 &other)
{
	this->x -= other.x;
	this->y -= other.y;
	return *this;
}

game::Vector2 game::Vector2::operator*(const double &other)
{
	return Vector2(this->x * other, this->y * other);
}

game::Vector2::operator glm::vec2() const
{
	return glm::vec2(x, y);
}

std::ostream &game::operator<<(std::ostream &os, const Vector2 &v)
{
	os << v.x << "," << v.y;
	return os;
}
