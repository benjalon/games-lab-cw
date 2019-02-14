uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TextureCoord;
layout (location = 2) in vec3 in_Normal;

out vec2 v_vTexcoord;

void main()
{
	gl_Position = projectionMatrix * modelviewMatrix * vec4( in_Position, 1.0 );

	v_vTexcoord = in_TextureCoord;
}
