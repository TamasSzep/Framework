// OpenGLRender/Utilities/RenderPassCB.cpp

#include <OpenGLRender/Utilities/RenderPassCB.h>

namespace OpenGLRender
{
	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera* pCamera, RenderPassCBType* pUniformBufferData)
	{
		UpdateRenderPassCB(&pCamera, pUniformBufferData, 0, 1);
	}

	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera** pCameras,
		RenderPassCBType* pUniformBufferData, unsigned startBufferIndex, unsigned endBufferIndex)
	{
		unsigned count = endBufferIndex - startBufferIndex;
		for (unsigned i = 0; i < count; i++)
		{
			auto& camera = *pCameras[i];
			auto& renderPassCBData = pUniformBufferData[startBufferIndex + i];
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