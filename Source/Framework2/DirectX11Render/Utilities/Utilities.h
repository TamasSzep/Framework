// DirectX11Render/Utilities/Utilities.h

#ifndef _DIRECTX11RENDER_UTILITIES_H_INCLUDED
#define _DIRECTX11RENDER_UTILITIES_H_INCLUDED

#include <DirectX11Render/Utilities/Utility.h>
#include <DirectX11Render/Utilities/BackgroundCMRenderer.h>
#include <DirectX11Render/Utilities/ScreenshotHandler.h>
#include <DirectX11Render/Utilities/TextRenderer.h>

namespace DirectX11Render
{
	class Utilites
	{
	public:

		BackgroundCMRenderer BackgroundCMRenderer;
		ScreenshotHandler ScreenshotHandler;
		TextRenderer TextRenderer;

		void Initialize(const UtilityInitializationContext& context);
		void PreUpdate(const UtilityUpdateContext& context);
		void PostUpdate(const UtilityUpdateContext& context);
	};
}

#endif