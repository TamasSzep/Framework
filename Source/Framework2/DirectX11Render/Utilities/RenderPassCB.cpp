// DirectX11Render/Utilities/RenderPassCB.cpp

#include <DirectX11Render/Utilities/RenderPassCB.h>

namespace DirectX11Render
{
	ConstantBuffer CreateRenderPassCB(ID3D11Device* device, unsigned maxCountRenderPasses)
	{
		ConstantBuffer constantBuffer;
		constantBuffer.Initialize(device, sizeof(RenderPassCBType), maxCountRenderPasses);
		return constantBuffer;
	}

	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera* pCamera, ConstantBuffer* pConstantBuffer)
	{
		UpdateRenderPassCB(&pCamera, pConstantBuffer, 0, 1);
	}

	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera** pCameras,
		ConstantBuffer* pConstantBuffer, unsigned startBufferIndex, unsigned endBufferIndex)
	{
		unsigned count = endBufferIndex - startBufferIndex;
		for (unsigned i = 0; i < count; i++)
		{
			auto& camera = *pCameras[i];
			auto& renderPassCBData = pConstantBuffer->Access<RenderPassCBType>(startBufferIndex + i);
			renderPassCBData.CameraPosition = camera.GetPosition();
			renderPassCBData.CameraDirection = camera.GetDirection();
			renderPassCBData.CameraUp = camera.GetUp();
			renderPassCBData.CameraRight = camera.GetRight();
			renderPassCBData.ViewMatrix = camera.GetViewMatrix();
			renderPassCBData.ViewMatrixInvserse = camera.GetViewMatrixInverse();
			renderPassCBData.ProjectionMatrix = camera.GetProjectionMatrix();
			renderPassCBData.ProjectionMatrixInverse = camera.GetProjectionMatrixInverse();
			renderPassCBData.ViewProjectionMatrix = camera.GetViewProjectionMatrix();
			renderPassCBData.ViewProjectionMatrixInverse = camera.GetViewProjectionMatrixInverse();
		}
	}
}