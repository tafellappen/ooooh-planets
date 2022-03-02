#include "HybridEmitter.h"

HybridEmitter::HybridEmitter(
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
)
{
	this->particlesEmitPerSec = particlesEmitPerSec;
	this->particleLifetime = particleLifetime;
	this->maxParticles = maxParticles;
	this->color = color;
	this->vs = vs;
	this->ps = ps;
	this->context = context;
	this->texture = texture;
	//this->sampler = sampler;

	this->secBetweenParticleEmit = 1.0f / particlesEmitPerSec;

	timeSinceLastEmit = 0.0f;
	livingCount = 0;
	firstLivingIndex = 0;
	firstDeadIndex = 0;
	transform = std::make_shared<Transform>();
	//emitFromPoint = true;

	particles = std::make_shared<ParticleData[]>(maxParticles);

	// Create an index buffer for particle drawing
	// indices as if we had two triangles per particle
	unsigned int* indices = new unsigned int[maxParticles * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
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
	ibDesc.ByteWidth = sizeof(unsigned int) * maxParticles * 6;
	device->CreateBuffer(&ibDesc, &indexData, indexBuffer.GetAddressOf());
	delete[] indices;

	//structured buffer for passing the particle array data to the gpu
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(ParticleData);
	desc.ByteWidth = sizeof(ParticleData) * maxParticles;
	device->CreateBuffer(&desc, 0, particleDataBuffer.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxParticles;
	device->CreateShaderResourceView(particleDataBuffer.Get(), &srvDesc, particleDataSRV.GetAddressOf());


}

HybridEmitter::~HybridEmitter()
{
	//delete[] particles;
	//delete vs; //i guess my asset manager already cleans these up, so i dont need these here?
	//delete ps;
}

void HybridEmitter::Update(float dt, float currentTime)
{
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
			for (int i = firstLivingIndex; i < maxParticles; i++)
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
			for (int i = 0; i < maxParticles; i++)
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
			particles.get() + firstLivingIndex, //Source = particle array w/ offset to first living particle
			sizeof(ParticleData) * livingCount); //Amount = num of particles in bytes
	}
	else
	{
		//copy from beginning of array until the first dead
		memcpy(
			mapped.pData, //Destination = start of particle buffer
			particles.get(), //Source = start of the particle array
			sizeof(ParticleData) * firstDeadIndex); //Amount = num of particles up to first dead

		//copy from the first living to the end of the array
		memcpy(
			(void*)((ParticleData*)mapped.pData + firstDeadIndex), //Destination = particle buffer, but the part after where the previous part we copied is
			particles.get() + firstLivingIndex, //Source = particle array w/ offset to first living particle
			sizeof(ParticleData) * (maxParticles - firstLivingIndex)); //Amount = num of particles in bytes
	}
	//unmap (unlock) now that we're done with it
	context->Unmap(particleDataBuffer.Get(), 0);

	//std::cout << "living: " << livingCount << std::endl;
	//std::cout << "firstDead:   " << firstDeadIndex << std::endl;
	//std::cout << "firstLiving: " << firstLivingIndex << std::endl;
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

	ps->SetFloat4("Color", color);
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
	emissionRectDimensions = DirectX::XMFLOAT3(x, y, z);
	emitterShape = EmitterShape::RectPrism;
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

void HybridEmitter::EmitParticle(float emitTime)
{
	if (livingCount >= maxParticles) //dont spawn more if its reached the max
		return;

	//update the particle with new information
	particles.get()[firstDeadIndex].EmitTime = emitTime;
	particles.get()[firstDeadIndex].StartVelocity = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	if (emitterShape == EmitterShape::RectPrism)
	{
		particles.get()[firstDeadIndex].StartPosition = transform->GetPosition();

	}
	else
	{
		//i will be lazy and not center it around the HybridEmitter transform
		float x = rand() / (float)RAND_MAX * emissionRectDimensions.x;
		float y = rand() / (float)RAND_MAX * emissionRectDimensions.y;
		float z = rand() / (float)RAND_MAX * emissionRectDimensions.z;


		particles.get()[firstDeadIndex].StartPosition = DirectX::XMFLOAT3(x, y, z);
	}



	firstDeadIndex++;
	firstDeadIndex %= maxParticles;
	//if (firstDeadIndex > maxParticles) 
	//{
	//	firstDeadIndex = 0; //wrapping back to the front of the array
	//}

	livingCount++;
}

void HybridEmitter::UpdateSingleParticle(float currentTime, int index)
{
	float age = currentTime - particles.get()[index].EmitTime;

	// Update and check for death
	if (age >= particleLifetime)
	{
		//the first living index is also the "oldest" particle, so this is how we "kill" it
		firstLivingIndex++;
		firstLivingIndex %= maxParticles;
		livingCount--;
	}
}
