#include "ShaderStructsFunctions.hlsli"


cbuffer ExternalData : register(b0)
{
	DirectionalLight light1;
	DirectionalLight light2;
	DirectionalLight light3;
	//DirectionalLight light4;
	float3 ambientColor;

	float specularExponent;

	float3 camWorldPos;
}

//define all texture related resources
Texture2D albedoTexture : register(t0); // t for texture
Texture2D normalMap : register(t1); // t for texture
SamplerState basicSampler : register(s0); //s for sampler
//pbr
Texture2D roughnessMap : register(t2); // t for texture
Texture2D metalnessMap : register(t3); // t for texture


float4 main(VertexToPixel_NormalMap input) : SV_TARGET
{
	float3 surfaceColor = albedoTexture.Sample(basicSampler, input.uv).rgb;
	surfaceColor = pow(surfaceColor, 2.2f); //un-gamma correct the albedo texture, so that the end result of the gamma correction is ACTUALLY correct

	//sample the normal map
	//map the normals from the stored [0, 1] range to the [-1, 1] range
	//todo: make this a method in the shader include file
	float3 unpackedNormal = normalMap.Sample(basicSampler, input.uv).rgb * 2 - 1; //wait, why is this not using mul()
	float3 N = normalize(input.normal);
	float3 T = normalize(input.tangent);
	T = normalize(T - N * dot(T, N)); //Gram-Schmidt orthogonalization
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	//return float4(unpackedNormal, 1);

	input.normal = normalize(mul(unpackedNormal, TBN));

	//---PBR Stuff---
	float roughness = roughnessMap.Sample(basicSampler, input.uv).r;
	float metalness = metalnessMap.Sample(basicSampler, input.uv).r;

	float3 specularColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, metalness);


	float3 finalColor =
		CalcLightingColor(input.normal, input.worldPos, light1, surfaceColor, camWorldPos, roughness, metalness, specularColor) + //specularExponent) +
		CalcLightingColor(input.normal, input.worldPos, light2, surfaceColor, camWorldPos, roughness, metalness, specularColor) + //specularExponent) +
		CalcLightingColor(input.normal, input.worldPos, light3, surfaceColor, camWorldPos, roughness, metalness, specularColor) + //specularExponent) +
		(ambientColor * surfaceColor);

	//return float4(light.Color, 1);
	finalColor = pow(finalColor, 1.0f / 2.2f); //gamma correction
	return float4(finalColor, 1);
}
