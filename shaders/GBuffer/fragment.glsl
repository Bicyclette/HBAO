#version 410 core

layout (location = 0) out vec4 fragPosView;
layout (location = 1) out vec4 fragNormalView;
layout (location = 2) out vec4 fragAlbedo;
layout (location = 3) out vec4 fragMetallicRoughness;

in VS_OUT
{
	vec3 fPosView;
	flat vec3 fNormalView;
	vec2 fTexCoords;
	mat3 TBN;
	mat3 normalMat;
} fs_in;

struct Material
{
	vec3 albedo;
	float roughness;
	float metallic;
	float opacity;
	sampler2D tex_albedo;
	bool has_tex_albedo;
	sampler2D tex_roughness;
	bool has_tex_roughness;
	sampler2D tex_metallic;
	bool has_tex_metallic;
	sampler2D tex_normal;
	bool has_tex_normal;
};

uniform Material material;

void main()
{
	float opacity = material.opacity;
	vec3 albedo = material.albedo;
	if(material.has_tex_albedo)
	{
		albedo = texture(material.tex_albedo, fs_in.fTexCoords).xyz;
		opacity = texture(material.tex_albedo, fs_in.fTexCoords).a;
	}
	vec3 normalView = normalize(fs_in.fNormalView);
	if(material.has_tex_normal)
	{
		vec3 normal = texture(material.tex_normal, fs_in.fTexCoords).xyz;	// tangent space
		normal = normalize(normal * 2.0f - 1.0f);
		normalView = normalize(fs_in.TBN * normal);							// view space
	}
	float metallic = material.metallic;
	if(material.has_tex_metallic)
	{
		metallic = texture(material.tex_metallic, fs_in.fTexCoords).b;
	}
	float roughness = material.roughness;
	if(material.has_tex_roughness)
	{
		roughness = texture(material.tex_roughness, fs_in.fTexCoords).g;
	}

	fragPosView = vec4(fs_in.fPosView, 1.0f);
	fragNormalView = vec4(normalView, 1.0f);
	fragAlbedo = vec4(albedo, opacity);
	fragMetallicRoughness = vec4(0.0f, roughness, metallic, 1.0f);
}
