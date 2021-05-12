#include "Emitter.h"


using namespace DirectX;

Emitter::Emitter(int maxParticles, int particlesPerSecond, float lifetime, float startSize, float endSize, DirectX::XMFLOAT4 startColor, DirectX::XMFLOAT4 endColor, DirectX::XMFLOAT3 startVelocity, DirectX::XMFLOAT3 velocityRandomRange, DirectX::XMFLOAT3 emitterPosition, DirectX::XMFLOAT3 positionRandomRange, DirectX::XMFLOAT4 rotationRandomRanges, DirectX::XMFLOAT3 emitterAcceleration, Microsoft::WRL::ComPtr<ID3D11Device> device, std::shared_ptr<SimpleVertexShader> vertShader, std::shared_ptr<SimplePixelShader> pixShader, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture, bool isSpriteSheet, unsigned int spriteSheetWidth, unsigned int spriteSheetHeight)
{
	vertexShader = vertShader;
	pixelShader = pixShader;
	this->texture = texture;

    this->particlesPerSecond = particlesPerSecond;
    this->secondsPerParticle = 1.0f / particlesPerSecond;

    //spritesheet stuff
    this->isSpriteSheet = isSpriteSheet;
    this->spriteSheetWidth = max(spriteSheetWidth, 1);
    this->spriteSheetHeight = max(spriteSheetHeight, 1);
    this->spriteSheetFrameWidth = 1.0f / this->spriteSheetWidth;
    this->spriteSheetFrameHeight = 1.0f / this->spriteSheetHeight;

    //particle stuff
    this->maxParticles = maxParticles;
    this->lifetime = lifetime;
    this->startColor = startColor;
    this->endColor = endColor;
    this->startVelocity = startVelocity;
    this->startSize = startSize;
    this->endSize = endSize;
    this->particlesPerSecond = particlesPerSecond;
    this->secondsPerParticle = 1.0f / particlesPerSecond;

    //non-params
    timeSinceEmit = 0;
    livingParticleCount = 0;
    firstAliveIndex = 0;
    firstDeadIndex = 0;

	// Make the particle array
	particles = new Particle[maxParticles];
	ZeroMemory(particles, sizeof(Particle) * maxParticles);

	// Create local particle vertices

	DefaultUVs[0] = XMFLOAT2(0, 0);
	DefaultUVs[1] = XMFLOAT2(1, 0);
	DefaultUVs[2] = XMFLOAT2(1, 1);
	DefaultUVs[3] = XMFLOAT2(0, 1);

	// Create UV's here, as those will usually stay the same
	localParticleVertices = new ParticleVertex[4 * maxParticles];
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		localParticleVertices[i + 0].UV = DefaultUVs[0];
		localParticleVertices[i + 1].UV = DefaultUVs[1];
		localParticleVertices[i + 2].UV = DefaultUVs[2];
		localParticleVertices[i + 3].UV = DefaultUVs[3];
	}


	// Create buffers for drawing particles

	// DYNAMIC vertex buffer (no initial data necessary)
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(ParticleVertex) * 4 * maxParticles;
	device->CreateBuffer(&vbDesc, 0, vertexBuffer.GetAddressOf());

	// Index buffer data
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
}

Emitter::~Emitter()
{
	delete particles;
	particles = nullptr;
	delete localParticleVertices;
	localParticleVertices = nullptr;
}

void Emitter::Update(float dt)
{
	//cyclic buffer - this check tells us if the living particles are contiguous or not
	if (firstAliveIndex < firstDeadIndex) //if first alive is before first dead, that means it's contiguous
	{
		//go through each alive particle and update it
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
		{
			UpdateSingleParticle(dt, i);
		}
	}
	else //if first alive is after first dead (ie, closer to the end of the array than first dead), that means it's not contiguous so we have to wrap around to the beginning of the array
	{
		//to do this, start from the first alive particle and go to the end of the array
		//then go back to the beginning and go up to the index of the first dead particle

		for (int i = firstAliveIndex; i < maxParticles; i++) //first part
		{
			UpdateSingleParticle(dt, i);
		}

		for (int i = 0; i < firstDeadIndex; i++)
		{
			UpdateSingleParticle(dt, i);
		}

	}
}

void Emitter::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	//copy to the buffer
	CopyParticlesToGPU(context, camera);

	//buffers
	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vertexShader->SetShader();
	vertexShader->CopyAllBufferData();

	pixelShader->SetShaderResourceView("particle", texture.Get());
	pixelShader->SetShader();

	//draw the alive parts of the buffer - wrap around (just like in Update)
	if (firstAliveIndex < firstDeadIndex) //coniguous
	{
		//calculate the index count based on the count of living particles
		//calculate the starting point based on the index of the first alive
		context->DrawIndexed(livingParticleCount * 6, firstAliveIndex * 6, 0); //the 6 is for the number of verts per particle. They are being drawn with two triangles forming a square
	}
	else
	{
		//draw first part, from 0 to the first dead
		context->DrawIndexed(firstDeadIndex * 6, 0, 0);

		//draw second part, from first alive to the end of the array
		context->DrawIndexed((maxParticles - firstAliveIndex) * 6, firstAliveIndex * 6, 0);

	}
}

void Emitter::UpdateSingleParticle(float dt, int index)
{
	//make sure particle is not dead before updating
	if (IsParticleDead(index))
		return;

	//update the specific particle and check if it's now dead
	particles[index].Age += dt;
	if (IsParticleDead(index)) 
	{
		//shift the alive count
		firstAliveIndex++;
		firstAliveIndex %= maxParticles; //im confused, what is this line supposed to do?
		livingParticleCount--;
		return;
	}

	// color, size, position, etc are determined based on the starting and ending values and where the particle is in it's "life cycle"
	//aka linear interpolation
	//need to know percentage for lerp
	float agePercent = particles[index].Age / lifetime;

	//color interpolation
	XMStoreFloat4(&particles[index].Color, XMVectorLerp(XMLoadFloat4(&startColor), XMLoadFloat4(&endColor), agePercent));

	//rotation lerp
	float rotationStart = particles[index].RotationStart;
	float rotationEnd = particles[index].RotationEnd;
	particles[index].Rotation = rotationStart + agePercent * (rotationEnd - rotationStart);

	//size lerp
	particles[index].Size = startSize + agePercent * (endSize - startSize);

	//position will be based on constant acceleration function instead of lerp
	XMVECTOR startPos = XMLoadFloat3(&particles[index].StartPosition);
	XMVECTOR startVel = XMLoadFloat3(&particles[index].StartVelocity);
	XMVECTOR acceleration = XMLoadFloat3(&emitterAcceleration); //if im right about how acceleration works then this might not look good for the comet. Check here first when it doesnt look good
	float t = particles[index].Age;
	//constant acceleration used to calculate the new position
	XMVECTOR calculatedPosition = acceleration * t * t / 2.0f + startVel * t + startPos;
	XMStoreFloat3(&particles[index].Position, calculatedPosition); //store it
}

void Emitter::SpawnParticle()
{
	//check if particles should be spawned
	if (livingParticleCount == maxParticles)
		return;

	//The first dead particle is the "oldest" dead particle, AKA it's first in line to be reset alive again
	particles[firstDeadIndex].Age = 0;
	particles[firstDeadIndex].Size = startSize;
	particles[firstDeadIndex].Color = startColor;
	
	particles[firstDeadIndex].StartPosition = emitterPosition;
	//yeah we want to start at the emitter but also not EXACTLY at the emitter, randomize the position within a certain range
	particles[firstDeadIndex].StartPosition.x = (((float)rand() / RAND_MAX) * 2 - 1) * positionRandomRange.x;
	particles[firstDeadIndex].StartPosition.y = (((float)rand() / RAND_MAX) * 2 - 1) * positionRandomRange.y;
	particles[firstDeadIndex].StartPosition.z = (((float)rand() / RAND_MAX) * 2 - 1) * positionRandomRange.z;

	particles[firstDeadIndex].StartVelocity = startVelocity;
	//same with the velocity
	particles[firstDeadIndex].StartVelocity.x = (((float)rand() / RAND_MAX) * 2 - 1) * velocityRandomRange.x;
	particles[firstDeadIndex].StartVelocity.y = (((float)rand() / RAND_MAX) * 2 - 1) * velocityRandomRange.y;
	particles[firstDeadIndex].StartVelocity.z = (((float)rand() / RAND_MAX) * 2 - 1) * velocityRandomRange.z;

	//same with start and end rotations
	float rotStartMin = rotationRandomRanges.x;
	float rotStartMax = rotationRandomRanges.y;
	particles[firstDeadIndex].RotationStart = ((float)rand() / RAND_MAX) * (rotStartMax - rotStartMin) + rotStartMin;

	float rotEndMin = rotationRandomRanges.z;
	float rotEndMax = rotationRandomRanges.w;
	particles[firstDeadIndex].RotationStart = ((float)rand() / RAND_MAX) * (rotEndMax - rotEndMin) + rotEndMin;

	//incrementing and wrapping
	firstDeadIndex++;
	firstDeadIndex %= maxParticles; //i still dont see what this line is supposed to do

	livingParticleCount++;
}

//maybe a bit excessive to make a method for a single line of code but i feel like its a tad more readable this way
bool Emitter::IsParticleDead(int index)
{
	return (particles[index].Age >= lifetime);
}

void Emitter::CopyParticlesToGPU(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	//update the local buffer
	//dont waste time on the dead particles

	//cyclic buffer - need to check how to go through it
	if (firstAliveIndex < firstDeadIndex)
	{
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
			CopyOneParticle(i, camera);
	}
	else
	{
		for (int i = firstAliveIndex; i < maxParticles; i++)
			CopyOneParticle(i, camera);

		for (int i = firstDeadIndex; i < 0; i++)
			CopyOneParticle(i, camera);
	}

	//now send the updated buffer to the GPU
	/*D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, localParticleVertices, sizeof(ParticleVertex) * 4 * maxParticles);

	context->Unmap(vertexBuffer.Get(), 0);*/
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, localParticleVertices, sizeof(ParticleVertex) * 4 * maxParticles);

	context->Unmap(vertexBuffer.Get(), 0);
}

void Emitter::CopyOneParticle(int index, std::shared_ptr<Camera> camera)
{
	int i = index * 4;

	localParticleVertices[i + 0].Position = CalcParticleVertexPosition(index, 0, camera);
	localParticleVertices[i + 1].Position = CalcParticleVertexPosition(index, 1, camera);
	localParticleVertices[i + 2].Position = CalcParticleVertexPosition(index, 2, camera);
	localParticleVertices[i + 3].Position = CalcParticleVertexPosition(index, 3, camera);

	localParticleVertices[i + 0].Color = particles[index].Color;
	localParticleVertices[i + 1].Color = particles[index].Color;
	localParticleVertices[i + 2].Color = particles[index].Color;
	localParticleVertices[i + 3].Color = particles[index].Color;

	//if it's a spritesheet texture, need to update the UV coords as the particle ages
	if (isSpriteSheet)
	{
		// How old is this particle as a percentage
		float agePercent = particles[index].Age / lifetime;

		// get the sprite sheet index based on the 
		int ssIndex = (int)floor(agePercent * (spriteSheetWidth * spriteSheetHeight));

		// Get the U/V indices (basically column & row index across the sprite sheet)
		int uIndex = ssIndex % spriteSheetWidth;
		int vIndex = ssIndex / spriteSheetHeight; // Integer division is important here!

		// Convert to a top-left corner in uv space (0-1)
		float u = uIndex / (float)spriteSheetWidth;
		float v = vIndex / (float)spriteSheetHeight;

		localParticleVertices[i + 0].UV = XMFLOAT2(u, v);
		localParticleVertices[i + 1].UV = XMFLOAT2(u + spriteSheetFrameWidth, v);
		localParticleVertices[i + 2].UV = XMFLOAT2(u + spriteSheetFrameWidth, v + spriteSheetFrameHeight);
		localParticleVertices[i + 3].UV = XMFLOAT2(u, v + spriteSheetFrameHeight);
	}
}

DirectX::XMFLOAT3 Emitter::CalcParticleVertexPosition(int particleIndex, int quadCornerIndex, std::shared_ptr<Camera> camera)
{
	//get right and up vectors from view matrix
	XMFLOAT4X4 view = camera->GetViewMatrix();

	XMVECTOR camRight = XMVectorSet(view._11, view._21, view._31, 0);
	XMVECTOR camUp = XMVectorSet(view._12, view._22, view._32, 0);

	//determine offset of this particular quad corner
	//UV's are already set when emitter is created, so we can start with that and then modify for each corner
	XMFLOAT2 offset = DefaultUVs[quadCornerIndex];
	offset.x = offset.x * 2 - 1; //expand from [0, 1] to [-1, 1]
	offset.y = (offset.y * -2 + 1); //flip for y

	//load into a vector as a float 3 with a Z of 0
	//create and apply a X rotation matrix to the offset
	XMVECTOR offsetVec = XMLoadFloat2(&offset);
	XMMATRIX rotMatrix = XMMatrixRotationZ(particles[particleIndex].Rotation);
	offsetVec = XMVector3Transform(offsetVec, rotMatrix);

	//add and scale the camera up and right vectors to the position of the particle
	XMVECTOR posVec = XMLoadFloat3(&particles[particleIndex].Position);
	posVec += camRight * XMVectorGetX(offsetVec) * particles[particleIndex].Size;
	posVec += camUp * XMVectorGetY(offsetVec) * particles[particleIndex].Size;

	//set the position to the return value
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, posVec);
	return pos;

}
