//TEXTURED - should a texture be used?

in vec2 TexCoords;
out vec4 color;

uniform sampler2D texSampler;
uniform vec4 flatColour;

void main()
{
	//Sample texture if one is used, otherwise use the flat colour
	#ifdef TEXTURED
		color = texture( texSampler, TexCoords );
	#else
		color = flatColour;
	#endif
}  