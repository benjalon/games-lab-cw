//HAS_BONES - should bones be applied

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

const int MAX_BONES = 70; // Max number of bones
uniform mat4 bones[MAX_BONES]; // Bone transformations 

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TextureCoord;
layout (location = 2) in vec3 in_Normal;
layout (location = 3) in vec3 in_Tangent;
layout (location = 4) in ivec4 in_BoneID;
layout (location = 5) in vec4 in_Weight;

out mat4 v_mModel;
out vec3 v_vPosition;
out vec2 v_vTexcoord;
out vec3 v_vNormal;
out mat3 v_mTBN;

void main()
{
	v_mModel = modelMatrix;
	v_vTexcoord = in_TextureCoord;

	vec4 pos = vec4(in_Position, 1.0);
	vec4 normal = vec4(in_Normal, 1.0);
	
	#ifdef HAS_BONES
		mat4 boneTransform = bones[in_BoneID[0]] * in_Weight[0];
		boneTransform += bones[in_BoneID[1]] * in_Weight[1];
		boneTransform += bones[in_BoneID[2]] * in_Weight[2];
		boneTransform += bones[in_BoneID[3]] * in_Weight[3];

		pos = boneTransform * pos;
		normal = boneTransform * normal;
	#endif

	v_vPosition = (modelMatrix * pos).xyz;

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    v_vNormal = normalize(normalMatrix * normal.xyz);

	vec3 tangent = normalize(normalMatrix * in_Tangent);
    tangent = normalize(tangent - dot(tangent, v_vNormal) * v_vNormal);

    vec3 bitangent = cross(v_vNormal, tangent);
	
    v_mTBN = transpose(mat3(tangent, bitangent, v_vNormal));  

	// Calculate the MVP position of this vertex for drawing
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * pos;
}
