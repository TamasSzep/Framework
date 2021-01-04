// DirectX12Render/Resources/Texture2D.h

#ifndef _DIRECTX12RENDER_TEXTURE2D_H_INCLUDED_
#define _DIRECTX12RENDER_TEXTURE2D_H_INCLUDED_

#include <DirectX12Render/Resources/Texture2DBase.h>

#include <Core/DataStructures/SimpleTypeVector.hpp>

namespace DirectX12Render
{
	enum class TextureMipmapGenerationMode
	{
		Default, Pow2Box
	};

	class Texture2D : public Texture2DBase
	{
	public:

		void Initialize(ID3D12Device* device,
			const Texture2DDescription& description,
			D3D12_RESOURCE_STATES startState,
			IDescriptorHeap* srvDescHeap = nullptr);

		void SetData(ID3D12GraphicsCommandList* commandList,
			UploadBuffer* uploadBuffer, Core::SimpleTypeVectorU<unsigned char*> data,
			TextureMipmapGenerationMode mipmapMode = TextureMipmapGenerationMode::Default);
	};
}

#endif