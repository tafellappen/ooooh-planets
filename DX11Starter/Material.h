#pragma once
#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <memory>
#include "WICTextureLoader.h"

class Material
{
public:
	Material(
		DirectX::XMFLOAT4 tint, 
		std::shared_ptr<SimplePixelShader> pixShader, 
		std::shared_ptr<SimpleVertexShader> vertShader, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texSRV, //todo: organize this better
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughSRV

	);
	Material(
		DirectX::XMFLOAT4 tint, 
		std::shared_ptr<SimplePixelShader> pixShader, 
		std::shared_ptr<SimpleVertexShader> vertShader, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texSRV, //todo: organize this better
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normSRV,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughSRV

	);
	~Material();

	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTextureSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetNormalSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetRoughnessSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetMetalnessSRV();
	Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSamplerState();
	float GetSpecularExponent();


private:

	//void LoadTexture();

	DirectX::XMFLOAT4 colorTint;
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalnessSRV;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture2;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;


	float specularExponent; //should this just be a plain float? 
};

