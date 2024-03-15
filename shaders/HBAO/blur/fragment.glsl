#version 410 core

out float AO;

in VS_OUT
{
	vec2 texCoords;
} fs_in;

const float PI = 3.14159265f;

uniform sampler2D fAO;

uniform float sigma;
uniform int direction;

float gaussian(float sigma, int x)
{
	vec2 texelSize = 1.0f / textureSize(fAO, 0);
	float gaussLeft = 1.0f / sqrt(2.0f * PI * sigma);
	float twoSigmaSquared = 2.0f * sigma * sigma;

	float g = gaussLeft * exp(-(x*x)/twoSigmaSquared);
	return g;
}

void main()
{
	vec2 texelSize = 1.0f / textureSize(fAO, 0);
	
	// kernel size
	int halfSize = int(3 * sigma);
	
	vec2 blurDirection = (direction == 0) ? vec2(1.0f, 0.0f) : vec2(0.0f, 1.0f);

	//AO = 0.0f;
	float sum = 0.0f;
	for(int i = -halfSize; i <= halfSize; ++i)
	{
		vec2 uv = fs_in.texCoords + (texelSize * blurDirection * i);
		float g = gaussian(sigma, i);
		sum += g;
		AO += texture(fAO, uv).r * g;
	}
	AO /= sum;
}