#pragma once
#include "Transform.h"
#include <DirectXMath.h>
#include <Windows.h>
#include <memory>
class Camera
{
public:
	Camera(float x, float y, float z, float aspectRatio);
	~Camera();

	void Update(float dt, HWND windowHandle);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	std::shared_ptr<Transform> GetTransform();

private:
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	//std::shared_ptr<Transform> transform;
	Transform transform;

	POINT previousMousePosition;
};

