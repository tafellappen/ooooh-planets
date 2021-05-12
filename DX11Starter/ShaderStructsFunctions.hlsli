#ifndef __GGP_SHADER_INCLUDES__ // Each file needs a unique identifier here!
#define __GGP_SHADER_INCLUDES__

//----- STRUCTS -----
// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 position		: POSITION;     // XYZ position
	float3 tangent		: TANGENT;
	float3 normal		: NORMAL;
	float3 uv			: TEXCOORD;
};

struct VertexShaderInput_Particle
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float4 color		: COLOR;
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 color		: COLOR;        // RGBA color
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION;
};

struct VertexToPixel_NormalMap
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 color		: COLOR;        // RGBA color
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
};

struct VertexToPixel_Sky
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float3 sampleDir	: DIRECTION;
};

struct VertexToPixel_Particle
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
	float4 color		: TEXCOORD1;
};

struct VertexToPixel_PostProcess
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float2 uv			: TEXCOORD0;
};

struct DirectionalLight
{
	int Type;
	float3 Color;
	float Intensity;
	float3 DirectionOrPos;
};

//----- FUCNTIONS -----
float3 Diffuse(float3 normal, float3 dirToLight)
{
	float dotProd = dot(normal, dirToLight);
	return saturate(dotProd);
}

float3 Specular(float3 normal, float3 worldPosition, float specExp, float3 cameraWorldPos, float3 lightDirection)
{
	float3 view = normalize(cameraWorldPos - worldPosition);
	float3 reflection = reflect(lightDirection, normal);

	float3 spec = pow(saturate(dot(reflection, view)), specExp);
	return spec;
}

//float Normal() {
//
//}

// float3 CalcLightingColor(float3 normal, float3 worldPosition, DirectionalLight light, float3 surfaceColor, float specExp, float3 cameraWorldPos)
// {
// 	float3 directionToLight; //= GetDirectionToLight(light);
// 	float3 directionOfLight;
// 	//normalized direction to the light
// 	if (light.Type == 0) //directional light
// 	{
// 		directionToLight = normalize(-light.DirectionOrPos);
// 		directionOfLight = normalize(light.DirectionOrPos);

// 	}
// 	else if (light.Type == 1) //point light
// 	{
// 		directionToLight = light.DirectionOrPos - worldPosition;
// 		directionOfLight = normalize(-directionToLight);
// 	}

// 	float diffuseVal = Diffuse(normal, directionToLight);
// 	float specVal = Specular(normal, worldPosition, specExp, cameraWorldPos, directionOfLight);
// 	specVal *= any(diffuseVal); //this is for when the normals cause "self-shadowing"

// 	float3 finalColor = (diffuseVal * light.Color * surfaceColor) + specVal;
// 	// +
// 	//(ambientColor * surfaceColor);

// 	return finalColor;
// }

//----- PBR -----

//constants
// Make sure to place these at the top of your shader(s) or shader include file
// - You don’t necessarily have to keep all the comments; they’re here for your reference
// The fresnel value for non-metals (dielectrics)
// Page 9: "F0 of nonmetals is now a constant 0.04"
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
static const float F0_NON_METAL = 0.04f;
// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal
// Handy to have this as a constant
static const float PI = 3.14159265359f;


//Lambert diffuse BRDF- Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float DiffusePBR(float3	normal, float3	dirToLight)
{
	return saturate(dot(normal, dirToLight));
}
// Calculates diffuse amount based on energy conservation
// 
// diffuse - Diffuse amount
// specular - Specular color(including light color)
// metalness - surface metalness amount
// 
// Metals should have an albedo of(0, 0, 0)...mostly
// See slide 65: http ://blog.selfshadow.com/publications/s2014-shading-course / hoffman / s2014_pbs_physics_math_slides.pdf
float3 DiffuseEnergyConserve(float diffuse, float3 specular, float metalness)
{
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}
// GGX (Trowbridge - Reitz)
// 
// a - Roughness
// h - Half vector
// n - Normal
//  
// D(h, n) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1) ^ 2
float SpecDistribution(float3 n, float3 h, float roughness)
{
	// Pre-calculations
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!
		// ((n dot h) ^ 2 * (a ^ 2 - 1) + 1)
	float denomToSquare = NdotH2 * (a2 - 1) + 1;
	// Can go to zero if roughness is 0 and NdotH is 1

	// Final value
	return a2 / (PI * denomToSquare * denomToSquare);
}
// Fresnel term - Schlick approx.
//  
// v - View vector
// h - Half vector
// f0 - Value when l = n(full specular color)
// 
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h)) ^ 5
float3 Fresnel(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
	float VdotH = saturate(dot(v, h));
	// Final value
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}
// Geometric Shadowing - Schlick - GGX(based on Schlick	- Beckmann)
	// - k is remapped to a / 2, roughness remapped to (r+1)/2
	// 
	// n - Normal
	// v - View vector
	// 
	// G(l,v,h)
float GeometricShadowing(float3 n, float3 v, float3 h, float roughness)
{
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	return NdotV / (NdotV * (1 - k) + k);
}
// Microfacet BRDF (Specular)
// 
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - part of the denominator are canceled out by numerator (see below)
// 
// D() -Spec Dist - Trowbridge - Reitz(GGX)
// F() - Fresnel- Schlick approx
// G() - Geometric Shadowing -Schlick -GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float metalness, float3 specColor)
{
	// Other vectors
	float3 h = normalize(v + l);
	// Grab various functions
	float D = SpecDistribution(n, h, roughness);
	float3 F = Fresnel(v, h, specColor);
	float G = GeometricShadowing(n, v, h, roughness) * GeometricShadowing(n, l, h, roughness);
	// Final formula
	// Denominator dot products partially canceled by G()!
	// See page 16: http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
	return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
}


float3 CalcLightingColor(float3 normal, float3 worldPosition, DirectionalLight light, float3 surfaceColor, float3 cameraWorldPos, float roughness, float metalness, float3 specularColor)
{
	float3 directionToLight; //= GetDirectionToLight(light);
	float3 directionOfLight;
	//normalized direction to the light
	if (light.Type == 0) //directional light
	{
		directionToLight = normalize(-light.DirectionOrPos);
		directionOfLight = normalize(light.DirectionOrPos);

	}
	else if (light.Type == 1) //point light
	{
		directionToLight = light.DirectionOrPos - worldPosition;
		directionOfLight = normalize(-directionToLight);
	}
	float3 view = normalize(cameraWorldPos - worldPosition); //direction to the camera

	float diffuseVal = DiffusePBR(normal, directionToLight);
	float3 specVal = MicrofacetBRDF(normal, directionToLight, view, roughness, metalness, specularColor);
	specVal *= any(diffuseVal);

	//conserving energy
	float3 balancedDiffuse = DiffuseEnergyConserve(diffuseVal, specVal, metalness);

	float3 total = (balancedDiffuse * surfaceColor + specVal) * light.Intensity * light.Color;
	return total;
}
#endif