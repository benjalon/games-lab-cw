uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform vec2 offset;
uniform vec4 color;

layout (location = 0) in vec4 in_Position;

out vec2 TexCoords;
out vec4 ParticleColor;

void main()
{
    float scale = 10.0f;
    TexCoords = in_Position.zw;
    ParticleColor = color;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4((in_Position.xy * scale) + offset, 0.0, 1.0);
}