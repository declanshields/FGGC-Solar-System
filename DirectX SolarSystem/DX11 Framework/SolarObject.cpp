#include "SolarObject.h"

SolarObject::SolarObject(MeshData mesh, ID3D11ShaderResourceView* texture, XMFLOAT4 difMtrl, XMFLOAT4 ambMtrl, XMFLOAT4 specMtrl)
{
	m_Mesh = mesh;
	m_Texture = texture;
	m_DiffuseMtrl = difMtrl;
	m_AmbientMtrl = ambMtrl;
	m_SpecularMtrl = specMtrl;
}


SolarObject::~SolarObject()
{
	m_Texture = nullptr;
	delete m_Texture;
}
