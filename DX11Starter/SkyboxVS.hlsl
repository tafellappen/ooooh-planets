#include "ShaderStructsFunctions.hlsli"

cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix projection;
}

VertexToPixel_Sky main(VertexShaderInput input)
{
	VertexToPixel_Sky output;

	matrix viewNoTranslation = view;

	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	matrix vp = mul(projection, viewNoTranslation);
	output.position = mul(vp, float4(input.position, 1.0f));

	output.position.z = output.position.w;

	output.sampleDir = input.position;

	return output;
}