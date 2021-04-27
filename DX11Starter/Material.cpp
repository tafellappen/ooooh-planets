#include "Material.h"

Material::Material(DirectX::XMFLOAT4 tint, std::shared_ptr<SimplePixelShader> pixShader, std::shared_ptr<SimpleVertexShader> vertShader, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texSRV, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughSRV)
{
	colorTint = tint;
	pixelShader = pixShader;
	vertexShader = vertShader;
	textureSRV = texSRV;
	samplerState = sampler;
	metalnessSRV = metalSRV;
	roughnessSRV = roughSRV;
}

Material::Material(DirectX::XMFLOAT4 tint, std::shared_ptr<SimplePixelShader> pixShader, std::shared_ptr<SimpleVertexShader> vertShader, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texSRV, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normSRV,	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV,	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughSRV)
{
	colorTint = tint;
	pixelShader = pixShader;
	vertexShader = vertShader;
	textureSRV = texSRV;
	normalSRV = normSRV;
	samplerState = sampler;
	metalnessSRV = metalSRV;
	roughnessSRV = roughSRV;
}

Material::~Material()
{
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetTextureSRV()
{
	return textureSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetNormalSRV()
{
	return normalSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetRoughnessSRV()
{
	return roughnessSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetMetalnessSRV()
{
	return metalnessSRV;
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> Material::GetSamplerState()
{
	return samplerState;
}

float Material::GetSpecularExponent()
{
	return specularExponent;
}

//void Material::LoadTexture()
//{
//
//}
