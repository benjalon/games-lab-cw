/**
 * Input.h
 * Defines the interface for obtaining user input.
 */

#pragma once

#include <string>
#include <unordered_set>

#include "GameEngine.h"
#include "Vector2.h"

namespace game::input
{
	//Key representation type
	using Input = int;

	//Current cursor position
	extern Vector2 cursor_pos;

	//Set of inputs just pressed
	extern std::unordered_set<Input> pressed;

	//Set of inputs just released
	extern std::unordered_set<Input> released;

	//Set of inputs currently held down
	extern std::unordered_set<Input> held;


	//Tests if pressed inputs contain the given input
	bool is_pressed(Input i);

	//Tests if released inputs contain the given input
	bool is_released(Input i);

	//Tests if held inputs contain the given input
	bool is_held(Input i);


	//Input codes (derived from GLFW)

	constexpr Input KEY_SPACE = 32;
	constexpr Input KEY_APOSTROPHE = 39;
	constexpr Input KEY_COMMA = 44;
	constexpr Input KEY_MINUS = 45;
	constexpr Input KEY_PERIOD = 46;
	constexpr Input KEY_SLASH = 47;
	constexpr Input KEY_0 = 48;
	constexpr Input KEY_1 = 49;
	constexpr Input KEY_2 = 50;
	constexpr Input KEY_3 = 51;
	constexpr Input KEY_4 = 52;
	constexpr Input KEY_5 = 53;
	constexpr Input KEY_6 = 54;
	constexpr Input KEY_7 = 55;
	constexpr Input KEY_8 = 56;
	constexpr Input KEY_9 = 57;
	constexpr Input KEY_SEMICOLON = 59;
	constexpr Input KEY_EQUAL = 61;
	constexpr Input KEY_A = 65;
	constexpr Input KEY_B = 66;
	constexpr Input KEY_C = 67;
	constexpr Input KEY_D = 68;
	constexpr Input KEY_E = 69;
	constexpr Input KEY_F = 70;
	constexpr Input KEY_G = 71;
	constexpr Input KEY_H = 72;
	constexpr Input KEY_I = 73;
	constexpr Input KEY_J = 74;
	constexpr Input KEY_K = 75;
	constexpr Input KEY_L = 76;
	constexpr Input KEY_M = 77;
	constexpr Input KEY_N = 78;
	constexpr Input KEY_O = 79;
	constexpr Input KEY_P = 80;
	constexpr Input KEY_Q = 81;
	constexpr Input KEY_R = 82;
	constexpr Input KEY_S = 83;
	constexpr Input KEY_T = 84;
	constexpr Input KEY_U = 85;
	constexpr Input KEY_V = 86;
	constexpr Input KEY_W = 87;
	constexpr Input KEY_X = 88;
	constexpr Input KEY_Y = 89;
	constexpr Input KEY_Z = 90;
	constexpr Input KEY_LEFT_BRACKET = 91;
	constexpr Input KEY_BACKSLASH = 92;
	constexpr Input KEY_RIGHT_BRACKET = 93;
	constexpr Input KEY_GRAVE_ACCENT = 96;
	constexpr Input KEY_WORLD_1 = 161;
	constexpr Input KEY_WORLD_2 = 162;

	constexpr Input KEY_ESCAPE = 256;
	constexpr Input KEY_ENTER = 257;
	constexpr Input KEY_TAB = 258;
	constexpr Input KEY_BACKSPACE = 259;
	constexpr Input KEY_INSERT = 260;
	constexpr Input KEY_DELETE = 261;
	constexpr Input KEY_RIGHT = 262;
	constexpr Input KEY_LEFT = 263;
	constexpr Input KEY_DOWN = 264;
	constexpr Input KEY_UP = 265;
	constexpr Input KEY_PAGE_UP = 266;
	constexpr Input KEY_PAGE_DOWN = 267;
	constexpr Input KEY_HOME = 268;
	constexpr Input KEY_END = 269;
	constexpr Input KEY_CAPS_LOCK = 280;
	constexpr Input KEY_SCROLL_LOCK = 281;
	constexpr Input KEY_NUM_LOCK = 282;
	constexpr Input KEY_PRINT_SCREEN = 283;
	constexpr Input KEY_PAUSE = 284;
	constexpr Input KEY_F1 = 290;
	constexpr Input KEY_F2 = 291;
	constexpr Input KEY_F3 = 292;
	constexpr Input KEY_F4 = 293;
	constexpr Input KEY_F5 = 294;
	constexpr Input KEY_F6 = 295;
	constexpr Input KEY_F7 = 296;
	constexpr Input KEY_F8 = 297;
	constexpr Input KEY_F9 = 298;
	constexpr Input KEY_F10 = 299;
	constexpr Input KEY_F11 = 300;
	constexpr Input KEY_F12 = 301;
	constexpr Input KEY_F13 = 302;
	constexpr Input KEY_F14 = 303;
	constexpr Input KEY_F15 = 304;
	constexpr Input KEY_F16 = 305;
	constexpr Input KEY_F17 = 306;
	constexpr Input KEY_F18 = 307;
	constexpr Input KEY_F19 = 308;
	constexpr Input KEY_F20 = 309;
	constexpr Input KEY_F21 = 310;
	constexpr Input KEY_F22 = 311;
	constexpr Input KEY_F23 = 312;
	constexpr Input KEY_F24 = 313;
	constexpr Input KEY_F25 = 314;
	constexpr Input KEY_KP_0 = 320;
	constexpr Input KEY_KP_1 = 321;
	constexpr Input KEY_KP_2 = 322;
	constexpr Input KEY_KP_3 = 323;
	constexpr Input KEY_KP_4 = 324;
	constexpr Input KEY_KP_5 = 325;
	constexpr Input KEY_KP_6 = 326;
	constexpr Input KEY_KP_7 = 327;
	constexpr Input KEY_KP_8 = 328;
	constexpr Input KEY_KP_9 = 329;
	constexpr Input KEY_KP_DECIMAL = 330;
	constexpr Input KEY_KP_DIVIDE = 331;
	constexpr Input KEY_KP_MULTIPLY = 332;
	constexpr Input KEY_KP_SUBTRACT = 333;
	constexpr Input KEY_KP_ADD = 334;
	constexpr Input KEY_KP_ENTER = 335;
	constexpr Input KEY_KP_EQUAL = 336;
	constexpr Input KEY_LEFT_SHIFT = 340;
	constexpr Input KEY_LEFT_CONTROL = 341;
	constexpr Input KEY_LEFT_ALT = 342;
	constexpr Input KEY_LEFT_SUPER = 343;
	constexpr Input KEY_RIGHT_SHIFT = 344;
	constexpr Input KEY_RIGHT_CONTROL = 345;
	constexpr Input KEY_RIGHT_ALT = 346;
	constexpr Input KEY_RIGHT_SUPER = 347;
	constexpr Input KEY_MENU = 348;

	constexpr Input MOUSE_BUTTON_1 = 0;
	constexpr Input MOUSE_BUTTON_2 = 1;
	constexpr Input MOUSE_BUTTON_3 = 2;
	constexpr Input MOUSE_BUTTON_4 = 3;
	constexpr Input MOUSE_BUTTON_5 = 4;
	constexpr Input MOUSE_BUTTON_6 = 5;
	constexpr Input MOUSE_BUTTON_7 = 6;
	constexpr Input MOUSE_BUTTON_8 = 7;
	constexpr Input MOUSE_BUTTON_LAST = MOUSE_BUTTON_8;
	constexpr Input MOUSE_BUTTON_LEFT = MOUSE_BUTTON_1;
	constexpr Input MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_2;
	constexpr Input MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3;
}
