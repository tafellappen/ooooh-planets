#include "ShaderStructsFunctions.hlsli"


cbuffer ExternalData : register(b0)
{
	/*DirectionalLight light1;
	DirectionalLight light2;
	DirectionalLight light3;*/
	//DirectionalLight light4;

	//float specularExponent;
	float3 camWorldPos;
}

//define all texture related resources
Texture2D emissiveTexture : register(t0); // t for texture
//Texture2D normalMap : register(t1); // t for texture
SamplerState basicSampler : register(s0); //s for sampler
////pbr
Texture2D roughnessMap : register(t2); // t for texture
Texture2D mask : register(t3); // t for texture


// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	//need sampler state and input uv (this is testing line)
	//float4 textureColor = diffuseTexture.Sample(basicSampler, input.uv);
	//return textureColor;

	input.normal = normalize(input.normal);
	//float3 surfaceColor = input.color;
	float3 surfaceColor = emissiveTexture.Sample(basicSampler, input.uv).rgb;
	float3 maskValue = mask.Sample(basicSampler, input.uv).rgb;

	//gamma correcting it makes it really yellow
	surfaceColor = pow(surfaceColor, 2.2f); //un-gamma correct the diffuse texture, so that the end result of the gamma correction is ACTUALLY correct
	//return float4(surfaceColor, 1);
	
	//---PBR Stuff---
	//float roughness = roughnessMap.Sample(basicSampler, input.uv).r;
	//float metalness = metalnessMap.Sample(basicSampler, input.uv).r;
	/*float roughness = roughnessMap.Sample(basicSampler, input.uv).r;
	float metalness = metalnessMap.Sample(basicSampler, input.uv).r;

	float3 specularColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, metalness);*/

	//Actively deciding to not use the mask and to not gamma correct 
	//because this material is actually for red hot steel,
	//it doesn't look like a sun when you do both of these things or even do only one of them
	float3 finalColor = surfaceColor;//-maskValue;
		//CalcLightingColor(input.normal, input.worldPos, light1, surfaceColor, camWorldPos, roughness, metalness, specularColor) + //specularExponent) +
		//CalcLightingColor(input.normal, input.worldPos, light2, surfaceColor, camWorldPos, roughness, metalness, specularColor) + //specularExponent) +
		//CalcLightingColor(input.normal, input.worldPos, light3, surfaceColor, camWorldPos, roughness, metalness, specularColor) + //specularExponent) +
		//(ambientColor * surfaceColor);
	
	//return float4(light.Color, 1);
	finalColor = pow(finalColor, 1.0f / 2.2f); //gamma correction
	return float4(finalColor, 1);
}
