// DirectX12Render/Utilities/Utilities.h

#ifndef _DIRECTX12RENDER_UTILITIES_H_INCLUDED
#define _DIRECTX12RENDER_UTILITIES_H_INCLUDED

#include <DirectX12Render/Utilities/Utility.h>
#include <DirectX12Render/Utilities/TextRenderer.h>
#include <DirectX12Render/Utilities/MSAAResolver.h>
#include <DirectX12Render/Utilities/BackgroundCMRenderer.h>

namespace DirectX12Render
{
	class Utilites
	{	
	public:

		TextRenderer TextRenderer;
		MSAAResolver MSAAResolver;
		BackgroundCMRenderer BackgroundCMRenderer;

		void Initialize(
			const UtilityInitializationContext& context,
			StreamedDescriptorHeap& CBV_SRV_UAV_DescriptorHeap);
	};
}

#endif