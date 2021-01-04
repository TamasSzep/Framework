// OpenGLRender/Utilities/RenderPassCB.h

#ifndef _OPENGLRENDER_RENDERPASSCB_H_
#define _OPENGLRENDER_RENDERPASSCB_H_

#include <EngineBuildingBlocks/Graphics/Camera/Camera.h>

namespace OpenGLRender
{
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

	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera* pCamera,
		RenderPassCBType* pUniformBufferData);
	void UpdateRenderPassCB(EngineBuildingBlocks::Graphics::Camera** pCameras,
		RenderPassCBType* pUniformBufferData,
		unsigned startBufferIndex, unsigned endBufferIndex);
}

#endif