#include "Camera.h"
#include "Input.h"

using namespace DirectX;

Camera::Camera(float x, float y, float z, float aspectRatio)
{
	transform.SetPosition(x, y, z);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
}

void Camera::Update(float dt, HWND windowHandle)
{
	//later: speed changing maybe
	Input& input = Input::GetInstance();
	float speed = dt * 3.0f;
	//movement (check keyboard)
	if (input.KeyDown('W')) { transform.MoveRelative(0, 0, speed); }
	if (input.KeyDown('S')) { transform.MoveRelative(0, 0, -speed); }
	if (input.KeyDown('A')) { transform.MoveRelative(-speed, 0, 0); }
	if (input.KeyDown('D')) { transform.MoveRelative(speed, 0, 0); }

	//mouse looking
	POINT mousePos = {};
	GetCursorPos(&mousePos);
	ScreenToClient(windowHandle, &mousePos); //make relative to window
	
	//left mouse?
	if (input.MouseLeftDown()) 
	{
		float xDiff = dt * (mousePos.x - previousMousePosition.x);
		float yDiff = dt * (mousePos.y - previousMousePosition.y);

		transform.Rotate(yDiff, xDiff, 0); //in that order
	}

	UpdateViewMatrix();

	//store mouse pos

	previousMousePosition = mousePos;
}

void Camera::UpdateViewMatrix()
{
	//
	XMFLOAT3 pitchYawRoll = transform.GetRotation();
	XMVECTOR forwardDirection = XMVector3Rotate(
		XMVectorSet(0, 0, 1, 0),
		XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));

	XMFLOAT3 position = transform.GetPosition();
	XMMATRIX view = XMMatrixLookToLH(
		XMLoadFloat3(&position),
		forwardDirection,
		XMVectorSet(0, 1, 0, 0)); //world's up vector

	XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMMATRIX proj = XMMatrixPerspectiveFovLH(
		XM_PIDIV4, //FOV in RADs
		aspectRatio,
		0.01f, //near clip plane (small but never zero)
		100.0f); //far clip plane (bigger but not massive

	XMStoreFloat4x4(&projectionMatrix, proj);
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}

std::shared_ptr<Transform> Camera::GetTransform()
{
	return std::make_shared<Transform>(transform);
}
