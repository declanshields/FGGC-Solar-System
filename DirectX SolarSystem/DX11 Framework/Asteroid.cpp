#include "Asteroid.h"

Asteroid::Asteroid(float x, float y, float z, float xScale, float yScale, float zScale, float rotation, float orbit)
{
	m_xOffset = x;
	m_yOffset = y;
	m_zOffset = z;

	m_xScaling = xScale;
	m_yScaling = yScale;
	m_zScaling = zScale;

	m_RotationPeriod = rotation;
	m_OrbitPeriod = orbit;
}

void Asteroid::Update(float time, float speed)
{
	XMStoreFloat4x4(&m_Matrix, XMMatrixScaling(m_xScaling, m_yScaling, m_zScaling) * XMMatrixRotationY(m_RotationPeriod * time * speed) * XMMatrixTranslation(m_xOffset, m_yOffset, m_zOffset) * XMMatrixRotationY(m_OrbitPeriod * time * speed));
}

void Asteroid::Update(float time, float speed, XMFLOAT4X4 referenceMatrix)
{

	XMStoreFloat4x4(&m_Matrix, XMMatrixScaling(m_xScaling, m_yScaling, m_zScaling) * XMMatrixRotationY(m_RotationPeriod * time * speed) * XMMatrixTranslation(m_xOffset, m_yOffset, m_zOffset) * XMMatrixRotationY(m_OrbitPeriod * time * speed) * XMLoadFloat4x4(&referenceMatrix));
}