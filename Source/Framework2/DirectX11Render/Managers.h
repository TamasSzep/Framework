// DirectX11Render/Managers.h

#ifndef _DIRECTX11RENDER_MANAGERS_H_INCLUDED_
#define _DIRECTX11RENDER_MANAGERS_H_INCLUDED_

#include <EngineBuildingBlocks/PathHandler.h>
#include <DirectX11Render/Primitive.h>
#include <DirectX11Render/Resources/Shader.h>
#include <DirectX11Render/Resources/PipelineState.h>
#include <DirectX11Render/Resources/ResourceState.h>
#include <DirectX11Render/Resources/ConstantTextureManager.h>

namespace DirectX11Render
{
	class Managers
	{
	public: // Resource handling.

		ShaderManager ShaderManager;
		PipelineStateManager PipelineStateManager;
		ResourceStateManager ResourceStateManager;
		ConstantTextureManager ConstantTextureManager;
		PrimitiveManager PrimitiveManager;

	public:

		Managers(EngineBuildingBlocks::PathHandler& pathHandler);
	};
}

#endif