uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform vec2 position;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TextureCoord;

out vec2 TexCoords;

void main()
{
	TexCoords = in_TextureCoord;
	
	// Calculate the MVP position of this vertex for drawing
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( in_Position, 1.0 );
}