//TEXTURED - should a texture be used?
//N_AMBIENT - number of ambient lights

in vec4 v_vPosition;
in vec2 v_vTexcoord;
in vec3 v_vNormal;

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
	
	vec3 lightPos = vec3(0.0, 1.0, 0.0);
	vec3 lightDirection = normalize(lightPos - v_vPosition.xyz);
	vec3 lightColor = vec3(1.0, 0.0, 0.0);

	vec3 normal = normalize(v_vNormal);
	float diff = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diff * lightColor;

	colour *= vec4(ambient + diffuse, 1.0 );
}
