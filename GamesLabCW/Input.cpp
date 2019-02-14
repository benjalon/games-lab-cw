/**
 * Input.cpp
 * Defines the interface for obtaining user input.
 */

#include "Input.h"

namespace game::input
{
	Vector2 cursor_pos;
	std::unordered_set<Input> pressed;
	std::unordered_set<Input> released;
	std::unordered_set<Input> held;
}
