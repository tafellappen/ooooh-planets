#pragma once
#include <DirectXMath.h>
#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include <iostream>
#include <random>

#include "SimpleShader.h"
#include "Material.h"
#include "Transform.h"
#include "Camera.h"


enum class EmitterShape
{
	Point,
	RectPrism,
	Sphere
};

enum class ParticleMoveType
{
	Constant,
	PhysicsSimulation
};

struct ParticleData
{
	float EmitTime;
	//float DeathTime;
	float Lifespan;

	DirectX::XMFLOAT3 StartPosition;
	DirectX::XMFLOAT3 StartVelocity;
	DirectX::XMFLOAT3 StartDirection;

	DirectX::XMFLOAT4 StartColor;
	DirectX::XMFLOAT4 EndColor;

	float Acceleration;
	//DirectX::XMFLOAT3 InitialForces;
};

//yeah im not really sure what to do here. maybe i will pull this out into its own class
struct Colors
{
	DirectX::XMFLOAT4 Red = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
};

struct EmitterData
{
	// shape and dimensions
	EmitterShape EmitShape = EmitterShape::Point;
	float SphereRadius = 0.0f;
	DirectX::XMFLOAT3 RectDimensions = {};

	//general spawning info
	float ParticlesPerSecond = 0.0f;
	float ParticleLifetime = 0.0f;
	int MaxParticles = 0.0f;

	//variables for things used while particles are alive
	DirectX::XMFLOAT4 StartColor = {};
	DirectX::XMFLOAT4 EndColor = {};
	DirectX::XMFLOAT3 StartVelocity = {};
	DirectX::XMFLOAT3 StartDirection = {};
	float StartSpeed = 0.0f; //for when direction will be figured out later
	//DirectX::XMFLOAT3 StartForce = {};


	//stuff for rendering
	std::shared_ptr<SimpleVertexShader> VS;
	std::shared_ptr<SimplePixelShader> PS;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Texture;


	Microsoft::WRL::ComPtr<ID3D11Device> Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;

};

class HybridEmitter
{
public:
	HybridEmitter(EmitterData* emitData);

	~HybridEmitter();

	void Update(float dt, float currentTime);
	void Draw(Camera* camera, float currentTime);

	std::shared_ptr<Transform> GetTransform();
	void SetRectBounds(float x, float y, float z);
	void SetEmitterShape(EmitterShape emitShape);

	EmitterData* GetEmitterData();

	//std::shared_ptr<SimpleVertexShader> GetVS() { return vs; }
	//std::shared_ptr<SimplePixelShader> GetPS() { return ps; }
private:
	DirectX::XMFLOAT3* initialForces; //array of all of the forces that need to be applied when the particle spawns
	DirectX::XMFLOAT3* constantForces; //array of all of the forces that need to be applied for the duration of the particle's life


	EmitterData* emitterData;

	int firstLivingIndex;
	int firstDeadIndex;
	int livingCount;
	//int maxParticles;
	ParticleData* particles; //pointer to the first element of the array

	//float particlesEmitPerSec;
	float secBetweenParticleEmit;
	float timeSinceLastEmit;

	//float particleLifetime;

	EmitterShape emitterShape;
	//DirectX::XMFLOAT3 emissionRectDimensions;
	//DirectX::XMFLOAT4 color;

	std::default_random_engine generator;
	//float sphereRadius;
	//DirectX::XMFLOAT3 constantStartVelocity;

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

	void CreateMissingDefaultValues(); //maybe wont need this???

	void EmitParticle(float emitTime);
	DirectX::XMFLOAT3 RandomRectPosition();
	void UpdateSingleParticle(float currentTime, int index);

	/// <summary>
	/// picks a random point within a sphere
	/// </summary>
	DirectX::XMFLOAT3 RandomSphereLocation(DirectX::XMFLOAT3 direction);
	DirectX::XMFLOAT3 RandomSphericalDirection();

	float CalcAcceleration();
};

