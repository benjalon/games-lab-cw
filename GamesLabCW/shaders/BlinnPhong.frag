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

struct DirectionalLight
{
	vec3 colour;
	vec3 position;
};
uniform DirectionalLight directionalLights[N_DIRECTIONAL];

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

	vec3 normal = normalize(v_vNormal);

	//Apply directional lights
	//vec3 lightPos = vec3(1.0, 1.0, 0.0);
	//vec3 lightColor = vec3(1.0, 0.0, 0.0);
	vec3 diffuse = vec3(0.0);
	for (int i = 0; i < N_DIRECTIONAL; i++)
	{
		vec3 lightDirection = normalize(directionalLights[i].position - v_vPosition.xyz);
		float diff = max(dot(normal, lightDirection), 0.0);
		diffuse += diff * directionalLights[i].colour;
	}
	

//	// TODO MAKE THIS NOT HARD CODED
//	vec3 cameraPosition = vec3(0.0, 2.0, 2.0);
//
//	// Calculate specular light
//	float specularStrength = 1;
//	vec3 viewDirection = normalize(cameraPosition - v_vPosition.xyz);
//	vec3 reflectDirection = reflect(-lightDirection, normal);
//	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
//	vec3 specular = specularStrength * spec * vec3(0.0, 1.0, 0.0); 

//	colour *= vec4((ambient + diffuse + specular), 1.0);
	colour *= vec4((ambient + diffuse), 1.0);
}
