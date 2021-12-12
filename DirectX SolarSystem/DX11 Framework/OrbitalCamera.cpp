#include "OrbitalCamera.h"

OrbitalCamera::OrbitalCamera(XMFLOAT4X4 position, XMFLOAT4X4 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	_eye = position;
	_at = at;
	_up = up;

	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;

	eyeFloat = XMFLOAT3(_eye._41, _eye._42, _eye._43);
	atFloat = XMFLOAT3(_at._41, _at._42, at._43);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(XMLoadFloat3(&eyeFloat), XMLoadFloat3(&atFloat), XMLoadFloat3(&_up)));
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, 0.01f, 100.0f));

	XMMATRIX _viewMatrix = XMLoadFloat4x4(&_view);
	XMMATRIX _projectionMatrix = XMLoadFloat4x4(&_projection);
	XMMATRIX _viewProj = _projectionMatrix * _viewMatrix;

	XMStoreFloat4x4(&_viewProjection, _viewProj);
}

void OrbitalCamera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;

	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, 0.01f, 100.0f));
}

void OrbitalCamera::Update(XMFLOAT4X4 pos, XMFLOAT4X4 at)
{
	_eye = pos;
	_at = at;

	eyeFloat = XMFLOAT3(_eye._41, _eye._42, _eye._43);
	atFloat = XMFLOAT3(_at._41, _at._42, _at._43);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(XMLoadFloat3(&eyeFloat), XMLoadFloat3(&atFloat), XMLoadFloat3(&_up)));
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, 0.01f, 100.0f));
}

void OrbitalCamera::Update()
{
	_eye._41 = eyeFloat.x;
	_eye._42 = eyeFloat.y;
	_eye._43 = eyeFloat.z;

	_at._41 = atFloat.x;
	_at._42 = atFloat.y;
	_at._43 = atFloat.z;

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(XMLoadFloat3(&eyeFloat), XMLoadFloat3(&atFloat), XMLoadFloat3(&_up)));
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, 0.01f, 100.0f));
}


XMFLOAT4X4 OrbitalCamera::GetViewProjection()
{
	XMMATRIX _viewMatrix = XMLoadFloat4x4(&_view);
	XMMATRIX _projectionMatrix = XMLoadFloat4x4(&_projection);
	XMMATRIX _viewProj = _projectionMatrix * _viewMatrix;

	XMStoreFloat4x4(&_viewProjection, _viewProj);

	return _viewProjection;
}