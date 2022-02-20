#include "Transform.h"

using namespace DirectX; 

Transform::Transform()
{
	position = XMFLOAT3(0, 0, 0);
	pitchYawRoll = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);
	parent = 0;
	//scale = XMFLOAT3(0, 0, 0); //DONT DO THIS IT WILL NOT SHOW UP DUMBASS
	//init world matrix
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	isDirty = false;
}

Transform::~Transform()
{
}

void Transform::AddChild(Transform* child)
{

	if (!child)
	{
		return;
	}

	//check if child already exists
	if (IndexOfChild(child) >= 0)
	{
		return;
	}

	//Get matrices
	XMFLOAT4X4 parentWorld = GetWorldMatrix();
	XMMATRIX parentWorldMatrix = XMLoadFloat4x4(&parentWorld);

	XMFLOAT4X4 childWorld = child->GetWorldMatrix();
	XMMATRIX childWorldMatrix = XMLoadFloat4x4(&parentWorld);

	//Get parent matrix's inverse
	XMMATRIX parentWorldInv = XMMatrixInverse(0, parentWorldMatrix);

	//Multiply the child by the inverse parent
	XMMATRIX newChildWorld = childWorldMatrix * parentWorldInv;

	//Set the child's transform from this new matrix
	XMFLOAT4X4 relativeChildWorld;
	XMStoreFloat4x4(&relativeChildWorld, newChildWorld);
	
	//Update child
	children.push_back(child);
	child->parent = this;

	//Child is now out of date
	child->isDirty = true;
	child->MarkChildTransformsDirty();

}

void Transform::RemoveChild(Transform* child)
{
	auto removeChild = std::find(children.begin(), children.end(), child);
	if (removeChild == children.end())
	{
		return;
	}

	children.erase(removeChild);
	child->parent = 0;
	child->MarkChildTransformsDirty();
}

void Transform::SetParent(Transform* newParent)
{
	//remove when it's an existing parent
	if(this->parent)
	{
		this->parent->RemoveChild(this);
	}

	//add when it's a new parent
	if (newParent)
	{
		newParent->AddChild(this);
	}
}

Transform* Transform::GetParent()
{
	return parent;
}

Transform* Transform::GetChild(unsigned int index)
{
	return children[index];
}

int Transform::IndexOfChild(Transform* child)
{
	if(!child)
	{
		return -1;
	}

	for (unsigned int i = 0; i < children.size(); i++)
	{
		if (children[i] == child)
		{
			return (int)i;
		}

	}
	return -1;

}

unsigned int Transform::GetChildCount()
{
	return (unsigned int)children.size();
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

		//check if parent exists
		if (parent)
		{
			XMFLOAT4X4 parentWorld = parent->GetWorldMatrix();
			worldMat *= XMLoadFloat4x4(&parentWorld);
		}

		//printf(translationMat[0]);
		//store in an XMFLOAT4x4 so we can return
		XMStoreFloat4x4(&worldMatrix, worldMat);
		isDirty = false;

	}

	return worldMatrix;
}

void Transform::MarkChildTransformsDirty()
{
	for (int i = 0; i < children.size();i++)
	{
		children[i]->isDirty = true;
		children[i] ->MarkChildTransformsDirty();
	}
}
