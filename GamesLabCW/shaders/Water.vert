uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 cameraPosition;
uniform float delta;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TextureCoord;
layout (location = 2) in vec3 in_Normal;
layout (location = 3) in vec3 in_Tangent;

out mat4 v_mModel;
out vec3 v_vPosition;
out vec2 v_vTexcoord;
out vec3 v_vNormal;
out mat3 v_mTBN;

struct Wave
{
	float Amplitude;
	vec3 Direction;
	float Frequency;
	float Phase;
	float Sharpness;
};

float generateWave(Wave wave);
vec3 generateDifferential(Wave wave);

void main()
{
    v_mModel = modelMatrix;
	v_vPosition = (modelMatrix * vec4(in_Position, 1.0)).xyz;
	v_vTexcoord = in_TextureCoord;

	

	// Make waves
	Wave wave;
	wave.Amplitude = 1.25;
	wave.Direction = vec3(-1.0, 0.0, 0.0);
	wave.Frequency = 0.1;
	wave.Phase = 0.5;
	wave.Sharpness = 4.0;

	float yPos = generateWave(wave);
	vec3 diff = generateDifferential(wave);

	vec4 pos = vec4(in_Position, 1.0);
	pos.y = yPos;
	pos *= modelMatrix;

	vec3 eyePos = vec3(1.0, 1.0, -1.0);
	vec3 eyeDir = normalize(eyePos - pos.xyz);
	vec4 viewDir = vec4(normalize(cameraPosition - pos.xyz), 1.0);
	viewDir *= modelMatrix;


	vec3 normal = vec3(-diff.x , 1.0, -diff.z);
	normal.y = 1.0;

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    v_vNormal = normalize(normalMatrix * normal);

	vec3 tangent = normalize(normalMatrix * in_Tangent);
    tangent = normalize(tangent - dot(tangent, v_vNormal) * v_vNormal);

    vec3 bitangent = cross(v_vNormal, tangent);
	
    v_mTBN = transpose(mat3(tangent, bitangent, v_vNormal));  



	// Calculate the MVP position of this vertex for drawing
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * pos;
}

float generateWave(Wave wave) 
{
	float dp = dot(wave.Direction.x, in_Position.x) + dot(wave.Direction.z, in_Position.z);
	float dtOffset = dot(dp, wave.Frequency) + delta * wave.Phase;
	return wave.Amplitude * pow(dot(sin(dtOffset), 0.5) + 0.5, wave.Sharpness);
}

vec3 generateDifferential(Wave wave)
{
	float dp = dot(wave.Direction.x, in_Position.x) + dot(wave.Direction.z, in_Position.z);
	float dtOffset = dot(dp, wave.Frequency) + delta * wave.Phase;
	float waveReduction = 0.5 * wave.Sharpness * wave.Frequency * wave.Amplitude;

	float offset = waveReduction * pow(dot(sin(dtOffset), 0.5) + 0.5, wave.Sharpness - 1.0);
	float cosDTOffset = cos(dtOffset);
	float dp2 = dot(offset, cosDTOffset);

	return vec3(dot(dp2, wave.Direction.x), 0.0, dot(dp2, wave.Direction.z));
}