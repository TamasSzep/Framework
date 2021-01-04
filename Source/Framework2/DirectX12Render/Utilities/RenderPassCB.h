// DirectX12Render/Utilities/RenderPassCB.h

#ifndef _DIRECTX12RENDER_RENDERPASSCB_H_
#define _DIRECTX12RENDER_RENDERPASSCB_H_

#include <EngineBuildingBlocks/Graphics/Camera/Camera.h>
#include <DirectX12Render/Resources/ConstantBuffer.h>

namespace DirectX12Render
{
	const unsigned c_DefaultMaxCountRenderPasses = 32;

	struct RenderPassCBType
	{
		glm::vec3 CameraPosition;
		float Padding0;
		glm::vec3 CameraDirection;
		float Padding1;
		glm::vec3 CameraUp;
		float Padding2;
		glm::vec3 CameraRight;
		float Padding3;
		glm::mat4 ViewMatrix;
		glm::mat4 ViewMatrixInvserse;
		glm::mat4 ProjectionMatrix;
		glm::mat4 ProjectionMatrixInverse;
		glm::mat4 ViewProjectionMatrix;
		glm::mat4 ViewProjectionMatrixInverse;
	};

	ConstantBuffer CreateRenderPassCB(unsigned maxCountRenderPasses
		= c_DefaultMaxCountRenderPasses);

	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera* pCamera,
		ConstantBuffer* pConstantBuffer);
	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera** pCameras,
		ConstantBuffer* pConstantBuffer,
		unsigned startBufferIndex, unsigned endBufferIndex);
}

#endif