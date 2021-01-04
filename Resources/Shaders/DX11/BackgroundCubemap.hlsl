// DX11/BackgroundCubemap.hlsl

#include "../HLSL/Structures.hlsl"
#include "RenderPass.hlsl"

///// Visible to pixel shader.

TextureCube Background : register(t0);
SamplerState BackgroundSampler : register(s0);

static const float LastZ = 0.99999f;

VertexPos4Direction VSMain(VertexPos3TextCoordNormal input)
{
	VertexPos4Direction output = (VertexPos4Direction)0;

	output.Position = float4(input.Position.xy, LastZ, 1.0f);
	float4 worldPosition4 = mul(ViewProjectionMatrixInverse, float4(input.Position, 1.0f));
	output.Direction = worldPosition4.xyz / worldPosition4.w - CameraPosition;

	return output;
}

float4 PSMain(VertexPos4Direction input) : SV_TARGET
{
	return float4(Background.Sample(BackgroundSampler, input.Direction).xyz, 1.0f);
}