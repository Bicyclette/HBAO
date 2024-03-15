#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT
{
	vec3 fPosView;
	flat vec3 fNormalView;
	vec2 fTexCoords;
	mat3 TBN;
	mat3 normalMat;
} vs_out;

uniform mat4 modelView;
uniform mat4 proj;
uniform mat3 normalMat;

void main()
{
	// normalizing normal
	vec3 n = normalize(normal);

	// position
	gl_Position = proj * modelView * vec4(position, 1.0f);

	// compute TBN matrix
	vec3 T = normalize(normalMat * tangent);
	vec3 B = normalize(normalMat * bitangent);
	vec3 N = normalize(normalMat * n);

	// vertex shader output
	vs_out.fPosView = vec3(modelView * vec4(position, 1.0f));
	vs_out.fNormalView = normalize(normalMat * n);
	vs_out.fTexCoords = texCoords;
	vs_out.TBN = mat3(T, B, N);
	vs_out.normalMat = normalMat;
}
