#include "ShaderStructsFunctions.hlsli"

cbuffer externalData : register(b0)
{
	int debugWireframe;
};


// Textures and such
Texture2D particle		: register(t0);
SamplerState trilinear	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel_Particle input) : SV_TARGET
{
	return(1, 1, 1, 1);
	// Sample texture and combine with input color
	float4 color = particle.Sample(trilinear, input.uv) * input.color;

	// Return either particle color or white (for debugging)
	return lerp(color, float4(1, 1, 1, 0.25f), debugWireframe);
}