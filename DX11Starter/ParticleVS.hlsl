#include "ShaderStructsFunctions.hlsli"


cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	//float specularExponent;

	//float3 camWorldPos;

	//float3 offset;
	matrix world;

	matrix view;
	matrix projection;
}



// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel_Particle main(VertexShaderInput_Particle input)
{
	// Set up output struct
	VertexToPixel_Particle output;

	//calculate output position
	matrix viewProj = mul(projection, view);
	output.position = mul(viewProj, float4(input.position, 1.0f));

	//pass uv
	output.uv = input.uv;
	output.color = input.color;

	return output;
}