// OpenGLRender/Utilities/Utilities.cpp

#include <OpenGLRender/Utilities/Utilities.h>

#include <OpenGLRender/Managers.h>

using namespace OpenGLRender;

void Utilites::Initialize(const UtilityInitializationContext& context)
{
	BackgroundCMRenderer.Initialize(context);
	ScreenshotHandler.Initialize(context);
	TextRenderer.Initialize(context);
}

void Utilites::PreUpdate(const UtilityUpdateContext& context)
{
	TextRenderer.Update(context);
}

void Utilites::PostUpdate(const UtilityUpdateContext& context)
{
}