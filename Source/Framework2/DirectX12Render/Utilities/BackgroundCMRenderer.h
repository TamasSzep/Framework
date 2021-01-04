// DirectX12Render/Utilities/BackgroundCMRenderer.h

#ifndef _DIRECTX12RENDER_BACKGROUNDCMRENDERER_H_
#define _DIRECTX12RENDER_BACKGROUNDCMRENDERER_H_

#include <DirectX12Render/Utilities/Utility.h>
#include <DirectX12Render/Primitive.h>
#include <DirectX12Render/Resources/ConstantBuffer.h>

namespace DirectX12Render
{
	class BackgroundCMRenderer
	{
		IndexedPrimitive m_BackgroundPrimitive;
		GraphicsPipelineStateObject* m_BackgroundPSO;
		Core::SimpleTypeUnorderedVectorU<Texture2D*> m_BackgroundCubemaps;

	public:

		BackgroundCMRenderer();
		void Initialize(const UtilityInitializationContext& context);	
		unsigned AddTexture(
			const UtilityInitializationContext& context, 
			StreamedDescriptorHeap& CBV_SRV_UAV_DescriptorHeap,
			const char* name);
		void RemoveTexture(unsigned index);
		void ClearTextures();

		void Render(const UtilityRenderContext& context,
			ConstantBuffer* renderPassConstantBuffer,
			unsigned textureIndex, unsigned constantBufferIndex);
	};
}

#endif