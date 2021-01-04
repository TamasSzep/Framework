// DirectX12Render/Managers.h

#ifndef _DIRECTX12RENDER_MANAGER_H_INCLUDED
#define _DIRECTX12RENDER_MANAGER_H_INCLUDED

#include <DirectX12Render/DirectX12Includes.h>
#include <DirectX12Render/Resources/Shader.h>
#include <DirectX12Render/Resources/RootSignature.h>
#include <DirectX12Render/Resources/PipelineStateObject.h>
#include <DirectX12Render/Resources/TransferBuffer.h>
#include <DirectX12Render/Resources/ConstantTextureManager.h>
#include <DirectX12Render/Primitive.h>
#include <DirectX12Render/Resources/DescriptorHeap.h>

namespace DirectX12Render
{
	class Managers
	{
	public: // Resource handling.

		ShaderManager ShaderManager;
		RootSignatureManager RootSignatureManager;
		PipelineStateObjectManager PipelineStateObjectManager;
		
		TransferBufferManager TransferBufferManager;
		ConstantTextureManager ConstantTextureManager;
		PrimitiveManager PrimitiveManager;

	public:

		Managers(EngineBuildingBlocks::PathHandler& pathHandler);
	};
}

#endif