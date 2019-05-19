in vec2 TexCoords;
in vec4 ParticleColor;
out vec4 color;

uniform sampler2D texSampler;

void main()
{
    color = ParticleColor;
}  