// DirectX11Render/Utilities/BackgroundCMRenderer.h

#ifndef _DIRECTX11RENDER_BACKGROUNDCMRENDERER_H_
#define _DIRECTX11RENDER_BACKGROUNDCMRENDERER_H_

#include <DirectX11Render/Primitive.h>
#include <DirectX11Render/Resources/PipelineState.h>
#include <DirectX11Render/Resources/ResourceState.h>
#include <DirectX11Render/Resources/ConstantBuffer.h>
#include <DirectX11Render/Utilities/Utility.h>

namespace DirectX11Render
{
	class BackgroundCMRenderer
	{
		IndexedPrimitive m_Primitive;
		unsigned m_PipelineStateIndex;
		Core::SimpleTypeUnorderedVectorU<Texture2D*> m_BackgroundCubemaps;

	public:

		BackgroundCMRenderer();
		void Initialize(const UtilityInitializationContext& context);	
		unsigned AddTexture(
			const UtilityInitializationContext& context, const char* name);
		void RemoveTexture(unsigned index);
		void ClearTextures();

		void Render(const UtilityRenderContext& context,
			ConstantBuffer& renderPassConstantBuffer,
			unsigned textureIndex, unsigned constantBufferIndex);
	};
}

#endif