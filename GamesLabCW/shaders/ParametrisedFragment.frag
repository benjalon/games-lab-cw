#version 330 core

in vec2 v_vTexcoord;

out vec4 colour;

uniform sampler2D texSampler;
uniform vec4 flatColour;

void main()
{
	//Sample texture if one is used, otherwise use the flat colour
	#ifdef TEXTURED
		colour = texture( texSampler, v_vTexcoord );
	#else
		colour = flatColour;
	#endif
}
