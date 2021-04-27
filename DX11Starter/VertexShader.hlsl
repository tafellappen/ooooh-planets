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
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in future assignments).

	//output.position = float4(input.position + offset, 1.0f);
	//output.position = mul(worldMatrix, float4(input.position, 1.0f));
	matrix wvp = mul(mul(projection, view), world);
	output.position = mul(wvp, float4(input.position, 1.0f));

	// Pass the color through 
	// - The values will be interpolated per-pixel by the rasterizer
	// - We don't need to alter it here, but we do need to send it to the pixel shader
	output.color = colorTint;

	output.uv = input.uv; //we have to remember to set everything in the VertexToPixel in both shaders

	output.normal = mul((float3x3)world, input.normal);

	//for point lights - get the worldspace position of this vertex
	output.worldPos = mul(input.position, world);

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}