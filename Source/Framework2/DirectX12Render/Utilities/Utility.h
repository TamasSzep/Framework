// DirectX12Render/Utilities/Utility.h

#ifndef _DIRECTX12RENDER_UTILITY_H_
#define _DIRECTX12RENDER_UTILITY_H_

#include <EngineBuildingBlocks/PathHandler.h>
#include <DirectX12Render/Managers.h>

namespace DirectX12Render
{
	struct UtilityInitializationContext
	{
		ID3D12Device* Device;
		ID3D12GraphicsCommandList* CommandList;

		EngineBuildingBlocks::PathHandler* PathHandler;
		Managers* DX12M;

		unsigned FrameCount;
		unsigned MultisampleCount;
	};

	struct UtilityRenderContext
	{
		ID3D12GraphicsCommandList* CommandList;

		Managers* DX12M;
			
		unsigned FrameIndex;
	};

	struct UtilityUpdateContext
	{
		unsigned FrameIndex;
	};
}

#endif