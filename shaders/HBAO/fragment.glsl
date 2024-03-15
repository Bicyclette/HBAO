#version 410 core

#define M_PI 3.1415926535897932384626433832795

out float HBAO;

in VS_OUT
{
	vec2 texCoords;
} fs_in;

const int MAX_DIRECTIONS = 128;
uniform vec3 directions[MAX_DIRECTIONS];
uniform int Nd;
uniform int Ns;
uniform float R;
uniform float angle_bias;

uniform sampler2D rvec_texture;
uniform isampler2D randStep_texture;
uniform sampler2D fViewPos;
uniform sampler2D fViewNormal;

uniform mat4 proj;

uniform float screenWidth;
uniform float screenHeight;

void main()
{
	if(texture(fViewPos, fs_in.texCoords).a == 0.0f)
	{
		HBAO = 1.0f;
		return;
	}

	vec2 repeat = vec2(screenWidth / 4.0f, screenHeight / 4.0f);

	vec3 viewPos = texture(fViewPos, fs_in.texCoords).xyz;
	vec3 viewNormal = texture(fViewNormal, fs_in.texCoords).xyz;
	vec3 rvec = texture(rvec_texture, fs_in.texCoords * repeat).xyz;
	int randStep = texture(randStep_texture, fs_in.texCoords * repeat).r;

	vec3 tangent = normalize(rvec - viewNormal * dot(rvec, viewNormal));
	vec3 bitangent = cross(viewNormal, tangent);
	mat3 TBN = mat3(tangent, bitangent, viewNormal);

	int num_steps = Ns + randStep;
	float ray_step = R / float(num_steps);

	float n = float(Nd) * 0.5f;
	HBAO = 0.0f;
	for(int i = 0; i < Nd; ++i)
	{
		// compute tangent angle
		vec3 direction = TBN * directions[i];
		float t = atan(direction.z / length(direction.xy)); // tangent angle

		for(int j = 0; j < num_steps; ++j)
		{
			float ratio = j * ray_step;
			vec3 dir_sample = viewPos + (direction * R * ratio);
			vec4 screenPos = proj * vec4(dir_sample, 1.0f);
			vec3 viewPos_sample = ((screenPos.xyz / screenPos.w) + 1.0f) / 2.0f;
			vec3 Si = texture(fViewPos, viewPos_sample.xy).xyz;
			
			vec3 D = Si - viewPos;

			// compute length of (Si - P)
			float r = length(D);
			
			if(r > R) { continue; }

			// compute elevation angle
			float alpha = atan(D.z / length(D.xy));

			// deduce horizon angle
			float h = max(t, alpha);

			// compute occlusion
			float W = max(0.0f, 1.0f - pow((r / R), 2.0f));
			float ao = sin(h) - sin(t + angle_bias);
			HBAO += (ao * W) / (n * M_PI);
		}
	}

	HBAO = 1.0f - HBAO;
}