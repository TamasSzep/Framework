// Test/ShaderTest.hlsl

#include "../../HLSL/Structures.hlsl"
#include "../RenderPass.hlsl"
#include "../Lighting.hlsl"

////////////////////////////////////// STRUCTURES /////////////////////////////////////

struct MaterialCBType
{
	float3 DiffuseColor;
	float _Padding0;
	float3 SpecularColor;
	uint DiffuseTextureIndex;
};

struct ObjectCBType
{
	float4x4 ModelMatrix;
	float4x4 InverseMatrix;
	float4x4 MVPMatrix;
};

////////////////////////////////////// DATA /////////////////////////////////////

///// Visible to all shaders.

///// Visible to vertex shader.

ConstantBuffer<ObjectCBType> Object : register(b0);

///// Visible to pixel shader.

ConstantBuffer<RenderPassCBType> RenderPass : register(b1);
ConstantBuffer<MaterialCBType> Material : register(b2);
ConstantBuffer<LightingCBType> Lighting : register(b3);

// Don't forget to use NonUniformResourceIndex(index) if the execution is divergent in an (even instanced) draw call.
Texture2D AllTextureArray[] : register(t0);
SamplerState MySampler : register(s0);

uint MyMaterialIndex : register(b0);

static const float3 SpecularColor = float3(1.0f, 1.0f, 1.0f);

///////////////////////////////////// SHADERS /////////////////////////////////////


#if (IS_USING_VERTEX_COLOR == 1)
VertexPos4TextCoordNormalWPosColor VSMain(VertexPos3TextCoordNormalColor input)
#else
VertexPos4TextCoordNormalWPos VSMain(VertexPos3TextCoordNormal input)
#endif
{
#if (IS_USING_VERTEX_COLOR == 1)
	VertexPos4TextCoordNormalWPosColor result;
#else
	VertexPos4TextCoordNormalWPos result;
#endif

	float4 position1 = float4(input.Position, 1.0f);
	float4 normal0 = float4(input.Normal, 0.0f);

	result.Position = mul(Object.MVPMatrix, position1);
	result.TexCoord = input.TexCoord;
	result.Normal = normalize(mul(normal0, Object.InverseMatrix).xyz);
	result.WorldPosition = mul(Object.ModelMatrix, position1).xyz;

#if (IS_USING_VERTEX_COLOR == 1)
	result.VertexColor = input.VertexColor;
#endif

	return result;
}

#if (IS_USING_VERTEX_COLOR == 1)
float4 PSMain(VertexPos4TextCoordNormalWPosColor input) : SV_TARGET
#else
float4 PSMain(VertexPos4TextCoordNormalWPos input) : SV_TARGET
#endif
{
	float3 normal = normalize(input.Normal);

	uint diffuseTextureIndex = Material.DiffuseTextureIndex;
	float4 diffuseColor4 = AllTextureArray[diffuseTextureIndex].Sample(MySampler, input.TexCoord);
	float3 diffuseColor = diffuseColor4.xyz;

	float3 specularColor;
#if (IS_USING_MATERIAL_COLORS == 1)
	diffuseColor *= Material.DiffuseColor;
	specularColor = Material.SpecularColor;
#else
	specularColor = SpecularColor;
#endif

#if (IS_USING_VERTEX_COLOR == 1)
	diffuseColor *= input.VertexColor;
	specularColor *= input.VertexColor;
#endif

	float opacity = diffuseColor4.w;

	float3 color = EvaluatePhong(Lighting, normal, input.WorldPosition, RenderPass.CameraPosition,
		diffuseColor, specularColor);

	return float4(color, opacity);
}