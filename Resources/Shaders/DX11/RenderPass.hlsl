// DX11/RenderPass.hlsl

#ifndef _RENDERPASS_HLSL_
#define _RENDERPASS_HLSL_

cbuffer RenderPassCBType : register(b0)
{
	float3 CameraPosition;
	float Padding0;
	float3 CameraDirection;
	float Padding1;
	float3 CameraUp;
	float Padding2;
	float3 CameraRight;
	float Padding3;
	float4x4 ViewMatrix;
	float4x4 ViewMatrixInvserse;
	float4x4 ProjectionMatrix;
	float4x4 ProjectionMatrixInverse;
	float4x4 ViewProjectionMatrix;
	float4x4 ViewProjectionMatrixInverse;
};

#endif