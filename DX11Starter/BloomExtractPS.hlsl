#include "ShaderStructsFunctions.hlsli"

cbuffer ExternalData : register(b0)
{
	float bloomThreshold;
}

//define all texture related resources
Texture2D pixel : register(t0); // t for texture
SamplerState samplerState : register(s0); //s for sampler

float4 main(VertexToPixel_PostProcess input) : SV_TARGET
{
	float4 pixelColor = pixel.Sample(samplerState, input.uv);

	//calculates the brightness/luminance of pixels and compare with the threshold
	float luminance = dot(pixelColor.rgb, float3(0.2126, 0.7152, 0.0722));	//luminance value

	// Return this pixel's color if its luminance is over the threshold value
	if (luminance >= bloomThreshold)
	{
		return pixelColor;
	}

	return float4(0, 0, 0, 1);
}