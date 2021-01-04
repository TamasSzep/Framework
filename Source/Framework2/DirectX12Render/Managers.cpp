// DirectX12Render/Managers.cpp

#include <DirectX12Render/Managers.h>

using namespace DirectX12Render;

Managers::Managers(EngineBuildingBlocks::PathHandler& pathHandler)
	: ShaderManager(&pathHandler)
	, ConstantTextureManager(&TransferBufferManager, &pathHandler)
{
}