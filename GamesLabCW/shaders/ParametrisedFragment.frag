//TEXTURED - should a texture be used?
//N_AMBIENT - number of ambient lights

in vec2 v_vTexcoord;

out vec4 colour;

uniform sampler2D texSampler;
uniform vec4 flatColour;

struct AmbientLight
{
	vec3 colour;
	float intensity;
};
uniform AmbientLight ambientLights[N_AMBIENT];

void main()
{
	//Sample texture if one is used, otherwise use the flat colour
	#ifdef TEXTURED
		colour = texture( texSampler, v_vTexcoord );
	#else
		colour = flatColour;
	#endif

	//Apply ambient lights
	vec3 ambient = vec3( 0.0 );
	for (int i = 0; i < N_AMBIENT; i++)
		ambient += ambientLights[i].intensity * ambientLights[i].colour;
	colour *= vec4( ambient, 1.0 );
}
