#version 410 core

#define M_PI 3.1415926535897932384626433832795

out vec4 color_response;

in VS_OUT
{
	vec2 fTexCoords;
	mat4 view;
	mat4 lightSpaceMatrix;
} fs_in;

struct DirectionalLight
{
	vec3 direction; // view space
	vec3 color;
	float intensity;
};

uniform sampler2D fViewPos;
uniform sampler2D fViewNormal;
uniform sampler2D fAlbedo;
uniform sampler2D fMetallicRoughness;
uniform sampler2D fDepth;
uniform sampler2D shadowMap;
uniform sampler2D fSSAO;
uniform sampler2D fHBAO;
uniform float shadowMap_bias;

uniform int draw_mode;
uniform int ao_mode;

uniform float cam_near;
uniform float cam_far;

uniform DirectionalLight directionalLight;

uniform vec3 camPos;

float linearize_depth(float d)
{
	return (2 * cam_near) / ((cam_near + cam_far) - d * (cam_far - cam_near));
}

float normal_distribution(vec3 wi, vec3 wh, vec3 n, float roughness) // GGX distribution
{
	float r2 = roughness * roughness;
	float n_dot_wh = max(dot(n, wh), 0.0f);
	float n_dot_wh2 = n_dot_wh * n_dot_wh;
	float denom = 1.0f + (r2 - 1.0f) * n_dot_wh2;
	return r2 / (M_PI * denom * denom);
}

vec3 fresnel_term(vec3 wi, vec3 wh, vec3 F0)
{
	float wi_dot_wh = max(dot(wi, wh), 0.0f);
	return F0 + (1.0f - F0) * pow((1.0f - max(0.0f, wi_dot_wh)), 5);
}

float GGX_schlick(vec3 w, vec3 n, float roughness)
{
	float k = roughness * sqrt(2.0f / M_PI);
	float n_dot_w = max(dot(n, w), 0.0f);
	return n_dot_w / (n_dot_w * (1.0f - k) + k);
}

float geometric_term(vec3 wi, vec3 wo, vec3 n, float roughness)
{
	return GGX_schlick(wi, n, roughness) * GGX_schlick(wo, n, roughness);
}

vec3 BRDF(vec3 wi, vec3 wo, vec3 wh, vec3 n, vec3 F0, vec3 albedo, float metallic, float roughness)
{
	float n_dot_wi = max(dot(n, wi), 0.0f);
	float n_dot_wo = max(dot(n, wo), 0.0f);

	// specular part
	float D = normal_distribution(wi, wh, n, roughness);
	vec3 F = fresnel_term(wi, wh, F0);
	float G = geometric_term(wi, wo, n, roughness);
	vec3 specular = (D * F * G) / (4 * n_dot_wi * n_dot_wo + 0.001);

	// diffuse part
	vec3 Kd = (vec3(1.0f) - F);
	Kd *= (1.0f - metallic);
	vec3 diffuse = Kd / M_PI;

	// result
	return diffuse * albedo + specular;
}

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 aces(vec3 x)
{
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec4 pbr()
{
	float opacity = texture(fAlbedo, fs_in.fTexCoords).a;
	if(opacity == 0.0f)
	{
		return vec4(1.0f);
	}
	vec3 albedo = texture(fAlbedo, fs_in.fTexCoords).rgb;
	vec3 normal = texture(fViewNormal, fs_in.fTexCoords).xyz;
	float metallic = texture(fMetallicRoughness, fs_in.fTexCoords).b;
	float roughness = texture(fMetallicRoughness, fs_in.fTexCoords).g;
	vec3 fPos_view = texture(fViewPos, fs_in.fTexCoords).xyz;

	// radiance
	vec3 F0 = vec3(0.16 * (1.0 - metallic) + albedo * metallic);
	vec3 wo = normalize(-fPos_view);
	vec3 wi = normalize(-directionalLight.direction);
	vec3 wh = normalize(wi + wo);
	vec3 brdf = BRDF(wi, wo, wh, normal, F0, albedo, metallic, roughness);
	float n_dot_wi = max(dot(normal, wi), 0.0f);
	vec3 radiance = directionalLight.color * directionalLight.intensity * brdf * n_dot_wi;

	return vec4(radiance, 1.0f);
}

float shadow_factor()
{
	// get fragment position in light space
	vec3 fView = texture(fViewPos, fs_in.fTexCoords).xyz;
	vec3 fWorld = vec3(inverse(fs_in.view) * vec4(fView, 1.0f));
	vec4 fLight = fs_in.lightSpaceMatrix * vec4(fWorld, 1.0f);
	vec3 projCoords = fLight.xyz / fLight.w;
	projCoords = (projCoords + 1.0f) / 2.0f;

	// get fragment's depth in light space
	float fragment_depth = projCoords.z;

	// sample shadow map depth in an area around current fragment position
	float ratio = 0.0f;
	vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
	for(int x = -2; x <= 2; ++x)
	{
		for(int y = -2; y <= 2; ++y)
		{
			float shadowMap_depth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadowMap_depth += shadowMap_bias;

			// shadow test
			if(fragment_depth > shadowMap_depth)
			{
				ratio += 1.0f;
			}
		}
	}

	return ratio / 25.0f;
}

void main()
{
	if(draw_mode == 0) // shaded
	{
		float ao;
		if(ao_mode == 0)
		{
			ao = texture(fSSAO, fs_in.fTexCoords).r;
		}
		else if(ao_mode == 1)
		{
			ao = texture(fHBAO, fs_in.fTexCoords).r;
		}
		else if(ao_mode == 2)
		{
			ao = 1.0f;
		}
		float shadow_ratio = shadow_factor();
		vec3 albedo = texture(fAlbedo, fs_in.fTexCoords).rgb;
		vec3 ambient = albedo * ao * 0.03f * directionalLight.intensity;
		float opacity = texture(fAlbedo, fs_in.fTexCoords).a;
		if(opacity == 0.0f)
		{
			ambient = albedo;
		}
		vec4 color = pbr() * ao;

		color_response = vec4(ambient + (1.0f - shadow_ratio) * color.rgb, 1.0f);

		// tone mapping
		color_response.rgb = aces(color_response.rgb);

		// gamma correction
		color_response.rgb = pow(color_response.rgb, vec3(1.0f/2.2f));
	}
	else if(draw_mode == 1) // depth
	{
		float depth = texture(fDepth, fs_in.fTexCoords).r;
		depth = linearize_depth(depth);
		color_response = vec4(vec3(depth), 1.0f);
	}
	else if(draw_mode == 2) // albedo
	{
		vec3 albedo = texture(fAlbedo, fs_in.fTexCoords).rgb;
		color_response = vec4(albedo, 1.0f);
	}
	else if(draw_mode == 3) // view position
	{
		vec3 fPos_view = texture(fViewPos, fs_in.fTexCoords).xyz;
		color_response = vec4(fPos_view, 1.0f);
	}
	else if(draw_mode == 4) // view normal
	{
		vec3 normalView = texture(fViewNormal, fs_in.fTexCoords).xyz;
		color_response = vec4(normalView, 1.0f);
	}
	else if(draw_mode == 5) // shadowMap
	{
		float shadowMapValue = texture(shadowMap, fs_in.fTexCoords).r;
		color_response = vec4(vec3(shadowMapValue), 1.0f);
	}
	else if(draw_mode == 6) // screen space ambient occlusion
	{
		float ssao = texture(fSSAO, fs_in.fTexCoords).r;
		color_response = vec4(vec3(ssao), 1.0f);
	}
	else if(draw_mode == 7) // screen space horizon based ambient occlusion
	{
		float hbao = texture(fHBAO, fs_in.fTexCoords).r;
		color_response = vec4(vec3(hbao), 1.0f);
	}
}