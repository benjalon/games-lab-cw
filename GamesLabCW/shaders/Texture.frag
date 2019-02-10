#version 330 core

in vec2 v_vTexcoord;

out vec4 colour;

uniform sampler2D texSampler;

void main()
{
	colour = texture( texSampler, v_vTexcoord );
}
