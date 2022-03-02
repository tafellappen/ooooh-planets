#pragma once

#include <DirectXMath.h>
#include <vector>
class Transform
{
public:
	Transform();
	~Transform();
	
	//hierarchy methods
		//adds the specified child to this transform's list of children, as long as the child isn't already in the list
		//set the current object as the child's parent.
		//notify the child that their transforms are now dirty
	void AddChild(Transform* child);

		//if child exists, remove it and also set the child's parent to nukk
	void RemoveChild(Transform* child);
		
		//set this object's parent to be the specified transform
		//if newParent is not null, also add the current object(this) to the new parent's list of children
	void SetParent(Transform* newParent);

		//returns the object's parent
	Transform* GetParent();

		//returns the child at the specified index in the children list
	Transform* GetChild(unsigned int index);

		//search through the list of children and return the index of the spcified child
	int IndexOfChild(Transform* child);

		//returns the number of children 
	unsigned int GetChildCount();

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
	//transform hierarchy
	Transform* parent;
	std::vector<Transform*> children;
		//recursely sets each child as having dirty transformation matrices
	void MarkChildTransformsDirty(); 

	//for raw values
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 pitchYawRoll; //not a quat, but we could use that instead as a FLOAT4
	DirectX::XMFLOAT3 scale;

	DirectX::XMFLOAT4X4 worldMatrix; //most recent value
	bool isDirty; //does the matrix need to be remade?
};

