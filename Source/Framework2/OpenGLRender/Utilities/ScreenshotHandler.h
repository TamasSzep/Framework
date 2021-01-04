// OpenGLRender/Utilities/ScreenshotHandler.h

#ifndef _OPENGLRENDER_SCREENSHOTHANDLER_H_INCLUDED_
#define _OPENGLRENDER_SCREENSHOTHANDLER_H_INCLUDED_

#include <Core/System/ThreadPool.h>
#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <OpenGLRender/Resources/Texture2D.h>
#include <OpenGLRender/Resources/FrameBufferObject.h>
#include <OpenGLRender/Utilities/Utility.h>

#include <map>
#include <mutex>

namespace OpenGLRender
{
	class ScreenshotHandler
	{
		EngineBuildingBlocks::PathHandler* m_PathHandler;
		Core::ByteVectorU m_PrintScreenBuffer;

		Core::ResourcePoolU<Texture2D> m_Textures;
		Core::ResourcePoolU<FrameBufferObject> m_FBOs;

		struct ResourceData
		{
			Texture2D* Texture;
			FrameBufferObject* FBO;
		};

		std::map<Texture2DDescription, ResourceData> m_FBOMap;

		Core::ThreadPool m_ThreadPool;

		ResourceData GetCopyResource(const Texture2DDescription& texDesc);
		
		void SaveCapturedImage(unsigned width, unsigned height, unsigned countChannels);
	
	public:

		ScreenshotHandler();

		void Initialize(const UtilityInitializationContext& context);

		void MakeScreenshot(const Texture2D* texture,
			unsigned width, unsigned height, PixelDataFormat pixelDataFormat);

		void MakeScreenshotFromFBO(unsigned fbo, unsigned readBuffer,
			unsigned width, unsigned height, PixelDataFormat pixelDataFormat);
	};
}

#endif