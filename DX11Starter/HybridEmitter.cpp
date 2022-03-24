#include "HybridEmitter.h"

HybridEmitter::HybridEmitter(EmitterData* emitData)
{
	this->emitterData = emitData;
	this->vs = emitterData->VS;
	this->ps = emitterData->PS;
	this->texture = emitterData->Texture;
	this->context = emitterData->Context;

	{
		//ensure direction is normalized
		DirectX::XMFLOAT3 dir = emitterData->StartDirection;
		DirectX::XMVECTOR normDirection = DirectX::XMVector3Normalize(DirectX::XMVectorSet(dir.x, dir.y, dir.z, 1.0f));
		DirectX::XMStoreFloat3(&emitterData->StartDirection, normDirection);
	}


	timeSinceLastEmit = 0.0f;
	livingCount = 0;
	firstLivingIndex = 0;
	firstDeadIndex = 0;
	transform = std::make_shared<Transform>();

	particles = new ParticleData[emitterData->MaxParticles];

	// Create an index buffer for particle drawing
	// indices as if we had two triangles per particle
	unsigned int* indices = new unsigned int[emitterData->MaxParticles * 6];
	int indexCount = 0;
	for (int i = 0; i < emitterData->MaxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	// Regular (static) index buffer
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * emitterData->MaxParticles * 6;
	emitterData->Device->CreateBuffer(&ibDesc, &indexData, indexBuffer.GetAddressOf());
	delete[] indices;

	//structured buffer for passing the particle array data to the gpu
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(ParticleData);
	desc.ByteWidth = sizeof(ParticleData) * emitterData->MaxParticles;
	emitterData->Device->CreateBuffer(&desc, 0, particleDataBuffer.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = emitterData->MaxParticles;
	emitterData->Device->CreateShaderResourceView(particleDataBuffer.Get(), &srvDesc, particleDataSRV.GetAddressOf());
}

HybridEmitter::~HybridEmitter()
{
	delete[] particles;
	//delete vs; //i guess my asset manager already cleans these up, so i dont need these here?
	//delete ps;
}

void HybridEmitter::Update(float dt, float currentTime)
{
	this->secBetweenParticleEmit = 1.0f / emitterData->ParticlesPerSecond;

	//"first" as in the first one you would get to as the index counts up
	//this would also make "first" living the "oldest" living particle
	//bool contiguousLiving = firstLivingIndex < firstDeadIndex;
	//bool nonContigLiving = firstLivingIndex > firstDeadIndex;

	if (livingCount > 0)
	{
		//managing the circular buffer
		if (firstLivingIndex < firstDeadIndex) //if contiguous living
		{
			for (int i = firstLivingIndex; i < firstDeadIndex; i++)
			{
				UpdateSingleParticle(currentTime, i);
			}
		}
		else if (firstLivingIndex > firstDeadIndex) //if not contiguous living
		{
			for (int i = firstLivingIndex; i < emitterData->MaxParticles; i++)
			{
				UpdateSingleParticle(currentTime, i);
			}
			for (int i = 0; i < firstDeadIndex; i++)
			{
				UpdateSingleParticle(currentTime, i);
			}
		}
		else//first dead and first alive would be equal in this case - meaning they are all alive
		{
			for (int i = 0; i < emitterData->MaxParticles; i++)
			{
				UpdateSingleParticle(currentTime, i);
			}
		}
	}

	timeSinceLastEmit += dt;
	while (timeSinceLastEmit > secBetweenParticleEmit)
	{
		EmitParticle(currentTime);
		timeSinceLastEmit -= secBetweenParticleEmit;
	}

	//mapping the buffer locks it on the GPU so we can write to it
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(particleDataBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	//copy living particles fas like sanic - but because of circular buffer they may not be contiguous, it may need two memcpy calls
	if (firstLivingIndex < firstDeadIndex)
	{
		//copy from firstLiving to firstDead
		memcpy(
			mapped.pData, //Destination = start of particle buffer
			particles + firstLivingIndex, //Source = particle array w/ offset to first living particle
			sizeof(ParticleData) * livingCount); //Amount = num of particles in bytes
	}
	else
	{
		//copy from beginning of array until the first dead
		memcpy(
			mapped.pData, //Destination = start of particle buffer
			particles, //Source = start of the particle array
			sizeof(ParticleData) * firstDeadIndex); //Amount = num of particles up to first dead

		//copy from the first living to the end of the array
		memcpy(
			(void*)((ParticleData*)mapped.pData + firstDeadIndex), //Destination = particle buffer, but the part after where the previous part we copied is
			particles + firstLivingIndex, //Source = particle array w/ offset to first living particle
			sizeof(ParticleData) * (emitterData->MaxParticles - firstLivingIndex)); //Amount = num of particles in bytes
	}
	//unmap (unlock) now that we're done with it
	context->Unmap(particleDataBuffer.Get(), 0);
}

void HybridEmitter::Draw(Camera* camera, float currentTime)
{
	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullBuffer = 0;
	context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//set shaders
	vs->SetShader();
	ps->SetShader();


	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vs->SetFloat("currentTime", currentTime);
	vs->CopyAllBufferData();

	vs->SetShaderResourceView("ParticleData", particleDataSRV.Get());

	//ps->SetFloat4("Color", color);
	ps->CopyAllBufferData();
	ps->SetShaderResourceView("Texture", texture.Get());
	//ps->SetSamplerState("BasicSampler", sampler);

	context->DrawIndexed(livingCount * 6, 0, 0);

}

std::shared_ptr<Transform> HybridEmitter::GetTransform()
{
	return transform;
}

void HybridEmitter::SetRectBounds(float x, float y, float z)
{
	emitterData->RectDimensions = DirectX::XMFLOAT3(x, y, z);
	emitterShape = EmitterShape::Sphere;
	//emitterShape = EmitterShape::RectPrism;
}

void HybridEmitter::SetEmitterShape(EmitterShape emitShape)
{
	switch (emitShape)
	{
	case EmitterShape::Point:

	case EmitterShape::RectPrism:
	case EmitterShape::Sphere:

	default:
		break;
	}
}

EmitterData* HybridEmitter::GetEmitterData()
{
	return emitterData;
}

void HybridEmitter::EmitParticle(float emitTime)
{
	if (livingCount >= emitterData->MaxParticles) //dont spawn more if its reached the max
		return;

	//update the particle with new information
	particles[firstDeadIndex].EmitTime = emitTime;
	particles[firstDeadIndex].Lifespan = emitterData->ParticleLifetime;
	//float debug = emitTime - 
	particles[firstDeadIndex].StartVelocity = emitterData->StartVelocity;
	particles[firstDeadIndex].Acceleration = CalcAcceleration();

	particles[firstDeadIndex].StartColor = emitterData->StartColor;
	particles[firstDeadIndex].EndColor = emitterData->EndColor;

	//particles[firstDeadIndex].StartVelocity = ;
	if (emitterData->EmitShape == EmitterShape::Point)
	{
		particles[firstDeadIndex].StartPosition = transform->GetPosition();
		//particles[firstDeadIndex].StartVelocity = emitterData->StartVelocity;

	}
	else if(emitterData->EmitShape == EmitterShape::RectPrism)
	{
		particles[firstDeadIndex].StartPosition = RandomRectPosition();
		//particles[firstDeadIndex].StartVelocity = emitterData->StartVelocity;
	}
	else if (emitterData->EmitShape == EmitterShape::Sphere)
	{
		DirectX::XMFLOAT3 direction = RandomSphericalDirection();
		DirectX::XMVECTOR speedScale = DirectX::XMVectorScale(DirectX::XMVectorSet(direction.x, direction.y, direction.z, 1.0f), emitterData->StartSpeed);
		DirectX::XMStoreFloat3(&particles[firstDeadIndex].StartVelocity, speedScale);
		
		particles[firstDeadIndex].StartPosition = RandomSphereLocation(direction);

	}



	firstDeadIndex++;
	firstDeadIndex %= emitterData->MaxParticles;
	//if (firstDeadIndex > emitterData->MaxParticles) 
	//{
	//	firstDeadIndex = 0; //wrapping back to the front of the array
	//}

	livingCount++;
}

DirectX::XMFLOAT3 HybridEmitter::RandomRectPosition()
{
	float x = 
		(rand() / (float)RAND_MAX * emitterData->RectDimensions.x)
		- (emitterData->RectDimensions.x / 2);
	float y = 
		(rand() / (float)RAND_MAX * emitterData->RectDimensions.y)
		- (emitterData->RectDimensions.y / 2);
	float z = 
		(rand() / (float)RAND_MAX * emitterData->RectDimensions.z)
		- (emitterData->RectDimensions.z / 2);
	return DirectX::XMFLOAT3(x, y, z);
}

void HybridEmitter::UpdateSingleParticle(float currentTime, int index)
{
	float age = currentTime - particles[index].EmitTime;

	// Update and check for death
	if (age >= emitterData->ParticleLifetime)
	{
		//the first living index is also the "oldest" particle, so this is how we "kill" it
		firstLivingIndex++;
		firstLivingIndex %= emitterData->MaxParticles;
		livingCount--;
	}
}

DirectX::XMFLOAT3 HybridEmitter::RandomSphereLocation(DirectX::XMFLOAT3 direction)
{
	// math source: https://math.stackexchange.com/questions/1585975/how-to-generate-random-points-on-a-sphere
	// c++ normal distribution info: https://www.cplusplus.com/reference/random/normal_distribution/

	//std::normal_distribution<float> distribution(5.0, 2.0); //i actually have no idea what to put for this but its getting normalized so it shouldnt matter maybe?

	//DirectX::XMFLOAT3 direction = RandomSphericalDirection();

	float x = direction.x * emitterData->SphereRadius;
	float y = direction.y * emitterData->SphereRadius;
	float z = direction.z * emitterData->SphereRadius;

	return DirectX::XMFLOAT3(x, y, z);
}

DirectX::XMFLOAT3 HybridEmitter::RandomSphericalDirection()
{
	std::normal_distribution<float> distribution(0, 4.0f); //lets do 4 standard deviations i guess why not
	float xGaus = distribution(generator);
	float yGaus = distribution(generator);
	float zGaus = distribution(generator);

	DirectX::XMVECTOR normalized = DirectX::XMVector3Normalize(DirectX::XMVectorSet(xGaus, yGaus, zGaus, 1.0f));
	DirectX::XMFLOAT3 output;
	DirectX::XMStoreFloat3(&output, normalized);
	return output;
}

float HybridEmitter::CalcAcceleration()
{
	//i just need to calculate acceleration and send *that* to the gpu. idk if there is more or less that is needed
	return 1.0f;
}
