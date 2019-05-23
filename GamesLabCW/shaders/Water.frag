//N_AMBIENT - number of ambient lights
//N_DIRECTIONAL - number of directional lights
//N_POINT - number of point lights

out vec4 colour;

in vec3 v_vPosition;
in vec2 v_vTexcoord;
in vec3 v_vNormal;
in mat3 v_mTBN;
in vec3 v_vEyePos;

uniform vec3 cameraPosition;
uniform samplerCube cubeSampler;
uniform sampler2D normalSampler;
uniform sampler2D texSampler;
uniform float shininess;

struct AmbientLight
{
	vec3 colour;
	float intensity;
	bool on;
};
uniform AmbientLight ambientLights[N_AMBIENT];

struct DirectionalLight
{
	vec3 colour;
	float intensity;
	vec3 direction;
	bool on;
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
	bool on;
};
uniform PointLight pointLights[N_POINT];

void main()
{
	float waterReflectivity = 0.7;

	// Bump mapping
	vec3 normal = texture(normalSampler, v_vTexcoord).xyz;
	normal = normalize(normal * 2.0 - 1.0);
	vec3 cameraPos = v_mTBN * cameraPosition;
	vec3 pos = v_mTBN * v_vPosition;

	vec3 viewDirection = normalize(cameraPos - pos);
	
	// Fresnel
	float airToWater = 0.02037;
	float fresnel = airToWater + (1.0 - airToWater) * pow(1.0 - dot(viewDirection, normal), 5.0);

	// Reflection
	vec3 reflection = reflect(-viewDirection, normal);

	// Refraction
	vec3 refraction = refract(-viewDirection, normal, 1.0 / 1.33);
	vec4 refractionColor = texture(cubeSampler, refraction);

	vec3 eyePos = v_mTBN * v_vEyePos;

	// Water base coloring
	float facing = 1.0 - max(dot(eyePos, normal), 0.0);
	vec4 deepColor = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 shallowColor = vec4(0.0, 0.5, 0.5, 1.0);

	// Blend the colors
	vec4 waterColor = mix(deepColor, shallowColor, facing);
	vec4 reflectionColor = texture(cubeSampler, reflection) * waterReflectivity;

	// HDR
	reflectionColor.xyz *= (1.0 + reflectionColor.w * 3.0);

	vec4 c = waterColor + reflectionColor * fresnel + refractionColor * (1.0 - fresnel);

	//vec4 baseColor = vec4(0.0, 0.5, 1.0, 0.8);
	
	vec3 ambient = vec3(0.0);
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	//Apply ambient lights
	for (int i = 0; i < N_AMBIENT; i++)
	{
		if (!ambientLights[i].on) 
		{
			continue;
		}
		ambient += ambientLights[i].intensity * ambientLights[i].colour;
	}
		
	// Apply directional lights
	for (int i = 0; i < N_DIRECTIONAL; i++)
	{
		if (!directionalLights[i].on) 
		{
			continue;
		}

		vec3 lightDirection;
		#ifdef NORMAL_MAPPED
			lightDirection = normalize(v_mTBN * directionalLights[i].direction);
		#else
			lightDirection = normalize(directionalLights[i].direction);
		#endif
		
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
		if (!pointLights[i].on) 
		{
			continue;
		}

		vec3 lightPosition;
		#ifdef NORMAL_MAPPED
			lightPosition = v_mTBN * pointLights[i].position;
		#else
			lightPosition = pointLights[i].position;
		#endif

		vec3 lightDirection = normalize(lightPosition - pos);
		
		// Calculate diffuse for this light source
		float diff = max(dot(normal, lightDirection), 0.0);
		
		// Calculate specular for this light source
		vec3 blinnHalfDirection = normalize(lightDirection + viewDirection); // Used to apply blinn correction to specular
		float spec = pow(max(dot(normal, blinnHalfDirection), 0.0), shininess);

		// Calculate attenuation (light drop-off) for this light source
		float dist = length(pointLights[i].position - pos);
		float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].exponent * (dist * dist));

		diffuse += (pointLights[i].intensity * diff * pointLights[i].colour) * attenuation;
		specular += (pointLights[i].intensity * spec * pointLights[i].colour) * attenuation; 
	}
	
	// Combine lights into blinn-phong lighting model
	vec3 blinnPhong = ambient + diffuse + specular;

	// Apply lighting
	vec4 result = c * vec4(blinnPhong, 1.0);

	// Apply gamma correction (fixes monitor color biases)
	vec3 gamma = vec3(1.0 / 2.2);
	result.xyz = pow(result.xyz, gamma).xyz;

	colour = result;




//    vec3 incidence = normalize(pos - cameraPos);
//    vec3 reflection = reflect(incidence, normalize(normal));
//	vec4 reflectionColor =vec4(texture(cubeSampler, reflection).rgb, 1.0) * waterReflectivity;
//
//	vec4 baseColor = vec4(0.0, 0.5, 1.0, 0.8);
//
//	colour = baseColor;//reflectionColor * baseColor;
}
