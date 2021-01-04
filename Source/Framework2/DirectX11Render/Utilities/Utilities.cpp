// DirectX11Render/Utilities/Utilities.cpp

#include <DirectX11Render/Utilities/Utilities.h>

#include <DirectX11Render/Managers.h>

using namespace DirectX11Render;

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