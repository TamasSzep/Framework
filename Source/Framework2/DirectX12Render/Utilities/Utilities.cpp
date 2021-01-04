// DirectX12Render/Utilities/Utilities.cpp

#include <DirectX12Render/Utilities/Utilities.h>

#include <DirectX12Render/Managers.h>

using namespace DirectX12Render;

void Utilites::Initialize(const UtilityInitializationContext& context,
	StreamedDescriptorHeap& CBV_SRV_UAV_DescriptorHeap)
{
	TextRenderer.Initialize(context, CBV_SRV_UAV_DescriptorHeap);
	MSAAResolver.Initialize(context);
	BackgroundCMRenderer.Initialize(context);
}