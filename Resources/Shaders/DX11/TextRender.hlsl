// DX11/TextRender.hlsl

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

///// Visible to all shader.

cbuffer ConstantsType : register(b0)
{
	float4 Color;
	float Depth;
};

///// Visible to pixel shader.

Texture2D DistanceTexture : register(t0);
SamplerState DistanceTextureSampler : register(s0);

static const float Smoothing = 1.0f / 16.0f;

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
	float alpha = smoothstep(0.5f - Smoothing, 0.5f + Smoothing, distance) * Color.a;
	return float4(Color.xyz, alpha);
}