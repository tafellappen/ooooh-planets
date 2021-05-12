#include "ShaderStructsFunctions.hlsli"

cbuffer ExternalData : register(b0)
{
	float intensityLevel;
}

//define all texture related resources
Texture2D originalPixels	: register(t0); // t for texture
Texture2D bloomedPixels	: register(t1); // t for texture
SamplerState samplerState : register(s0); //s for sampler

float4 main(VertexToPixel_PostProcess input) : SV_TARGET
{
	float4 totalColor = originalPixels.Sample(samplerState, input.uv);
	totalColor += bloomedPixels.Sample(samplerState, input.uv) * intensityLevel;

	return totalColor;
}