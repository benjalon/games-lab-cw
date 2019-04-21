uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

layout (location = 0) in vec3 in_Position;

out vec3 v_vPosition;

void main()
{
	v_vPosition = in_Position;

	vec4 pos = projectionMatrix * viewMatrix * vec4( in_Position, 1.0 );
	gl_Position = pos.xyww;
}
