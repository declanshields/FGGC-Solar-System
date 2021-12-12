//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	
	float3 Position;
	float Range;
	
	float3 Att;
	float Pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	
	float3 Position;
	float Range;
	
	float3 Direction;
	float Spot;
	
	float3 Att;
	float Pad;
};

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	
	float3 Direction;
	float Pad;
};

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	PointLight gPointLight, gPointLight2;
	SpotLight gSpotLights[6];
	
	matrix World;
	matrix View;
	matrix Projection;

    float4 DiffuseMtrl;
    float4 DiffuseLight;
	float4 AmbientMtrl;
	float4 AmbientLight;
	float4 SpecularMtrl;
	float4 SpecularLight;
	float SpecularPower;
	float3 EyePosW;
    float3 LightVecW;

    float gTime;
}

//--------------------------------------------------------------------------------------
//taken from Frank Luna 3D Game Programming with DirectX 11 pg 296
void ComputeDirectionalLight(DirectionalLight L, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
	//Initialise outputs 
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//The light vector aims opposite the direction the light rays travel
	float3 lightVec = -L.Direction;
	
	//Add ambient term
	ambient = AmbientMtrl * L.Ambient;
	
	//Add diffuse and specular
	float diffuseFactor = dot(lightVec, normal);

	//Flatten to avoid dynamic branching
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), SpecularMtrl.w);

		diffuse = diffuseFactor * DiffuseMtrl * L.Diffuse;
		specular = specFactor * SpecularMtrl * L.Specular;
	}
}
//taken from Frank Luna 3D Game Programming with DirectX 11 pg 297
void ComputePointLight(PointLight L, float3 Pos, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
	//Initialise ouputs
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//The vector from the surface to the light
	float3 lightVec = L.Position - Pos;
	
	//The distance from the surface to the light
	float d = length(lightVec);

	//Range test
	if (d > L.Range)
		return;
	
	//Normalise the light vector
	lightVec /= d;
	
	//Ambient term
	ambient = AmbientMtrl * L.Ambient;
	
	//diffuse and specular
	float diffuseFactor = dot(lightVec, normal);

	//flatten to avoid dynamic branching
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), SpecularMtrl.w);
		
		diffuse = diffuseFactor * DiffuseMtrl * L.Diffuse;
		specular = specFactor * SpecularMtrl * L.Specular;
	}
	
	//Attenuate
	float att = 1.0f / dot(L.Att, float3(0.5f, d, d * d));
	
	diffuse *= att;
	specular *= att;
}

//taken from Frank Luna 3D Game Programming with DirectX 11 pg 298
void ComputeSpotLight(SpotLight L, float3 pos, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
	//intialise outputs
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//the vector from the light to the surface
	float3 lightVec = L.Position - pos;
	
	//Distance from surface to the light
	float d = length(lightVec);

	//range test
	if (d > L.Range)
		return;

	//normalize light vector
	lightVec /= d;
	
	//ambient term
	ambient = AmbientMtrl * L.Ambient;
	
	//diffuse and specular
	float diffuseFactor = dot(lightVec, normal);

	//flatten to avoid dynamic branching
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), SpecularMtrl.w);

		diffuse = diffuseFactor * DiffuseMtrl * L.Diffuse;
		specular = specFactor * SpecularMtrl * L.Specular;
	}
	
	//scale by spotlight factor and attenuate
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	//scale by spotlight factor
	float att = spot / dot(L.Att, float3(1.0f, d, d * d));

	ambient *= spot;
	diffuse *= att;
	specular *= att;
}

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
	float4 Color : COLOR0;
    float3 Norm : NORMAL;
	float3 PosW : POSITION0;
	float2 Tex : TEXCOORD0;
};

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Vertex Shader - Implements Gouraud Shading using diffuse lighting only
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(float4 Pos : POSITION, float3 NormalL : NORMAL, float2 Tex : TEXCOORD0)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	output.Pos = mul(Pos, World);
	output.PosW = output.Pos;		
	float3 toEye = normalize(EyePosW - output.Pos.xyz);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

    //convert from local space to world space
    //W component of vector is 0 as vectors cannot be translated
	float3 normalW = mul(float4(NormalL, 0.0f), World).xyz;
	normalW = normalize(normalW);
    
	output.Norm = normalW;
	
	output.Tex = Tex;

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
	
	input.Norm = normalize(input.Norm);
	
	float3 toEyeW = normalize(EyePosW - input.PosW);
	
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	float4 A, D, S;
	
	//first point light
	ComputePointLight(gPointLight, input.PosW, input.Norm, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;
	
	//Spot Lights
	for (int i = 0; i < 5; i++)
	{
		ComputeSpotLight(gSpotLights[i], input.PosW, input.Norm, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		specular += S;
	}
	
	float4 litColor = ambient + diffuse + specular;
	    
	input.Color = litColor * textureColour;
	
	return input.Color;
}