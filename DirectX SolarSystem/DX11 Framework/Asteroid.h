#pragma once
#ifndef ASTEROID
#define ASTEROID

#include "d3d11_1.h"
#include <DirectXMath.h>

using namespace DirectX;

class Asteroid
{
private:
	//Matrix to store position
	XMFLOAT4X4 m_Matrix;

	//Floats to store random numbers within certain values for the positions and sizes of the asteroids
	float m_xOffset;
	float m_yOffset;
	float m_zOffset;

	float m_xScaling;
	float m_yScaling;
	float m_zScaling;

	//Float variables to store how fast the asteroid spins, and rotates within the asteroid belt
	float m_RotationPeriod;
	float m_OrbitPeriod;

    //Material variables
	const XMFLOAT4 m_AmbientMtrl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	const XMFLOAT4 m_DiffuseMtrl = XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f);
	const XMFLOAT4 m_SpecularMtrl = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);

    //Float to store the current simulation speed
	float simSpeed;
public:
	//Constructor
	Asteroid(float x, float y, float z, float xScale, float yScale, float zScale, float rotation, float orbit);

	//Destructor
	~Asteroid();

	//Update function to update the asteroid matrix
	void Update(float time, float speed);

	//Overload update function for asteroids around saturn
	void Update(float time, float speed, XMFLOAT4X4 referenceMatrix);

	//Function to get the matrix of the asteroid so they can be drawn
	XMFLOAT4X4 GetMatrix() { return m_Matrix; }
};

#endif