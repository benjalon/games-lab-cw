out vec4 colour;

in vec3 v_vPosition;
in vec3 v_vNormal;

uniform vec3 cameraPosition;
uniform samplerCube cubeSampler;

void main()
{
    vec3 incidence = normalize(v_vPosition - cameraPosition);
    vec3 reflection = reflect(incidence, normalize(v_vNormal));
    colour = vec4(texture(cubeSampler, reflection).rgb, 1.0);
}
