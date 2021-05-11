#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include "Mesh.h"
#include "Transform.h"
#include "Entity.h"
#include "Camera.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"
#include "WICTextureLoader.h"
#include <vector>
class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateBasicGeometry();

	void ResizeAllPostProcessResources();
	void ResizeOnePostProcessResource(Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv, float renderTargetScale = 1.0f);

	void DrawSky();

	void BloomExtract();
	void SingleDirectionBlur(
		float renderTargetScale,
		DirectX::XMFLOAT2 blurDirection,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> target,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceTexture);
	void BloomCombine();

	//void DrawMesh(Mesh* mesh);
	//void CreateLights();
	
	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//    Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//Microsoft::WRL::ComPtr<ID3D11Buffer> constantBufferVS;

	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<Entity*> entities;
	std::vector<std::shared_ptr<Material>> materials;
	
	std::vector<DirectionalLight> lights;

	// Shaders and shader-related constructs
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShaderNormal;
	std::shared_ptr<SimpleVertexShader> vertexShaderNormal;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	std::shared_ptr<Camera> camera; 
	DirectX::XMFLOAT3 ambientColor;


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture1Albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture1Normal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture1Metal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture1Rough;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture2Albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture2Normal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture2Metal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture2Rough;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sunEmmisive;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sunNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sunMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sunRough;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

	float bloomThreshold;
	float bloomLevelIntensity;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler; // Clamp sampler for post processing

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV;		// Allows us to render to a texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV;		// Allows us to sample from the same texture

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> bloomExtractRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bloomExtractSRV;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> blurHorizontalRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blurHorizontalSRV;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> blurVerticalRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blurVerticalSRV;

	SimpleVertexShader* ppVS;
	SimplePixelShader* bloomExtractPS;
	SimplePixelShader* bloomCombinePS;
	SimplePixelShader* gaussianBlurPS;

	//SimplePixelShader* emissivePS;

	//skybox stuff
	std::shared_ptr<Sky> skybox;
	//std::shared_ptr<SimplePixelShader> skyPS;
	//std::shared_ptr<SimpleVertexShader> skyVS;
	//std::shared_ptr<Mesh> skyMesh;
	//Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRasterState;


	//for testing
	//Transform transform;


};

