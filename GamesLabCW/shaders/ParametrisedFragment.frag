//TEXTURED - should a texture be used?
//N_AMBIENT - number of ambient lights

in vec4 v_vPosition;
in vec2 v_vTexcoord;
in vec3 v_vNormalMatrix;

out vec4 colour;

uniform sampler2D texSampler;
uniform vec4 flatColour;
uniform float shininess;

uniform vec3 cameraPosition;

struct AmbientLight
{
	vec3 colour;
	float intensity;
};
uniform AmbientLight ambientLights[N_AMBIENT];

struct DirectionalLight
{
	vec3 colour;
	float intensity;
	vec3 position;
};
uniform DirectionalLight directionalLights[N_DIRECTIONAL];

struct PointLight
{
	vec3 colour;
	float intensity;
	vec3 position;
	float constant;
	float linear;
	float exponent;
};
uniform PointLight pointLights[N_POINT];

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

	// Calculate lighting parameters
	vec3 normal = normalize(v_vNormalMatrix);
	vec3 viewDirection = normalize(cameraPosition - v_vPosition.xyz);

	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	// Apply directional lights
	for (int i = 0; i < N_DIRECTIONAL; i++)
	{
		vec3 lightDirection = normalize(directionalLights[i].position - v_vPosition.xyz);
		
		// Calculate diffuse for this light source
		float diff = max(dot(normal, lightDirection), 0.0);
		diffuse += directionalLights[i].intensity * diff * directionalLights[i].colour;
		
		// Calculate specular for this light source
		vec3 blinnHalfDirection = normalize(lightDirection + viewDirection); // Used to apply blinn correction to specular
//		vec3 reflectDirection = reflect(-lightDirection, normal);
		float spec = pow(max(dot(normal, blinnHalfDirection), 0.0), shininess);
		specular += directionalLights[i].intensity * spec * directionalLights[i].colour; 
	}

	// Apply point lights
	for (int i = 0; i < N_POINT; i++)
	{
		vec3 lightDirection = normalize(pointLights[i].position - v_vPosition.xyz);
		
		// Calculate diffuse for this light source
		float diff = max(dot(normal, lightDirection), 0.0);
		
		// Calculate specular for this light source
		vec3 blinnHalfDirection = normalize(lightDirection + viewDirection); // Used to apply blinn correction to specular
//		vec3 reflectDirection = reflect(-lightDirection, normal);
		float spec = pow(max(dot(normal, blinnHalfDirection), 0.0), shininess);

		// Calculate attenuation (light drop-off) for this light source
		float dist = length(pointLights[i].position - v_vPosition.xyz);
		float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].exponent * (dist * dist));

		diffuse += (pointLights[i].intensity * diff * pointLights[i].colour) * attenuation;
		specular += (pointLights[i].intensity * spec * pointLights[i].colour) * attenuation; 
	}
	
	// Combine lights into blinn-phong lighting model
	colour *= vec4((ambient + diffuse + specular), 1.0);
}