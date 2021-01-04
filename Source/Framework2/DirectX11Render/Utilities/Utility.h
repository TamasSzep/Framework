// DirectX11Render/Utilities/Utility.h

#ifndef _DIRECTX11RENDER_UTILITY_H_
#define _DIRECTX11RENDER_UTILITY_H_

#include <EngineBuildingBlocks/PathHandler.h>
#include <DirectX11Render/Managers.h>

namespace DirectX11Render
{
	struct UtilityInitializationContext
	{
		ID3D11Device* Device;
		ID3D11DeviceContext* Context;

		EngineBuildingBlocks::PathHandler* PathHandler;
		Managers* DX11M;

		unsigned MultisampleCount;
	};

	struct UtilityRenderContext
	{
		ID3D11DeviceContext* Context;

		Managers* DX11M;
	};

	struct UtilityUpdateContext
	{
		ID3D11DeviceContext* Context;
	};
}

#endif