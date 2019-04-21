uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TextureCoord;
layout (location = 2) in vec3 in_Normal;
layout (location = 3) in vec3 in_Tangent;

out mat4 v_mModel;
out vec3 v_vPosition;
out vec2 v_vTexcoord;
out vec3 v_vNormal;
out mat3 v_mTBN;

void main()
{
	v_mModel = modelMatrix;
	v_vPosition = (modelMatrix * vec4(in_Position, 1.0)).xyz;
	v_vTexcoord = in_TextureCoord;

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    v_vNormal = normalize(normalMatrix * in_Normal);

	vec3 tangent = normalize(normalMatrix * in_Tangent);
    tangent = normalize(tangent - dot(tangent, v_vNormal) * v_vNormal);

    vec3 bitangent = cross(v_vNormal, tangent);
	
    v_mTBN = transpose(mat3(tangent, bitangent, v_vNormal));  

	// Calculate the MVP position of this vertex for drawing
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( in_Position, 1.0 );
}
