#pragma once

#include <Windows.h>
#include <d3d11_1.h>
#include <DirectXMath.h>

struct SimpleVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;

	bool operator<(const SimpleVertex other) const {
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	}
};

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;
	DirectX::XMFLOAT3 Direction;
	float Pad;
};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Att;
	float Pad;
};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }
	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Direction;
	float Spot;

	DirectX::XMFLOAT3 Att;
	float Pad;
};