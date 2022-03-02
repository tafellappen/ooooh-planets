#pragma once
#include <DirectXMath.h>
#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include <iostream>


#include "SimpleShader.h"
#include "Material.h"
#include "Transform.h"
#include "Camera.h"

//__declspec(align(16))
struct ParticleData
{
	float EmitTime;
	DirectX::XMFLOAT3 StartPosition;
	DirectX::XMFLOAT3 StartVelocity;
};

struct EmitterData
{
	float ParticlesPerSecond;
	float ParticleLifetime;
	float MaxParticles;
};

enum EmitterShape
{
	Point,
	RectPrism,
	Sphere
};

class HybridEmitter
{
public:
	//std::shared_ptr<SimpleVertexShader> vs,
	//std::shared_ptr<SimplePixelShader> ps,
	HybridEmitter(
		float particlesEmitPerSec,
		float particleLifetime,
		float maxParticles,
		DirectX::XMFLOAT4 color,
		DirectX::XMFLOAT3 startVelocity,
		std::shared_ptr<SimpleVertexShader> vs,
		std::shared_ptr<SimplePixelShader> ps,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture
	);
	~HybridEmitter();

	void Update(float dt, float currentTime);
	void Draw(Camera* camera, float currentTime);

	std::shared_ptr<Transform> GetTransform();
	void SetRectBounds(float x, float y, float z);
	void SetEmitterShape(EmitterShape emitShape);
	//std::shared_ptr<SimpleVertexShader> GetVS() { return vs; }
	//std::shared_ptr<SimplePixelShader> GetPS() { return ps; }
private:
	int firstLivingIndex;
	int firstDeadIndex;
	int livingCount;
	int maxParticles;
	std::shared_ptr<ParticleData> particles; //pointer to the first element of the array

	float particlesEmitPerSec;
	float secBetweenParticleEmit;
	float timeSinceLastEmit;

	float particleLifetime;

	EmitterShape emitterShape;
	DirectX::XMFLOAT3 emissionRectDimensions;
	DirectX::XMFLOAT4 color;


	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

	//std::shared_ptr<SimpleVertexShader> vs;
	//std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleDataSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;

	//Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

	std::shared_ptr<Transform> transform;

	void EmitParticle(float emitTime);
	void UpdateSingleParticle(float currentTime, int index);
};

