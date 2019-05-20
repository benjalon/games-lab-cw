uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform float scale;
uniform vec3 offset;
uniform vec4 color;

layout (location = 0) in vec4 in_Position;

out vec2 TexCoords;
out vec4 ParticleColor;

void main()
{
    TexCoords = in_Position.zw;
    ParticleColor = color;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4((in_Position.xyz * scale) + offset, 1.0);
}