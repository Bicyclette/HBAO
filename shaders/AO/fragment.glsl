#version 410 core

out float AO;

in VS_OUT
{
	vec2 texCoords;
} fs_in;

const int MAX_SAMPLES = 128;
uniform vec3 samples[MAX_SAMPLES];
uniform int sample_count;
uniform float hemi_radius;

uniform sampler2D rvec_texture;
uniform sampler2D fViewPos;
uniform sampler2D fViewNormal;

uniform mat4 proj;

uniform float screenWidth;
uniform float screenHeight;

void main()
{
	vec2 repeat = vec2(screenWidth / 4.0f, screenHeight / 4.0f);
	float bias = 0.025f;

	vec3 viewPos = texture(fViewPos, fs_in.texCoords).xyz;
	vec3 viewNormal = texture(fViewNormal, fs_in.texCoords).xyz;
	vec3 rvec = texture(rvec_texture, fs_in.texCoords * repeat).xyz;

	vec3 tangent = normalize(rvec - viewNormal * dot(rvec, viewNormal));
	vec3 bitangent = cross(viewNormal, tangent);
	mat3 TBN = mat3(tangent, bitangent, viewNormal);

	float occlusion = 0.0f;
	for(int i = 0; i < sample_count; ++i)
	{
		vec3 s = TBN * samples[i];
		s = viewPos + s * hemi_radius;
		vec4 screenPos = proj * vec4(s, 1.0f);
		vec3 ao_sample = ((screenPos.xyz / screenPos.w) + 1.0f) / 2.0f;
		float s_depth = texture(fViewPos, ao_sample.xy).z;

		float rangeCheck = smoothstep(0.0, 1.0, hemi_radius / abs(viewPos.z - s_depth));
		if(s_depth >= (s.z + bias))
		{
			occlusion += 1.0f * rangeCheck;
		}
	}

	occlusion = 1.0f - (occlusion / sample_count);
	occlusion = pow(occlusion, 1.5f);

	AO = occlusion;
}