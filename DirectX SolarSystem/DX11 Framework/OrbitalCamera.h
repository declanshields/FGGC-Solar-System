#pragma once
#ifndef ORBITALCAMERA
#define ORBITALCAMERA

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <math.h>

using namespace DirectX;

class OrbitalCamera
{
private:
	//Float4x4 for storing position and at to orbit planet
	XMFLOAT3 eyeFloat;
	XMFLOAT3 atFloat;
	XMFLOAT3 _up;

	FLOAT _windowWidth;
	FLOAT _windowHeight;
	FLOAT _nearDepth;
	FLOAT _farDepth;

	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;
	XMFLOAT4X4 _viewProjection;

public:

	XMFLOAT4X4 _at;
	XMFLOAT4X4 _eye;

	//Constructior and desctructor for camera
	OrbitalCamera(XMFLOAT4X4 position, XMFLOAT4X4 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	~OrbitalCamera();

	// Overloaded Update function to make the current view and projection matrices
	void Update(XMFLOAT4X4 pos, XMFLOAT4X4 at);

	//Update Function for the free camera
	void Update();

	// Get and set methods
	void SetPosition(XMFLOAT3 eye) { eyeFloat = eye; }
	void SetLookAt(XMFLOAT3 at) { atFloat = at; }
	void SetUp(XMFLOAT3 up) { _up = up; }

	XMFLOAT4X4 GetPosition() { return _eye; }
	XMFLOAT4X4 GetAt() { return _at; }
	XMFLOAT3 GetUp() { return _up; }

	XMFLOAT3 GetFloatPos() { return eyeFloat; }
	XMFLOAT3 GetFloatAt() { return atFloat; }

	// Get method for matrices
	XMFLOAT4X4 GetViewMatrix() { return _view; }
	XMFLOAT4X4 GetProjectionMatrix() { return _projection; }
	XMFLOAT4X4 GetViewProjection();

	// A function to reshape the camera volume if the window is resized
	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
};

#endif