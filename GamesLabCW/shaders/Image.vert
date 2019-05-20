uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform vec2 position;

layout (location = 0) in vec4 in_Position;

out vec2 TexCoords;

void main()
{
    TexCoords = in_Position.zw;

    gl_Position = vec4((in_Position.xy) + position, 0.0, 1.0);
}