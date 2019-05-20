in vec2 TexCoords;
out vec4 color;

uniform sampler2D texSampler;

void main()
{
    color = texture(texSampler, TexCoords);
}  