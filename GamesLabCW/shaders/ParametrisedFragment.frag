//TEXTURED - should a texture be used?
//NORMAL_MAPPED - should a normal map be used?
//N_AMBIENT - number of ambient lights
//N_DIRECTIONAL - number of directional lights
//N_POINT - number of point lights

in vec4 v_vPosition;
in vec2 v_vTexcoord;
in vec3 v_vNormal;
in mat4 v_mMVP;

out vec4 colour;

uniform sampler2D texSampler;
uniform sampler2D normalSampler;
uniform vec4 flatColour;
uniform float shininess;
uniform mat4 modelMatrix;
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
	vec3 direction;
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
	vec3 baseColour = vec3(0.0);
	#ifdef TEXTURED
		baseColour = texture( texSampler, v_vTexcoord ).xyz;
	#else
		baseColour = flatColour.xyz;
	#endif

	#ifdef NORMAL_MAPPED
		baseColour = texture( normalSampler, v_vTexcoord ).xyz;
	#endif

	//Apply ambient lights
	vec3 ambient = vec3( 0.0 );
	for (int i = 0; i < N_AMBIENT; i++)
		ambient += ambientLights[i].intensity * ambientLights[i].colour;

	// Calculate lighting parameters
	vec3 normal = normalize(v_vNormal);
	vec3 viewDirection = normalize(cameraPosition - v_vPosition.xyz);

	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	// Apply directional lights
	for (int i = 0; i < N_DIRECTIONAL; i++)
	{
		vec3 lightDirection = normalize(directionalLights[i].direction);
		
		// Calculate diffuse for this light source
		float diff = max(dot(normal, lightDirection), 0.0);
		diffuse += directionalLights[i].intensity * diff * directionalLights[i].colour;
		
		// Calculate specular for this light source
		vec3 blinnHalfDirection = normalize(lightDirection + viewDirection); // Used to apply blinn correction to specular
		float spec = pow(max(dot(normal, blinnHalfDirection), 0.0), shininess);
		specular += directionalLights[i].intensity * spec * directionalLights[i].colour; 
	}

	// Apply point lights
	for (int i = 0; i < N_POINT; i++)
	{
		vec4 lightPosition = v_mMVP * vec4(pointLights[i].position, 1.0);
		vec3 lightDirection = normalize(lightPosition.xyz - v_vPosition.xyz);
		
		// Calculate diffuse for this light source
		float diff = max(dot(normal, lightDirection), 0.0);
		
		// Calculate specular for this light source
		vec3 blinnHalfDirection = normalize(lightDirection + viewDirection); // Used to apply blinn correction to specular
		float spec = pow(max(dot(normal, blinnHalfDirection), 0.0), shininess);

		// Calculate attenuation (light drop-off) for this light source
		float dist = length(pointLights[i].position - v_vPosition.xyz);
		float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].exponent * (dist * dist));

		diffuse += (pointLights[i].intensity * diff * pointLights[i].colour) * attenuation;
		specular += (pointLights[i].intensity * spec * pointLights[i].colour) * attenuation; 
	}
	
	// Combine lights into blinn-phong lighting model
	vec3 blinnPhong = ambient + diffuse + specular;

	// Apply lighting
	vec3 result = baseColour * blinnPhong;

	// Apply gamma correction (fixes monitor color biases)
	vec3 gamma = vec3(1.0 / 2.2);
	result = pow(result, gamma);

	colour = vec4(result, 1.0);
}