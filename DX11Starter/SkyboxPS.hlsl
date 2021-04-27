#include "ShaderStructsFunctions.hlsli"

//cbuffer ExternalData : register(b0)
//{
//	DirectionalLight light1;
//	DirectionalLight light2;
//	DirectionalLight light3;
//	//DirectionalLight light4;
//	float3 ambientColor;
//
//	float specularExponent;
//
//	float3 camWorldPos;
//}

//define all texture related resources
TextureCube textureCube : register(t0); // t for texture
SamplerState samplerState : register(s0); //s for sampler

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
	//return float4(1, 0, 0, 0);
	return textureCube.Sample(samplerState, input.sampleDir);
	//return surfaceColor;
}