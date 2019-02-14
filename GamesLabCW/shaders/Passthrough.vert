uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TextureCoord;
layout (location = 2) in vec3 in_Normal;

out vec4 v_vPosition;
out vec2 v_vTexcoord;
out vec3 v_vNormalMatrix;

void main()
{
	gl_Position = projectionMatrix * modelviewMatrix * vec4( in_Position, 1.0 );

	v_vPosition = gl_Position;
	v_vTexcoord = in_TextureCoord;
	v_vNormalMatrix = mat3(transpose(inverse(modelMatrix))) * in_Normal;
}
