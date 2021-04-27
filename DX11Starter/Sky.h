#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr
#include <memory>


#include "Mesh.h"
#include "Material.h"
#include "SimpleShader.h"
#include "WICTextureLoader.h"
#include "Camera.h"



class Sky
{
public:
	Sky(
		const char* meshPath,
		std::wstring ddsTexturePath,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, 
		std::wstring vertShaderPath,
		std::wstring pixShaderPath,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context
	);
	~Sky();

	void DrawSky(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera);

	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> GetRasterizerOptions();
	std::shared_ptr<Mesh> GetMesh();

private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMapTexSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthBufferComparisonType;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerOptions;

	std::shared_ptr<Mesh> skyMesh;
	std::shared_ptr<SimpleVertexShader> vertShader;
	std::shared_ptr<SimplePixelShader> pixShader;

};

