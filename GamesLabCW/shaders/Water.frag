out vec4 colour;

in vec3 v_vPosition;
in vec3 v_vNormal;

uniform vec3 cameraPosition;
uniform samplerCube cubeSampler;

void main()
{
	float waterReflectivity = 0.7;
    vec3 incidence = normalize(v_vPosition - cameraPosition);
    vec3 reflection = reflect(incidence, normalize(v_vNormal));
	vec4 reflectionColor =vec4(texture(cubeSampler, reflection).rgb, 1.0) * waterReflectivity;

	vec4 baseColor = vec4(0.0, 0.5, 1.0, 0.8);

	colour = reflectionColor * baseColor;
}
