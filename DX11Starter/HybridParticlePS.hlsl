cbuffer perMaterial : register(b0)
{
	float4 Color;
}

struct VertexToPixel
{
	float2 uv				: TEXCOORD;
	float4 position			: SV_POSITION; // The world position of this vertex
	float4 color			: COLOR;
};

//texture for the particles
Texture2D Texture			: register(t0);
SamplerState BasicSampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
	return Texture.Sample(BasicSampler, input.uv) * input.color;
}