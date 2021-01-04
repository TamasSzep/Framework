// DirectX11Render/Managers.cpp

#include <DirectX11Render/Managers.h>

using namespace DirectX11Render;

Managers::Managers(EngineBuildingBlocks::PathHandler& pathHandler)
	: ShaderManager(&pathHandler)
	, ConstantTextureManager(&pathHandler)
{
}