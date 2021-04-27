#pragma once

#include <DirectXMath.h>
class Transform
{
public:
	Transform();
	~Transform();

	//methods to adjust existing transforms
	void MoveAbsolute(float x, float y, float z); //move along world axes (ignore rotation)
	void MoveRelative(float x, float y, float z); //move along local axes (respect rotation)
	void Rotate(float pitch, float yaw, float roll);
	void Scale(float x, float y, float z);

	//methods to overwrite existing transforms
	void SetPosition(float x, float y, float z);
	void SetRotation(float pitch, float yaw, float roll);
	void SetScale(float x, float y, float z);

	//methdos to retrieve transform data
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();

	//method to return/calculate the resulting world matrix
	DirectX::XMFLOAT4X4 GetWorldMatrix();
private:
	//for raw values
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 pitchYawRoll; //not a quat, but we could use that instead as a FLOAT4
	DirectX::XMFLOAT3 scale;

	DirectX::XMFLOAT4X4 worldMatrix; //most recent value
	bool isDirty; //does the matrix need to be remade?
};

