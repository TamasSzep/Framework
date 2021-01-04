// DirectX11Render/Utilities/ScreenshotHandler.h

#ifndef _DIRECTX11RENDER_SCREENSHOTHANDLER_H_INCLUDED_
#define _DIRECTX11RENDER_SCREENSHOTHANDLER_H_INCLUDED_

#include <Core/System/ThreadPool.h>
#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <DirectX11Render/Resources/Texture2D.h>
#include <DirectX11Render/Utilities/Utility.h>

#include <map>
#include <mutex>

namespace DirectX11Render
{
	class ScreenshotHandler
	{
		EngineBuildingBlocks::PathHandler* m_PathHandler;
		Core::ByteVectorU m_PrintScreenBuffer;

		std::map<Texture2DDescription, Texture2D> m_Textures;

		std::mutex m_Mutex;
		Core::ThreadPool m_ThreadPool;

		Texture2D& GetTexture(ID3D11Device* device, 
			const Texture2DDescription& description);
		
		void SaveCapturedImage(unsigned width, unsigned height, unsigned countChannels);
	
	public:

		ScreenshotHandler();

		void Initialize(const UtilityInitializationContext& context);

		void MakeScreenshot(ID3D11Device* device,
			ID3D11DeviceContext* context, ID3D11Texture2D* texture,
			unsigned width, unsigned height, DXGI_FORMAT format);
	};
}

#endif