in vec2 TexCoords;
out vec4 color;

uniform sampler2D texSampler;

void main()
{
	vec4 texColor =  texture(texSampler, TexCoords);
	if(texColor.a < 0.1)
		discard;
    color = texColor;
}  