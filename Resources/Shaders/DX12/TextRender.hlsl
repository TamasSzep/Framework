// TextRender.hlsl

#include "../HLSL/Structures.hlsl"

struct SymbolData
{
	// Symbol data.
	float2 PositionMultiplier : TEXCOORD1;
	float2 PositionOffset     : TEXCOORD2;

	// Glyph data.
	float2 PositionInTex      : TEXCOORD3;
	float2 SizeInTex          : TEXCOORD4;
};

///// Visible to vertex shader.

float Depth : register(b0);

///// Visible to pixel shader.

Texture2D DistanceTexture : register(t0);
SamplerState DistanceTextureSampler : register(s0);

static const float Smoothing = 1.0f / 16.0f;

struct ConstantsType
{
	float4 Color;
};

ConstantBuffer<ConstantsType> PSConstants : register(b0);

VertexPos4TextCoord SDF_VS_Main(VertexPos3TextCoordNormal vertex, SymbolData instance)
{
	VertexPos4TextCoord output = (VertexPos4TextCoord)0;

	float2 multiplier = instance.PositionMultiplier;
	float2 offset = instance.PositionOffset;

	output.Position = float4(vertex.Position.xy * multiplier + offset, Depth, 1.0f);
	output.TexCoord = vertex.TexCoord * instance.SizeInTex + instance.PositionInTex;

	return output;
}

float4 SDF_PS_Main(VertexPos4TextCoord input) : SV_TARGET
{
	float distance = DistanceTexture.Sample(DistanceTextureSampler, input.TexCoord).x;
	float alpha = smoothstep(0.5f - Smoothing, 0.5f + Smoothing, distance) * PSConstants.Color.a;
	return float4(PSConstants.Color.xyz, alpha);
}