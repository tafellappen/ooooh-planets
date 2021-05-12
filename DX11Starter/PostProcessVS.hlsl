#include "ShaderStructsFunctions.hlsli"

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel_PostProcess main(uint id : SV_VertexID)
{
	// Set up output struct
	VertexToPixel_PostProcess output;

	// Calculate the UV to via the ID
	output.uv = float2(
		(id << 1) & 2,  // id % 2 * 2
		id & 2);

	// Adjust the position based on the UV
	output.position = float4(output.uv, 0, 1);
	output.position.x = output.position.x * 2 - 1;
	output.position.y = output.position.y * -2 + 1;

	return output;
}