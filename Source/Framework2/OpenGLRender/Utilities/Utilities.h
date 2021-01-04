// OpenGLRender/Utilities/Utilities.h

#ifndef _OPENGLRENDER_UTILITIES_H_INCLUDED
#define _OPENGLRENDER_UTILITIES_H_INCLUDED

#include <OpenGLRender/Utilities/Utility.h>
#include <OpenGLRender/Utilities/BackgroundCMRenderer.h>
#include <OpenGLRender/Utilities/ScreenshotHandler.h>
#include <OpenGLRender/Utilities/TextRenderer.h>

namespace OpenGLRender
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