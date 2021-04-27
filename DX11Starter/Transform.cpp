#include "Transform.h"

using namespace DirectX; 

Transform::Transform()
{
	position = XMFLOAT3(0, 0, 0);
	pitchYawRoll = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);
	//scale = XMFLOAT3(0, 0, 0); //DONT DO THIS IT WILL NOT SHOW UP DUMBASS

	//init world matrix
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	isDirty = false;
}

Transform::~Transform()
{
}

void Transform::MoveAbsolute(float x, float y, float z)
{

	XMVECTOR newPos = XMVectorAdd(XMLoadFloat3(&position), XMVectorSet(x, y, z, 0));
	XMStoreFloat3(&position, newPos);
	isDirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	//figure out what the transformation is relative to the rotation

	XMVECTOR desiredMovement = XMVectorSet(x, y, z, 0);
	//rotate this movement vector by our rotation
	XMVECTOR relativeMovement = XMVector3Rotate(
		desiredMovement,
		XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll))
	);

	//update
	XMStoreFloat3(&position, XMLoadFloat3(&position) + relativeMovement);
	isDirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMVECTOR newRot = XMVectorAdd(XMLoadFloat3(&pitchYawRoll), XMVectorSet(pitch, yaw, roll, 0));
	XMStoreFloat3(&pitchYawRoll, newRot);
	isDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	XMVECTOR newScale = XMVectorAdd(XMLoadFloat3(&scale), XMVectorSet(x, y, z, 0));
	XMStoreFloat3(&scale, newScale);
	isDirty = true;
}

void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	isDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	pitchYawRoll = XMFLOAT3(pitch, yaw, roll);
	isDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	isDirty = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetRotation()
{
	return pitchYawRoll;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (isDirty)
	{
		XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		XMMATRIX rotationMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
		//this rotation method expects the vector to be in the order pitch, yaw, roll
		//it is called RollPitchYaw because that is the order in which they are applied
		XMMATRIX scaleMat = XMMatrixScalingFromVector(XMLoadFloat3(&scale));

		//final world matrix
		XMMATRIX worldMat = scaleMat * rotationMat * translationMat;
		//printf(translationMat[0]);
		//store in an XMFLOAT4x4 so we can return
		XMStoreFloat4x4(&worldMatrix, worldMat);
		isDirty = false;

	}

	return worldMatrix;
}
