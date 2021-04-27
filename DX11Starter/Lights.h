#pragma once
#include <DirectXMath.h>

struct DirectionalLight
{
	int Type; //the type of light this is. 0 = Directional, 1 = Point
	DirectX::XMFLOAT3 Color;
	float Intensity;
	DirectX::XMFLOAT3 DirectionOrPos;

};