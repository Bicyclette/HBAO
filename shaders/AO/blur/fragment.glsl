#version 410 core

out float AO;

in VS_OUT
{
	vec2 texCoords;
} fs_in;

uniform sampler2D fAO;

void main()
{
	vec2 texelSize = 1.0f / textureSize(fAO, 0);
	float res = 0.0f;
	for(int i = -2; i <= 2; ++i)
	{
		for(int j = -2; j < 2; ++j)
		{
			res += texture(fAO, fs_in.texCoords + texelSize * vec2(i, j)).r;
		}
	}
	AO = res / 16.0f;
}