uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TextureCoord;
layout (location = 2) in vec3 in_Normal;
layout (location = 3) in vec3 in_Tangent;

out vec3 v_vPosition;
out vec2 v_vTexcoord;
out vec3 v_vNormal;
out mat3 v_mTBN;
out mat4 v_mModel;

void main()
{
    v_mModel = modelMatrix;
	v_vPosition = vec3(modelMatrix * vec4(in_Position, 1.0));
	v_vTexcoord = in_TextureCoord;

//	mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
	
	// Calculate normal (used when there's no normal mapping)
    v_vNormal = normalize(vec3(modelMatrix * vec4(in_Normal, 0.0)));
	
	// Calculate TBN matrix for normal mapping
	vec3 tangent = normalize(vec3(modelMatrix * vec4(in_Tangent, 0.0)));
    tangent = normalize(tangent - dot(tangent, v_vNormal) * v_vNormal);
    vec3 bitangent = cross(v_vNormal, tangent);
	v_mTBN = mat3(tangent, bitangent, v_vNormal);

	// Calculate the MVP position of this vertex for drawing
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4( in_Position, 1.0 );
}
