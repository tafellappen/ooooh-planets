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
VertexToPixel_NormalMap main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel_NormalMap output;

	matrix wvp = mul(mul(projection, view), world);
	output.position = mul(wvp, float4(input.position, 1.0f));

	// Pass the color through 
	output.color = colorTint;

	output.uv = input.uv; //we have to remember to set everything in the VertexToPixel in both shaders

	output.normal = mul((float3x3)world, input.normal);
	output.tangent = mul((float3x3)world, input.tangent);

	//for point lights - get the worldspace position of this vertex
	output.worldPos = mul(world, float4(input.position, 1.0f)).xyz;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}