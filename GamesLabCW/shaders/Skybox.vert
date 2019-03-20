uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout (location = 0) in vec3 in_Position;

out vec3 v_vPosition;

void main()
{
	v_vPosition = in_Position;

	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( in_Position, 1.0 );
}
