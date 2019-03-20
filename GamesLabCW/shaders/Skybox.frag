out vec4 colour;

in vec3 v_vPosition;

uniform samplerCube cubeSampler;

void main()
{
    colour = vec4(texture(cubeSampler, v_vPosition).rgb, 1.0);
}
