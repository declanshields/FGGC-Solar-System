#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "Structures.h"
#include "OBJLoader.h"
#include "SolarObject.h"
#include "Asteroid.h"
#include "OrbitalCamera.h"
#include <cstdlib>

struct ConstantBuffer
{
	PointLight gPointLight, gPointLight2;

	SpotLight gSpotLights[6];

	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 SpecularMtrl;
	XMFLOAT4 SpecularLight;
	float SpecularPower;
	XMFLOAT3 EyePosW; //camera pos in world space
	XMFLOAT3 LightVecW;

	float gTime;
};

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device* _pd3dDevice;
	ID3D11DeviceContext* _pImmediateContext;
	IDXGISwapChain* _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader* _pVertexShader;
	ID3D11PixelShader* _pPixelShader;
	ID3D11InputLayout* _pVertexLayout;
	ID3D11Buffer* _pVertexBuffer;
	ID3D11Buffer* _pPyramidVertexBuffer;
	ID3D11Buffer* _pPlaneVertexBuffer;
	ID3D11Buffer* _pIndexBuffer;
	ID3D11Buffer* _pPyramidIndexBuffer;
	ID3D11Buffer* _pPlaneIndexBuffer;
	ID3D11Buffer* _pConstantBuffer;
	XMFLOAT4X4              _world, _sun, _mercury, _venus, _venusAtmos, _earth, _moon, _mars, _phobos, _deimos, _jupiter, _europa, _io, _ganymede, _callisto, _saturn, _enceladus, _titan;
	XMFLOAT4X4              _uranus, _titania, _oberon, _neptune, _backPlane;
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;
	float                   gTime;

	int currentCam = 0;

	float simulationSpeed = 2.0f;

	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _solid;

	XMFLOAT3 lightDirection;
	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;

	XMFLOAT4 ambientMaterial;
	XMFLOAT4 ambientLight;

	XMFLOAT4 specularMaterial;
	XMFLOAT4 specularLight;
	float specularPower;
	XMFLOAT3 eyePos;

	//textures
	ID3D11ShaderResourceView* _pTextureRV = nullptr;

	//Asteroid Texture
	ID3D11ShaderResourceView* _pAsteroidTexture = nullptr;

	//Planet and moon textures
	ID3D11ShaderResourceView* _pSunTexture = nullptr;

	ID3D11ShaderResourceView* _pMercuryTexture = nullptr;

	ID3D11ShaderResourceView* _pVenusSurface = nullptr;
	ID3D11ShaderResourceView* _pVenusAtmos = nullptr;

	ID3D11ShaderResourceView* _pEarthTexture = nullptr;
	ID3D11ShaderResourceView* _pMoonTexture = nullptr;

	ID3D11ShaderResourceView* _pMarsTexture = nullptr;
	ID3D11ShaderResourceView* _pPhobosTexture = nullptr;
	ID3D11ShaderResourceView* _pDeimosTexture = nullptr;

	ID3D11ShaderResourceView* _pJupiterTexture = nullptr;
	ID3D11ShaderResourceView* _pEuropaTexture = nullptr;
	ID3D11ShaderResourceView* _pIoTexture = nullptr;
	ID3D11ShaderResourceView* _pGanymedeTexture = nullptr;
	ID3D11ShaderResourceView* _pCallistoTexture = nullptr;

	ID3D11ShaderResourceView* _pSaturnTexture = nullptr;
	ID3D11ShaderResourceView* _pEnceladusTexture = nullptr;
	ID3D11ShaderResourceView* _pTitanTexture = nullptr;

	ID3D11ShaderResourceView* _pUranusTexture = nullptr;
	ID3D11ShaderResourceView* _pTitaniaTexture = nullptr;
	ID3D11ShaderResourceView* _pOberonTexture = nullptr;

	ID3D11ShaderResourceView* _pNeptuneTexture = nullptr;

	ID3D11ShaderResourceView* _pPlaneTexture = nullptr;

	ID3D11SamplerState* _pSamplerLinear = nullptr;

	//mesh
	MeshData cubeMesh;
	MeshData sphereMesh;
	MeshData rocketMesh;

	//Cameras orbiting the planets
	OrbitalCamera* SunCamera;
	XMFLOAT4X4 _sunCameraPos;
	OrbitalCamera* MercuryCamera;
	XMFLOAT4X4 _MercuryCameraPos;
	OrbitalCamera* VenusCamera;
	XMFLOAT4X4 _VenusCameraPos;
	OrbitalCamera* EarthCamera;
	XMFLOAT4X4 _EarthCameraPos;
	OrbitalCamera* MarsCamera;
	XMFLOAT4X4 _MarsCameraPos;
	OrbitalCamera* JupiterCamera;
	XMFLOAT4X4 _JupiterCameraPos;
	OrbitalCamera* SaturnCamera;
	XMFLOAT4X4 _SaturnCameraPos;
	OrbitalCamera* UranusCamera;
	XMFLOAT4X4 _UranusCameraPos;
	OrbitalCamera* NeptuneCamera;
	XMFLOAT4X4 _NeptuneCameraPos;

	OrbitalCamera* FreeCamera;
	XMFLOAT4X4 _FreeCameraPos;
	XMFLOAT4X4 _FreeCameraDirection;

	ID3D11BlendState* Transparency;

	//Array to store all asteroid objects
	Asteroid* AsteroidArray[10000];

	//Arrays for each of Saturn's rings
	Asteroid* SaturnInnerRingArray[750];
	Asteroid* SaturnMidRingArray[1000];
	Asteroid* SaturnOuterRingArray[1500];
	
	//solar objects
	SolarObject* sun;

	SolarObject* mercury;

	SolarObject* venus;
	SolarObject* venusAtmos;

	SolarObject* earth;
	SolarObject* moon;

	SolarObject* mars;
	SolarObject* phobos;
	SolarObject* deimos;

	SolarObject* jupiter;
	SolarObject* europa;
	SolarObject* io;
	SolarObject* ganymede;
	SolarObject* callisto;

	SolarObject* saturn;
	SolarObject* enceladus;
	SolarObject* titan;

	SolarObject* uranus;
	SolarObject* titania;
	SolarObject* oberon;

	SolarObject* neptune;
private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};