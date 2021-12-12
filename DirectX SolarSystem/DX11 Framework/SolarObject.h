#pragma once
#ifndef SOLAROBJECT
#define SOLAROBJECT

#include "Structures.h"
#include "d3d11_1.h"
#include "DirectXmath.h"
#include "OBJLoader.h"

using namespace DirectX;

class SolarObject
{
private:
	XMFLOAT4 m_DiffuseMtrl = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 m_AmbientMtrl = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 m_SpecularMtrl = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	MeshData m_Mesh;
	ID3D11ShaderResourceView* m_Texture;

public:
	//constructor
	SolarObject(MeshData mesh, ID3D11ShaderResourceView* texture, XMFLOAT4 difMtrl, XMFLOAT4 ambMtrl, XMFLOAT4 specMtrl);

	//Destructor
	~SolarObject();

	//Get method for mesh
	MeshData GetMesh() { return m_Mesh; }

	//Get method for texture
	ID3D11ShaderResourceView* GetTexture() { return m_Texture; }

	//Get method for diffuse
	XMFLOAT4 GetDiffuseMtrl() { return m_DiffuseMtrl; }

	//Get method for ambient
	XMFLOAT4 GetAmbientMtrl() { return m_AmbientMtrl; }

	//Get method for specular
	XMFLOAT4 GetSpecularMtrl() { return m_SpecularMtrl; }
};

#endif