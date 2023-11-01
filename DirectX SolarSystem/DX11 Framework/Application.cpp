#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
    _hInst = nullptr;
    _hWnd = nullptr;
    _driverType = D3D_DRIVER_TYPE_NULL;
    _featureLevel = D3D_FEATURE_LEVEL_11_0;
    _pd3dDevice = nullptr;
    _pImmediateContext = nullptr;
    _pSwapChain = nullptr;
    _pRenderTargetView = nullptr;
    _pVertexShader = nullptr;
    _pPixelShader = nullptr;
    _pVertexLayout = nullptr;
    _pVertexBuffer = nullptr;
    _pIndexBuffer = nullptr;
    _pConstantBuffer = nullptr;
}

Application::~Application()
{
    Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
    {
        return E_FAIL;
    }

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

    //// Initialize the world matrix
    XMStoreFloat4x4(&_world, XMMatrixIdentity());

    XMFLOAT3 floatUp = XMFLOAT3(0.0f, 1.0f, 0.0f);

    XMFLOAT4X4 tempPos =
    {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -2.0f, 0.0f
    };

    XMFLOAT4X4 tempAt =
    {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f
    };

    SunCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);
    MercuryCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);
    VenusCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);
    EarthCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);
    MarsCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);
    JupiterCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);
    SaturnCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);
    UranusCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);
    NeptuneCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);

    tempPos =
    {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -3.0f, 0.0f
    };

    FreeCamera = new OrbitalCamera(tempPos, tempAt, floatUp, _WindowWidth, _WindowHeight, 0.01f, 100.0f);

    //assignment B3
    //create the cubes
    //load cube mesh
    cubeMesh = OBJLoader::Load("cube.obj", _pd3dDevice, false);
    sphereMesh = OBJLoader::Load("sphere.obj", _pd3dDevice, true);

    return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
    HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
    pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    //create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);
    _pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &_pVertexLayout);
    pVSBlob->Release();

    //create textures for planets
    CreateDDSTextureFromFile(_pd3dDevice, L"sun texture.dds", nullptr, &_pSunTexture);

    CreateDDSTextureFromFile(_pd3dDevice, L"Mercury texture.dds", nullptr, &_pMercuryTexture);

    CreateDDSTextureFromFile(_pd3dDevice, L"venus surface.dds", nullptr, &_pVenusSurface);
    CreateDDSTextureFromFile(_pd3dDevice, L"venus atmos.dds", nullptr, &_pVenusAtmos);

    CreateDDSTextureFromFile(_pd3dDevice, L"earth.dds", nullptr, &_pEarthTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"moon texture.dds", nullptr, &_pMoonTexture);

    CreateDDSTextureFromFile(_pd3dDevice, L"mars texture.dds", nullptr, &_pMarsTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"phobos texture.dds", nullptr, &_pPhobosTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"deimos texture.dds", nullptr, &_pDeimosTexture);

    CreateDDSTextureFromFile(_pd3dDevice, L"asteroid texture.dds", nullptr, &_pAsteroidTexture);

    CreateDDSTextureFromFile(_pd3dDevice, L"jupiter texture.dds", nullptr, &_pJupiterTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"io texture.dds", nullptr, &_pIoTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"europa texture.dds", nullptr, &_pEuropaTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"ganymede texture.dds", nullptr, &_pGanymedeTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"callisto texture.dds", nullptr, &_pCallistoTexture);

    CreateDDSTextureFromFile(_pd3dDevice, L"saturn texture.dds", nullptr, &_pSaturnTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"enceladus texture.dds", nullptr, &_pEnceladusTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"titan texture.dds", nullptr, &_pTitanTexture);

    CreateDDSTextureFromFile(_pd3dDevice, L"uranus texture.dds", nullptr, &_pUranusTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"titania texture.dds", nullptr, &_pTitaniaTexture);
    CreateDDSTextureFromFile(_pd3dDevice, L"oberon texture.dds", nullptr, &_pOberonTexture);

    CreateDDSTextureFromFile(_pd3dDevice, L"neptune texture.dds", nullptr, &_pNeptuneTexture);

    CreateDDSTextureFromFile(_pd3dDevice, L"cubemap/px.dds", nullptr, &_pPlaneTexture);

    // Assignment B2
    //create planets
    sun = new SolarObject(sphereMesh, _pSunTexture, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.8824f, 0.7294f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));

    mercury = new SolarObject(sphereMesh, _pMercuryTexture, XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));

    venus = new SolarObject(sphereMesh, _pVenusSurface, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    venusAtmos = new SolarObject(sphereMesh, _pVenusAtmos, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));

    earth = new SolarObject(sphereMesh, _pEarthTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    moon = new SolarObject(sphereMesh, _pMoonTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));

    mars = new SolarObject(sphereMesh, _pMarsTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    phobos = new SolarObject(sphereMesh, _pPhobosTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    deimos = new SolarObject(sphereMesh, _pDeimosTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));

    jupiter = new SolarObject(sphereMesh, _pJupiterTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    io = new SolarObject(sphereMesh, _pIoTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    europa = new SolarObject(sphereMesh, _pEuropaTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    ganymede = new SolarObject(sphereMesh, _pGanymedeTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    callisto = new SolarObject(sphereMesh, _pCallistoTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));

    saturn = new SolarObject(sphereMesh, _pSaturnTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    enceladus = new SolarObject(sphereMesh, _pEnceladusTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    titan = new SolarObject(sphereMesh, _pTitanTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));

    uranus = new SolarObject(sphereMesh, _pUranusTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    titania = new SolarObject(sphereMesh, _pTitaniaTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
    oberon = new SolarObject(sphereMesh, _pOberonTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));

    neptune = new SolarObject(sphereMesh, _pNeptuneTexture, XMFLOAT4(0.64f, 0.64f, 0.64f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));

    //Generate random seed
    srand(time(nullptr));
    for (int i = 0; i < 10000; i++)
    {
        //get a random number between -12.6 and 12.6 for both x and z
        float x = ((-1 + 2 * ((float)rand()) / RAND_MAX) * 12.6f);
        float z = ((-1 + 2 * ((float)rand()) / RAND_MAX) * 12.6f);

        //calculate the distance from the origin
        float distance = sqrt((x * x) + (z * z));

        //loop until x and z are inside the outer circle, and outside the inner circle
        while(distance > 12.8f || distance < 12.4f)
        {
            x = ((-1 + 2 * ((float)rand()) / RAND_MAX) * 12.6f);
            z = ((-1 + 2 * ((float)rand()) / RAND_MAX) * 12.6f);

            distance = sqrt((x * x) + (z * z));
        }

        AsteroidArray[i] = new Asteroid(x, (-1 + 2 * ((float)rand()) / RAND_MAX) / 10, z, (((float)rand()) / RAND_MAX) / 50, (((float)rand()) / RAND_MAX) / 50, (((float)rand()) / RAND_MAX) / 50, (((float)rand()) / RAND_MAX) / 2, 1 / (((((float)rand()) / RAND_MAX) * (6 - 3) + 3) * 365));
    }

    //Generate saturn's rings
    //inner ring
    for (int i = 0; i < 750; i++)
    {
        const float innerRadius = 2.5f;
	    //get random position for x and z within a circle with radius 2.5f
        float x = ((-1 + 2 * ((float)rand()) / RAND_MAX) * innerRadius);
        float z = ((-1 + 2 * ((float)rand()) / RAND_MAX) * innerRadius);

        //calculate distance from origin
        float distance = sqrt((x * x) + (z * z));

        //loop until x and z are within the ring, but a set distance from saturn
        float distanceFromSaturn = 1.75f;
        while(distance > innerRadius || distance < distanceFromSaturn)
        {
            x = ((-1 + 2 * ((float)rand()) / RAND_MAX) * innerRadius);
            z = ((-1 + 2 * ((float)rand()) / RAND_MAX) * innerRadius);

            distance = sqrt((x * x) + (z * z));
        }

        SaturnInnerRingArray[i] = new Asteroid(x, ((float)rand() / RAND_MAX) / 10, z, (((float)rand()) / RAND_MAX) / 15, (((float)rand()) / RAND_MAX) / 15, (((float)rand()) / RAND_MAX) / 15, 4.8f, 4.8f);
    }

    //middle ring
    for(int i = 0; i < 1000; i++)
    {
        const float middleRadius = 3.5f;
        //Get random position for x and z within a circle with radius 4.0f
        float x = ((-1 + 2 * ((float)rand()) / RAND_MAX) * middleRadius);
        float z = ((-1 + 2 * ((float)rand()) / RAND_MAX) * middleRadius);

        //calculate distance from origin
        float distance = sqrt((x * x) + (z * z));

        //loop until x and z are within the ring, but a set distance from saturn
        float distanceFromSaturn = 3.0f;
        while (distance > middleRadius || distance < distanceFromSaturn)
        {
            x = ((-1 + 2 * ((float)rand()) / RAND_MAX) * middleRadius);
            z = ((-1 + 2 * ((float)rand()) / RAND_MAX) * middleRadius);

            distance = sqrt((x * x) + (z * z));
        }

        SaturnMidRingArray[i] = new Asteroid(x, ((float)rand() / RAND_MAX) / 10, z, (((float)rand()) / RAND_MAX) / 15, (((float)rand()) / RAND_MAX) / 15, (((float)rand()) / RAND_MAX) / 15, 3.69f, 3.69f);
    }

    //Outer Ring
    for (int i = 0; i < 1500; i++)
    {
        const float outerRadius = 5.5f;
        //Get random position for x and z within a circle with radius 4.0f
        float x = ((-1 + 2 * ((float)rand()) / RAND_MAX) * outerRadius);
        float z = ((-1 + 2 * ((float)rand()) / RAND_MAX) * outerRadius);

        //calculate distance from origin
        float distance = sqrt((x * x) + (z * z));

        //loop until x and z are within the ring, but a set distance from saturn
        float distanceFromSaturn = 3.75f;
        while (distance > outerRadius || distance < distanceFromSaturn)
        {
            x = ((-1 + 2 * ((float)rand()) / RAND_MAX) * outerRadius);
            z = ((-1 + 2 * ((float)rand()) / RAND_MAX) * outerRadius);

            distance = sqrt((x * x) + (z * z));
        }

        SaturnOuterRingArray[i] = new Asteroid(x, ((float)rand() / RAND_MAX) / 10, z, (((float)rand()) / RAND_MAX) / 15, (((float)rand()) / RAND_MAX) / 15, (((float)rand()) / RAND_MAX) / 15, 3.69f, 3.69f);
    }

    if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

    return hr;
}

HRESULT Application::InitVertexBuffer()
{
    HRESULT hr;

    // Assignment B1
    
    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        //top right on front side
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.333333f, 0.666667f, -0.666667f), XMFLOAT2(1.0f, 1.0f) },

        //top left on front side
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-0.816497f, 0.408248f, -0.408248f), XMFLOAT2(0.0f, 1.0f) },

        //top left at back
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-0.333333f, 0.666667f, 0.666667f), XMFLOAT2(0.0f, 0.0f) },

        //top right at back
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.816497f, 0.408248f, 0.408248f), XMFLOAT2(1.0f, 0.0f) },

        //bottom right at front
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.666667f, -0.666667f, -0.333333f), XMFLOAT2(1.0f, 1.0f) },

        //bottom left at front
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-0.408248f, -0.408248f, -0.816487f), XMFLOAT2(0.0f, 1.0f) },

        //bottom left at back
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-0.666667f, -0.666667f, 0.333333f), XMFLOAT2(0.0f, 0.0f) },

        //bottom right at back
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.408248f, -0.408248f, 0.816497f), XMFLOAT2(1.0f, 0.0f) },
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    // Create pyramid vertex buffer
    SimpleVertex pyramidvertices[] =
    {
        //top middle
        { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

        //bottom left at front
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-0.298157f, -0.149071f, 0.0f) },

        //bottom right at front
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.298157f, -0.149071f, 0.0f) },

        //bottom left at back
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-0.298157f, 0.035191f, 0.298157f) },

        //bottom right at back
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(-0.298157f, 0.035191f, 0.298157f) },
    };

    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 5;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = pyramidvertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPyramidVertexBuffer);

    // Create plane vertex buffer
    SimpleVertex planevertices[] =
    {
        { XMFLOAT3(-2.0f, 0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.25f, 1.0f) },
        { XMFLOAT3(0.0f, 0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.6f, 1.0f) },
        { XMFLOAT3(1.0f, 0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(0.75f, 1.0f) },
        { XMFLOAT3(2.0f, 0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-2.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 0.75f) },
        { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.25f, 0.75f) },
        { XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(0.5f, 0.75f) },
        { XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(0.75f, 0.75f) },
        { XMFLOAT3(2.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(1.0f, 0.75f) },
        { XMFLOAT3(-2.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(0.0f, 0.5f) },
        { XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(0.25f, 0.5f) },
        { XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  ,XMFLOAT2(0.5f, 0.5f) }, 
        { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  ,XMFLOAT2(0.75f, 0.5f) },
        { XMFLOAT3(2.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  ,XMFLOAT2(1.0f, 0.5f) },
        { XMFLOAT3(-2.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(0.0f, 0.25f) },
        { XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(0.25f, 0.25f) },
        { XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  ,XMFLOAT2(0.5f, 0.25f) },
        { XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  ,XMFLOAT2(0.75f, 0.25f) },
        { XMFLOAT3(2.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  ,XMFLOAT2(1.0f, 0.25f) },
        { XMFLOAT3(-2.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) ,XMFLOAT2(0.25f, 0.0f) },
        { XMFLOAT3(0.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  ,XMFLOAT2(0.5f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  ,XMFLOAT2(0.75f, 0.0f) },
        { XMFLOAT3(2.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)  ,XMFLOAT2(1.0f, 0.0f) },
    };

    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 25;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = planevertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneVertexBuffer);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
    HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
        0,1,2,
        0,2,3,
        0,4,5,
        0,5,1,
        1,5,6,
        1,6,2,
        2,6,7,
        2,7,3,
        3,7,4,
        3,4,0,
        4,7,6,
        4,6,5
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    // Create pyramid index buffer
    WORD pyramidindices[] =
    {
         0,2,1,
         0,1,3,
         0,4,2,
         0,3,4,
         1,2,3,
         3,2,4
    };

    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 18;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = pyramidindices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPyramidIndexBuffer);

    // Create plane index buffer
    WORD planeindices[] =
    {
        0,5,6,
        0,6,1,
        1,6,7,
        1,7,2,
        2,7,8,
        2,8,3,
        3,8,9,
        3,9,4,
        5,10,11,
        5,11,6,
        6,11,12,
        6,12,7,
        7,12,13,
        7,13,8,
        8,13,14,
        8,14,9,
        10,15,16,
        10,16,11,
        11,16,17,
        11,17,12,
        12,17,18,
        12,18,13,
        13,18,19,
        13,19,14,
        15,20,21,
        15,21,16,
        16,21,22,
        16,22,17,
        17,22,23,
        17,23,18,
        18,23,24,
        18,24,19
    };

    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 96;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = planeindices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneIndexBuffer);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = { 0, 0, 640, 480 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);

    lightDirection = XMFLOAT3(-0.25f, 0.0f, 0.5f);
    diffuseMaterial = XMFLOAT4(0.8f, 0.5f, 0.5f, 1.0f);
    diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);
    ambientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);

    specularMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    specularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    specularPower = 10.0f;
    eyePos = XMFLOAT3(0.0f, 0.0f, -3.0f);

    if (!_hWnd)
        return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };


    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    //define depth buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = _WindowWidth;
    depthStencilDesc.Height = _WindowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    //create depth buffer
    _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
    _pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);



    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

    InitShadersAndInputLayout();

    InitVertexBuffer();

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);

    InitIndexBuffer();

    // Set index buffer
    _pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create the constant buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    //declare wire frame desc
    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    wfdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

    _pImmediateContext->RSSetState(_wireFrame);

    //declare solid fill desc
    D3D11_RASTERIZER_DESC solidDesc;
    ZeroMemory(&solidDesc, sizeof(D3D11_RASTERIZER_DESC));
    solidDesc.FillMode = D3D11_FILL_SOLID;
    solidDesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&solidDesc, &_solid);

    //create blend state
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(rtbd));
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;
    _pd3dDevice->CreateBlendState(&blendDesc, &Transparency);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();
    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pPyramidVertexBuffer) _pPyramidVertexBuffer->Release();
    if (_pPlaneVertexBuffer) _pPlaneVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
    if (_pPyramidIndexBuffer) _pPyramidIndexBuffer->Release();
    if (_pPlaneIndexBuffer) _pPlaneIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
    if (_depthStencilView) _depthStencilView->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();
    if (_wireFrame) _wireFrame->Release();
    if (_solid) _solid->Release();
    if (_pTextureRV) _pTextureRV->Release();
    if (_pSamplerLinear) _pSamplerLinear->Release();
    if (Transparency) Transparency->Release();

    if (_pEarthTexture) _pEarthTexture->Release();
    if (_pMoonTexture) _pMoonTexture->Release();

    if (_pSunTexture) _pSunTexture->Release();

    if (_pMercuryTexture) _pMercuryTexture->Release();

    if (_pVenusSurface) _pVenusSurface->Release();
    if (_pVenusAtmos) _pVenusAtmos->Release();

    if (_pMarsTexture) _pMarsTexture->Release();
    if (_pPhobosTexture) _pPhobosTexture->Release();
    if (_pDeimosTexture) _pDeimosTexture->Release();

    if (_pAsteroidTexture) _pAsteroidTexture->Release();

    if (_pJupiterTexture) _pJupiterTexture->Release();
    if (_pIoTexture) _pIoTexture->Release();
    if (_pGanymedeTexture) _pGanymedeTexture->Release();
    if (_pEuropaTexture) _pEuropaTexture->Release();
    if (_pCallistoTexture) _pCallistoTexture->Release();

    if (_pSaturnTexture) _pSaturnTexture->Release();
    if (_pEnceladusTexture) _pEnceladusTexture->Release();
    if (_pTitanTexture) _pTitanTexture->Release();

    if (_pUranusTexture) _pUranusTexture->Release();
    if (_pTitaniaTexture) _pTitaniaTexture->Release();
    if (_pOberonTexture) _pOberonTexture->Release();

    if (_pNeptuneTexture) _pNeptuneTexture->Release();

    if (_pPlaneTexture) _pPlaneTexture->Release();

    earth = nullptr;
    moon = nullptr;

    sun = nullptr;

    venus = nullptr;
    venusAtmos = nullptr;

    mercury = nullptr;

    mars = nullptr;
    phobos = nullptr;
    deimos = nullptr;

    jupiter = nullptr;
    europa = nullptr;
    io = nullptr;
    ganymede = nullptr;
    callisto = nullptr;

    saturn = nullptr;
    enceladus = nullptr;
    titan = nullptr;

    uranus = nullptr;
    titania = nullptr;
    oberon = nullptr;
    
    neptune = nullptr;

    delete earth, moon, sun, venus, venusAtmos, mercury, mars, phobos, jupiter, europa, io, ganymede, callisto, saturn, enceladus, titan, uranus, titania, oberon, neptune;

    for(int i = 0; i < 10000; i++)
    {
        delete AsteroidArray[i];
    }
    for(int i = 0; i < 750; i++)
    {
        delete SaturnInnerRingArray[i];
    }
    for(int i = 0; i < 1000; i++)
    {
        delete SaturnMidRingArray[i];
    }
    for(int i = 0; i < 1500; i++)
    {
        delete SaturnOuterRingArray[i];
    }
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float)XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount64();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    gTime = t;

    //get button press
    if (GetAsyncKeyState(VK_LEFT))
    {
        //if the left arrow key is pressed, set the rasterize state to wireframe
        _pImmediateContext->RSSetState(_wireFrame);
    }
    else if (GetAsyncKeyState(VK_RIGHT))
    {
        //else if the right arrow key is pressed, set the rasterize state to solid
        _pImmediateContext->RSSetState(_solid);
    }

    //get change in camera
    if (GetAsyncKeyState(VK_NUMPAD0))
    {
        currentCam = 0;
    }
    else if (GetAsyncKeyState(VK_NUMPAD1))
    {
        currentCam = 1;
    }
    else if (GetAsyncKeyState(VK_NUMPAD2))
    {
        currentCam = 2;
    }
    else if (GetAsyncKeyState(VK_NUMPAD3))
    {
        currentCam = 3;
    }
    else if (GetAsyncKeyState(VK_NUMPAD4))
    {
        currentCam = 4;
    }
    else if (GetAsyncKeyState(VK_NUMPAD5))
    {
        currentCam = 5;
    }
    else if (GetAsyncKeyState(VK_NUMPAD6))
    {
        currentCam = 6;
    }
    else if (GetAsyncKeyState(VK_NUMPAD7))
    {
        currentCam = 7;
    }
    else if (GetAsyncKeyState(VK_NUMPAD8))
    {
        currentCam = 8;
    }
    else if (GetAsyncKeyState(VK_NUMPAD9))
    {
        currentCam = 9;
    }

    //Sun
    XMStoreFloat4x4(&_sun, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationY(0.037f * t * simulationSpeed));

    //SunCamera - Camera that follows the sun
    XMStoreFloat4x4(&_sunCameraPos, XMMatrixTranslation(0.0f, 2.0f, -3.0f) * XMLoadFloat4x4(&_sun));
    SunCamera->Update(_sunCameraPos, _sun);

    //Mercury
    XMStoreFloat4x4(&_mercury, XMMatrixScaling(0.1f, 0.1f, 0.1f)* XMMatrixRotationY(0.01695f * t * simulationSpeed)* XMMatrixTranslation(2.5f, 0.0f, 0.0f)* XMMatrixRotationY(0.01136f * t * simulationSpeed));

    //Mercury Camera
    XMStoreFloat4x4(&_MercuryCameraPos, XMMatrixTranslation(0.0f, 2.0f, -3.0f) * XMLoadFloat4x4(&_mercury));
    MercuryCamera->Update(_MercuryCameraPos, _mercury);

    //VenusCamera - Camera that follows venus
    XMStoreFloat4x4(&_VenusCameraPos, XMMatrixTranslation(0.0f, 2.0f, -3.0f) * XMLoadFloat4x4(&_venus));
    VenusCamera->Update(_VenusCameraPos, _venus);

    //Venus Surface
    XMStoreFloat4x4(&_venus, XMMatrixScaling(0.2f, 0.2f, 0.2f)* XMMatrixRotationY(0.004115f * t * simulationSpeed)* XMMatrixTranslation(4.5f, 0.0f, 0.0f)* XMMatrixRotationY(0.00446f * t * simulationSpeed));

    //Venus Atmos
    XMStoreFloat4x4(&_venusAtmos, XMMatrixScaling(0.24f, 0.24f, 0.24f) * XMMatrixRotationY(0.004115f * t * 25 * simulationSpeed) * XMMatrixTranslation(4.5f, 0.0f, 0.0f) * XMMatrixRotationY(0.00446f * t * simulationSpeed));

    //Earth
    XMStoreFloat4x4(&_earth, XMMatrixScaling(0.2106f, 0.2106f, 0.2106f) * XMMatrixRotationY(t * simulationSpeed) * XMMatrixTranslation(8.032f, 0.0f, 0.0f) * XMMatrixRotationY(0.0027397f * t * simulationSpeed));

    //EarthCamera - Camera that follows earth
    XMStoreFloat4x4(&_EarthCameraPos, XMMatrixTranslation(0.0f, 2.0f, -3.0f) * XMLoadFloat4x4(&_earth));
    EarthCamera->Update(_EarthCameraPos, _earth);

    //Moon
    XMStoreFloat4x4(&_moon, XMMatrixScaling(0.25f, 0.25f, 0.25f) * XMMatrixRotationY(0.037f * t * simulationSpeed) * XMMatrixTranslation(2.50f, 0.0f, 0.0f) * XMMatrixRotationY(0.037f * t * simulationSpeed) * XMLoadFloat4x4(&_earth));

    //Mars
    XMStoreFloat4x4(&_mars, XMMatrixScaling(0.11214f, 0.11214f, 0.11214f) * XMMatrixRotationY(1.025f * t * simulationSpeed) * XMMatrixTranslation(11.6446f, 0.0f, 0.0f) * XMMatrixRotationY(0.0014556f * t * simulationSpeed));

    //Mars Camera
    XMStoreFloat4x4(&_MarsCameraPos, XMMatrixTranslation(0.0f, 2.0f, -3.0f) * XMLoadFloat4x4(&_mars));
    MarsCamera->Update(_MarsCameraPos, _mars);

    //Phobos
    XMStoreFloat4x4(&_phobos, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(3.125f * t * simulationSpeed) * XMMatrixTranslation(2.0f, 0.0f, 0.0f) * XMMatrixRotationY(3.125f * t * simulationSpeed) * XMLoadFloat4x4(&_mars));

    //Deimos
    XMStoreFloat4x4(&_deimos, XMMatrixScaling(0.05f, 0.05f, 0.05f) * XMMatrixRotationY(0.79f * t * simulationSpeed) * XMMatrixTranslation(3.0f, 0.0f, 0.0f) * XMMatrixRotationY(0.79f * t * simulationSpeed) * XMLoadFloat4x4(&_mars));

    //Asteroid Belt
    for(int i = 0; i < 10000; i++)
    {
        AsteroidArray[i]->Update(t, simulationSpeed);
    }

    //Jupiter
    XMStoreFloat4x4(&_jupiter, XMMatrixScaling(1.053f, 1.053f, 1.053f) * XMMatrixRotationY(2.4f * t * simulationSpeed) * XMMatrixTranslation(20.5f, 0.0f, 0.0f) * XMMatrixRotationY(0.0002283f * t * simulationSpeed));

    //Jupiter Camera
    XMStoreFloat4x4(&_JupiterCameraPos, XMMatrixTranslation(0.0f, 2.0f, -3.0f) * XMLoadFloat4x4(&_jupiter));
    JupiterCamera->Update(_JupiterCameraPos, _jupiter);

    //Io
    XMStoreFloat4x4(&_io, XMMatrixScaling(0.03456f, 0.03456f, 0.03456f)* XMMatrixRotationY(0.556f * t * simulationSpeed)* XMMatrixTranslation(1.25f, 0.0f, 0.0f)* XMMatrixRotationY(0.556f * t * simulationSpeed)* XMLoadFloat4x4(&_jupiter));

	//Europa
    XMStoreFloat4x4(&_europa, XMMatrixScaling(0.0484f, 0.0484f, 0.0484f) * XMMatrixRotationY(0.2857f * t * simulationSpeed) * XMMatrixTranslation(2.25f, 0.0f, 0.0f) * XMMatrixRotationY(0.28957f * t * simulationSpeed) * XMLoadFloat4x4(&_jupiter));

    //Ganymede
    XMStoreFloat4x4(&_ganymede, XMMatrixScaling(0.080256f, 0.080256f, 0.080256f)* XMMatrixRotationY(0.1395f * t * simulationSpeed)* XMMatrixTranslation(3.25f, 0.0f, 0.0f)* XMMatrixRotationY(0.1395f * t * simulationSpeed)* XMLoadFloat4x4(&_jupiter));

    //Callisto
    XMStoreFloat4x4(&_callisto, XMMatrixScaling(0.08f, 0.08f, 0.08f)* XMMatrixRotationY(0.0588f * t * simulationSpeed)* XMMatrixTranslation(4.25f, 0.0f, 0.0f)* XMMatrixRotationY(0.0588f * t * simulationSpeed)* XMLoadFloat4x4(&_jupiter));

    //Saturn
    XMStoreFloat4x4(&_saturn, XMMatrixScaling(1.0f, 1.0f, 1.0f)* XMMatrixRotationY(2.233f * t * simulationSpeed) * XMMatrixTranslation(40.0f, 0.0f, 0.0f) * XMMatrixRotationY(0.00009447f * t * simulationSpeed));

    //Saturn Camera
    XMStoreFloat4x4(&_SaturnCameraPos, XMMatrixTranslation(0.0f, 2.0f, -6.5f) * XMLoadFloat4x4(&_saturn));
    SaturnCamera->Update(_SaturnCameraPos, _saturn);

    //Saturn Inner Ring
    for(int i = 0; i < 750; i++)
    {
        SaturnInnerRingArray[i]->Update(t, simulationSpeed, _saturn);
    }

    //Saturn Middle Ring
    for(int i = 0; i < 1000; i++)
    {
        SaturnMidRingArray[i]->Update(t, simulationSpeed, _saturn);
    }

    //Saturn Outer Ring
    for(int i = 0; i < 1500; i++)
    {
        SaturnOuterRingArray[i]->Update(t, simulationSpeed, _saturn);
    }

    //Enceladus
    XMStoreFloat4x4(&_enceladus, XMMatrixScaling(0.0535f, 0.0535f, 0.0535f) * XMMatrixRotationY(0.7299f * t * simulationSpeed) * XMMatrixTranslation(6.0f, 0.0f, 0.0f) * XMMatrixRotationY(0.7299f * t * simulationSpeed) * XMLoadFloat4x4(&_saturn));

    //Titan
    XMStoreFloat4x4(&_titan, XMMatrixScaling(0.235f, 0.235f, 0.235f)* XMMatrixRotationY(0.0625f * t * simulationSpeed)* XMMatrixTranslation(8.0f, 0.0f, 0.0f)* XMMatrixRotationY(0.0625f * t * simulationSpeed)* XMLoadFloat4x4(&_saturn));

    //Uranus
    XMStoreFloat4x4(&_uranus, XMMatrixScaling(0.4355f, 0.4355f, 0.4355f)* XMMatrixRotationY(1.412f * t * simulationSpeed)* XMMatrixTranslation(60.0f, 0.0f, 0.0f)* XMMatrixRotationY(0.000032615f * t * simulationSpeed));

    //Uranus Camera
    XMStoreFloat4x4(&_UranusCameraPos, XMMatrixTranslation(0.0f, 2.0f, -3.0f) * XMLoadFloat4x4(&_uranus));
    UranusCamera->Update(_UranusCameraPos, _uranus);

    //Titania
    XMStoreFloat4x4(&_titania, XMMatrixScaling(0.1f, 0.1f, 0.1f)* XMMatrixRotationY(0.1148f * t * simulationSpeed)* XMMatrixTranslation(3.0f, 0.0f, 0.0f)* XMMatrixRotationY(0.1148f * t * simulationSpeed)* XMLoadFloat4x4(&_uranus));

    //Oberon
    XMStoreFloat4x4(&_oberon, XMMatrixScaling(0.08f, 0.08f, 0.08f)* XMMatrixRotationY(0.0769f * t * simulationSpeed)* XMMatrixTranslation(4.5f, 0.0f, 0.0f)* XMMatrixRotationY(0.0769f * t * simulationSpeed)* XMLoadFloat4x4(&_uranus));

    //Neptune
    XMStoreFloat4x4(&_neptune, XMMatrixScaling(0.4155f, 0.4155f, 0.4155f)* XMMatrixRotationY(1.5f * t * simulationSpeed)* XMMatrixTranslation(75.0f, 0.0f, 0.0f)* XMMatrixRotationY(0.0000166f * t * simulationSpeed));

    //Neptune Camera
    XMStoreFloat4x4(&_NeptuneCameraPos, XMMatrixTranslation(0.0f, 2.0f, -3.0f) * XMLoadFloat4x4(&_neptune));
    NeptuneCamera->Update(_NeptuneCameraPos, _neptune);

    //Free Camera
    if(currentCam == 9)
    {
	    if(GetAsyncKeyState('D'))
	    {
            FreeCamera->SetPosition(XMFLOAT3(FreeCamera->GetFloatPos().x + 0.01f, FreeCamera->GetFloatPos().y, FreeCamera->GetFloatPos().z));
            FreeCamera->SetLookAt(XMFLOAT3(FreeCamera->GetFloatAt().x + 0.01f, FreeCamera->GetFloatAt().y, FreeCamera->GetFloatAt().z));
	    }
        if(GetAsyncKeyState('A'))
        {
            FreeCamera->SetPosition(XMFLOAT3(FreeCamera->GetFloatPos().x - 0.01f, FreeCamera->GetFloatPos().y, FreeCamera->GetFloatPos().z));
            FreeCamera->SetLookAt(XMFLOAT3(FreeCamera->GetFloatAt().x - 0.01f, FreeCamera->GetFloatAt().y, FreeCamera->GetFloatAt().z));
        }
        if(GetAsyncKeyState('W'))
        {
            FreeCamera->SetPosition(XMFLOAT3(FreeCamera->GetFloatPos().x, FreeCamera->GetFloatPos().y, FreeCamera->GetFloatPos().z + 0.01f));
            FreeCamera->SetLookAt(XMFLOAT3(FreeCamera->GetFloatAt().x, FreeCamera->GetFloatAt().y, FreeCamera->GetFloatAt().z + 0.01f));
        }
        if (GetAsyncKeyState('S'))
        {
            FreeCamera->SetPosition(XMFLOAT3(FreeCamera->GetFloatPos().x, FreeCamera->GetFloatPos().y, FreeCamera->GetFloatPos().z - 0.01f));
            FreeCamera->SetLookAt(XMFLOAT3(FreeCamera->GetFloatAt().x, FreeCamera->GetFloatAt().y, FreeCamera->GetFloatAt().z - 0.01f));
        }
        if (GetAsyncKeyState(VK_SPACE))
        {
            FreeCamera->SetPosition(XMFLOAT3(FreeCamera->GetFloatPos().x, FreeCamera->GetFloatPos().y + 0.01f, FreeCamera->GetFloatPos().z));
            FreeCamera->SetLookAt(XMFLOAT3(FreeCamera->GetFloatAt().x, FreeCamera->GetFloatAt().y + 0.01f, FreeCamera->GetFloatAt().z));
        }
        if (GetAsyncKeyState(VK_CONTROL))
        {
            FreeCamera->SetPosition(XMFLOAT3(FreeCamera->GetFloatPos().x, FreeCamera->GetFloatPos().y - 0.01f, FreeCamera->GetFloatPos().z));
            FreeCamera->SetLookAt(XMFLOAT3(FreeCamera->GetFloatAt().x, FreeCamera->GetFloatAt().y - 0.01f, FreeCamera->GetFloatAt().z));
        }
    }
    FreeCamera->Update();

    ////Back plane
    //XMStoreFloat4x4(&_backPlane, XMMatrixScaling(25.0f, 25.0f, 25.0f) * XMMatrixTranslation(0.0, -5.0f, 0.0f));
}

void Application::Draw()
{
    //set the defualt blend state (no blending) for opaque objects
    _pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

    //render opaque objects

    //set buffers to the cube buffers
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);

    _pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.1f, 0.1f, 0.15f, 1.0f }; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
    _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    XMMATRIX world = XMLoadFloat4x4(&_world);
    XMMATRIX view = XMLoadFloat4x4(&SunCamera->GetViewMatrix());
    XMMATRIX projection = XMLoadFloat4x4(&SunCamera->GetProjectionMatrix());

    switch(currentCam)
    {
    case 0:
        view = XMLoadFloat4x4(&SunCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&SunCamera->GetProjectionMatrix());
        break;
    case 1:
        view = XMLoadFloat4x4(&MercuryCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&MercuryCamera->GetProjectionMatrix());
        break;
    case 2:
        view = XMLoadFloat4x4(&VenusCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&VenusCamera->GetProjectionMatrix());
        break;
    case 3:
        view = XMLoadFloat4x4(&EarthCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&EarthCamera->GetProjectionMatrix());
        break;
    case 4:
        view = XMLoadFloat4x4(&MarsCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&MarsCamera->GetProjectionMatrix());
        break;
    case 5:
        view = XMLoadFloat4x4(&JupiterCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&JupiterCamera->GetProjectionMatrix());
        break;
    case 6:
        view = XMLoadFloat4x4(&SaturnCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&SaturnCamera->GetProjectionMatrix());
        break;
    case 7:
        view = XMLoadFloat4x4(&UranusCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&UranusCamera->GetProjectionMatrix());
        break;
    case 8:
        view = XMLoadFloat4x4(&NeptuneCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&NeptuneCamera->GetProjectionMatrix());
        break;
    case 9:
        view = XMLoadFloat4x4(&FreeCamera->GetViewMatrix());
        projection = XMLoadFloat4x4(&FreeCamera->GetProjectionMatrix());
        break;
    }

    //
    // Update variables
    //
    ConstantBuffer cb;

    //gPointLight Data
    cb.gPointLight.Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
    cb.gPointLight.Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    cb.gPointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    cb.gPointLight.Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    cb.gPointLight.Att = XMFLOAT3(0.5f, 0.02f, 0.0f);
    cb.gPointLight.Range = 100.0f;

    //gSpotLights data
    for(int i = 0; i < 5; i++)
    {
        switch (i)
        {
        case 0:
            cb.gSpotLights[i].Position = XMFLOAT3(0.0f, 3.0f, 0.0f);
            cb.gSpotLights[i].Direction = XMFLOAT3(0.0f, -3.0f, 0.0f);
            break;
        case 1:
            cb.gSpotLights[i].Position = XMFLOAT3(0.0f, -3.0f, 0.0f);
            cb.gSpotLights[i].Direction = XMFLOAT3(0.0f, 3.0f, 0.0f);
            break;
        case 2:
            cb.gSpotLights[i].Position = XMFLOAT3(-3.0f, 0.0f, 0.0f);
            cb.gSpotLights[i].Direction = XMFLOAT3(3.0f, 0.0f, 0.0f);
            break;
        case 3:
            cb.gSpotLights[i].Position = XMFLOAT3(3.0f, 0.0f, 0.0f);
            cb.gSpotLights[i].Direction = XMFLOAT3(-3.0f, 0.0f, 0.0f);
            break;
        case 4:
            cb.gSpotLights[i].Position = XMFLOAT3(0.0f, 0.0f, -3.0f);
            cb.gSpotLights[i].Direction = XMFLOAT3(0.0f, 0.0f, 3.0f);
            break;
        case 5:
            cb.gSpotLights[i].Position = XMFLOAT3(0.0f, 0.0f, 3.0f);
            cb.gSpotLights[i].Direction = XMFLOAT3(0.0f, 0.0f, -3.0f);
            break;
        }

        cb.gSpotLights[i].Ambient = ambientLight;
        cb.gSpotLights[i].Diffuse = diffuseLight;
        cb.gSpotLights[i].Specular = specularLight;
        cb.gSpotLights[i].Range = 3.0f;
        cb.gSpotLights[i].Spot = 0.5f;
        cb.gSpotLights[i].Att = XMFLOAT3(0.5f, 0.01f, 0.0f);
    }

    cb.mWorld = XMMatrixTranspose(world);
    cb.mView = XMMatrixTranspose(view);
    cb.mProjection = XMMatrixTranspose(projection);
    cb.gTime = gTime;
    cb.DiffuseMtrl = diffuseMaterial;
    cb.AmbientMtrl = ambientMaterial;
    cb.SpecularMtrl = specularMaterial;

    switch(currentCam)
    {
    case 0:
        cb.EyePosW = XMFLOAT3(SunCamera->GetPosition()._41, SunCamera->GetPosition()._42, SunCamera->GetPosition()._43);
        break;
    case 1:
        cb.EyePosW = XMFLOAT3(MercuryCamera->GetPosition()._41, MercuryCamera->GetPosition()._42, MercuryCamera->GetPosition()._43);
        break;
    case 2:
        cb.EyePosW = XMFLOAT3(VenusCamera->GetPosition()._41, VenusCamera->GetPosition()._42, VenusCamera->GetPosition()._43);
        break;
    case 3:
        cb.EyePosW = XMFLOAT3(EarthCamera->GetPosition()._41, EarthCamera->GetPosition()._42, EarthCamera->GetPosition()._43);
        break;
    case 4:
        cb.EyePosW = XMFLOAT3(MarsCamera->GetPosition()._41, MarsCamera->GetPosition()._42, MarsCamera->GetPosition()._43);
        break;
    case 5:
        cb.EyePosW = XMFLOAT3(JupiterCamera->GetPosition()._41, JupiterCamera->GetPosition()._42, JupiterCamera->GetPosition()._43);
        break;
    case 6:
        cb.EyePosW = XMFLOAT3(SaturnCamera->GetPosition()._41, SaturnCamera->GetPosition()._42, SaturnCamera->GetPosition()._43);
        break;
    case 7:
        cb.EyePosW = XMFLOAT3(UranusCamera->GetPosition()._41, UranusCamera->GetPosition()._42, UranusCamera->GetPosition()._43);
        break;
    case 8:
        cb.EyePosW = XMFLOAT3(NeptuneCamera->GetPosition()._41, NeptuneCamera->GetPosition()._42, NeptuneCamera->GetPosition()._43);
        break;
    case 9:
        cb.EyePosW = XMFLOAT3(FreeCamera->GetPosition()._41, FreeCamera->GetPosition()._42, FreeCamera->GetPosition()._43);
        break;
    }

    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    //
    // Renders a triangle
    //
    _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
    _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

    //plane
    //_pImmediateContext->IASetVertexBuffers(0, 1, &_pPlaneVertexBuffer, &stride, &offset);
    //_pImmediateContext->IASetIndexBuffer(_pPlaneIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    //_pImmediateContext->PSSetShaderResources(0, 1, &_pPlaneTexture);
    //world = XMLoadFloat4x4(&_backPlane);
    //cb.mWorld = XMMatrixTranspose(world);
    //_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    //_pImmediateContext->DrawIndexed(96, 0, 0);

    //Mercury
    _pImmediateContext->IASetVertexBuffers(0, 1, &sphereMesh.VertexBuffer, &sphereMesh.VBStride, &sphereMesh.VBOffset);
    _pImmediateContext->IASetIndexBuffer(sphereMesh.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    _pImmediateContext->PSSetShaderResources(0, 1, &_pMercuryTexture);
    world = XMLoadFloat4x4(&_mercury);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Venus Surface
    _pImmediateContext->PSSetShaderResources(0, 1, &_pVenusSurface);
    world = XMLoadFloat4x4(&_venus);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Earth
    _pImmediateContext->PSSetShaderResources(0, 1, &_pEarthTexture);
    world = XMLoadFloat4x4(&_earth);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Moon
    _pImmediateContext->PSSetShaderResources(0, 1, &_pMoonTexture);
    world = XMLoadFloat4x4(&_moon);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Mars
    _pImmediateContext->PSSetShaderResources(0, 1, &_pMarsTexture);
    world = XMLoadFloat4x4(&_mars);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Phobos
    _pImmediateContext->PSSetShaderResources(0, 1, &_pPhobosTexture);
    world = XMLoadFloat4x4(&_phobos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Deimos
    _pImmediateContext->PSSetShaderResources(0, 1, &_pDeimosTexture);
    world = XMLoadFloat4x4(&_deimos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Asteroid belt
    _pImmediateContext->PSSetShaderResources(0, 1, &_pAsteroidTexture);
    for(int i = 0; i < 10000; i++)
    {
        world = XMLoadFloat4x4(&AsteroidArray[i]->GetMatrix());
        cb.mWorld = XMMatrixTranspose(world);
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
        _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);
    }

    //Jupiter
    _pImmediateContext->PSSetShaderResources(0, 1, &_pJupiterTexture);
    world = XMLoadFloat4x4(&_jupiter);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Io
    _pImmediateContext->PSSetShaderResources(0, 1, &_pIoTexture);
    world = XMLoadFloat4x4(&_io);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Europa
    _pImmediateContext->PSSetShaderResources(0, 1, &_pEuropaTexture);
    world = XMLoadFloat4x4(&_europa);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Ganymede
    _pImmediateContext->PSSetShaderResources(0, 1, &_pGanymedeTexture);
    world = XMLoadFloat4x4(&_ganymede);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Callisto
    _pImmediateContext->PSSetShaderResources(0, 1, &_pCallistoTexture);
    world = XMLoadFloat4x4(&_callisto);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Saturn
    _pImmediateContext->PSSetShaderResources(0, 1, &_pSaturnTexture);
    world = XMLoadFloat4x4(&_saturn);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Saturn Inner ring
    _pImmediateContext->PSSetShaderResources(0, 1, &_pAsteroidTexture);
    for (int i = 0; i < 750; i++)
    {
        world = XMLoadFloat4x4(&SaturnInnerRingArray[i]->GetMatrix());
        cb.mWorld = XMMatrixTranspose(world);
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
        _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);
    }

    //Saturn Mid Ring
    for(int i = 0; i < 1000; i++)
    {
        world = XMLoadFloat4x4(&SaturnMidRingArray[i]->GetMatrix());
        cb.mWorld = XMMatrixTranspose(world);
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
        _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);
    }

    //Saturn Outer Ring
    for(int i = 0; i < 1500; i++)
    {
        world = XMLoadFloat4x4(&SaturnOuterRingArray[i]->GetMatrix());
        cb.mWorld = XMMatrixTranspose(world);
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
        _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);
    }

    //Enceladus
    _pImmediateContext->PSSetShaderResources(0, 1, &_pEnceladusTexture);
    world = XMLoadFloat4x4(&_enceladus);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Titan
    _pImmediateContext->PSSetShaderResources(0, 1, &_pTitanTexture);
    world = XMLoadFloat4x4(&_titan);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Uranus
    _pImmediateContext->PSSetShaderResources(0, 1, &_pUranusTexture);
    world = XMLoadFloat4x4(&_uranus);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Titania
    _pImmediateContext->PSSetShaderResources(0, 1, &_pTitaniaTexture);
    world = XMLoadFloat4x4(&_titania);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Oberon
    _pImmediateContext->PSSetShaderResources(0, 1, &_pOberonTexture);
    world = XMLoadFloat4x4(&_oberon);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //Neptune
    _pImmediateContext->PSSetShaderResources(0, 1, &_pNeptuneTexture);
    world = XMLoadFloat4x4(&_neptune);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //set blend state for transparent objects
	//blend state
    //fine tune the blending equation
    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    _pImmediateContext->OMSetBlendState(Transparency, blendFactor, 0xffffffff);

    _pImmediateContext->IASetVertexBuffers(0, 1, &sphereMesh.VertexBuffer, &sphereMesh.VBStride, &sphereMesh.VBOffset);
    _pImmediateContext->IASetIndexBuffer(sphereMesh.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //render the sun as transparent so the ambient light can pass through
    _pImmediateContext->PSSetShaderResources(0, 1, &_pSunTexture);
    _pImmediateContext->IASetVertexBuffers(0, 1, &sphereMesh.VertexBuffer, &sphereMesh.VBStride, &sphereMesh.VBOffset);
    _pImmediateContext->IASetIndexBuffer(sphereMesh.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    world = XMLoadFloat4x4(&_sun);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    blendFactor[0] = 0.75f;
    blendFactor[1] = 0.75f;
    blendFactor[2] = 0.75f;

    //Venus Atmosphere
    _pImmediateContext->PSSetShaderResources(0, 1, &_pVenusAtmos);
    world = XMLoadFloat4x4(&_venusAtmos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(sphereMesh.IndexCount, 0, 0);

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}