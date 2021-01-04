// DirectX12Render/Utilities/RenderPassCB.cpp

#include <DirectX12Render/Utilities/RenderPassCB.h>

namespace DirectX12Render
{
	ConstantBuffer CreateRenderPassCB(unsigned maxCountRenderPasses)
	{
		return ConstantBuffer(DirectX12Render::ConstantBufferMappingType::AlwaysMapped,
			sizeof(RenderPassCBType), maxCountRenderPasses);
	}

	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera* pCamera,
		ConstantBuffer* pConstantBuffer)
	{
		UpdateRenderPassCB(&pCamera, pConstantBuffer, 0, 1);
	}

	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera** pCameras,
		ConstantBuffer* pConstantBuffer, unsigned startBufferIndex, unsigned endBufferIndex)
	{
		pConstantBuffer->StartWrite();

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

		pConstantBuffer->IndicateWrittenRangeWithViewIndex(startBufferIndex, endBufferIndex);
		pConstantBuffer->EndWrite();
	}
}