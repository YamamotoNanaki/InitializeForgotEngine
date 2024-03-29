cbuffer cbuff0:register(b0)
{
    matrix viewPro;
    matrix world;
    float3 cameraPos;
};

cbuffer ConstBufferDataTransform : register(b1)
{
    float explosion;
    float polygonSize;
    float gravity;
    float rotation;
    bool lightFlag;
    bool toonFlag;
};

cbuffer Material : register(b2)
{
    float4 color : packoffset(c0); //�F(RGBA)
    float3 ambient : packoffset(c1);
    float3 diffuse : packoffset(c2);
    float3 specular : packoffset(c3);
    float alpha : packoffset(c3.w);
};

static const int LIGHT_MAX = 100;
static const int DLIGHT_NUM = LIGHT_MAX;

struct DLight
{
	float3 lightv;
	float3 lightcolor;
	uint active;
};

static const int PLIGHT_NUM = LIGHT_MAX;

struct PLight
{
	float3 lightpos;
	float3 lightcolor;
	float3 lightatten;
	uint active;
};

static const int SLIGHT_NUM = LIGHT_MAX;

struct SLight
{
	float3 lightv;
	float3 lightpos;
	float3 lightcolor;
	float3 lightatten;
	float2 lightFactorAngleCos;
	uint active;
};

static const int CSHADOW_NUM = LIGHT_MAX;

struct CShadow
{
	float3 shadowv;
	float3 casterpos;
	float distanceCasterLight;
	float3 shadowatten;
	float2 shadowFactorAngleCos;
	uint active;
};

cbuffer LightBuff : register(b3)
{
    float3 ambientColor;
    DLight dLights[DLIGHT_NUM];
    PLight pLights[PLIGHT_NUM];
    SLight sLights[SLIGHT_NUM];
    CShadow cShadows[CSHADOW_NUM];
}

struct VSOutput
{
	float4 svpos : SV_POSITION;
	float4 worldpos : POSITION;
	float3 normal :NORMAL;
	float2 uv :TEXCOORD;
};

struct GSOutput
{
	float4 svpos : SV_POSITION;
	float4 worldpos : POSITION;
	float3 normal :NORMAL;
	float2 uv :TEXCOORD;
};