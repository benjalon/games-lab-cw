/**
 * Input.cpp
 * Defines the interface for obtaining user input.
 */

#include "Input.h"

#include "Utility.h"

namespace game::input
{
	Vector2 cursor_pos;
	std::unordered_set<Input> pressed;
	std::unordered_set<Input> released;
	std::unordered_set<Input> held;

	bool is_pressed(Input i)
	{
		return utility::contains(pressed, i);
	}

	//Tests if released inputs contain the given input
	bool is_released(Input i)
	{
		return utility::contains(released, i);
	}

	//Tests if held inputs contain the given input
	bool is_held(Input i)
	{
		return utility::contains(held, i);
	}
}
