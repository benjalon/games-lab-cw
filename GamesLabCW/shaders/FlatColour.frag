#version 330 core

out vec4 colour;

uniform vec4 flatColour;

void main()
{
	colour = flatColour;
}
