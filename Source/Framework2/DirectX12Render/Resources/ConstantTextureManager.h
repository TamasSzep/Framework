// DirectX12Render/Resources/ConstantTextureManager.h

#ifndef _DIRECTX12RENDER_CONSTANTTEXTUREMANAGER_H_INCLUDED_
#define _DIRECTX12RENDER_CONSTANTTEXTUREMANAGER_H_INCLUDED_

#include <DirectX12Render/Resources/Texture2D.h>

#include <Core/DataStructures/Pool.hpp>
#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <EngineBuildingBlocks/Graphics/Primitives/ModelLoader.h>

#include <string>
#include <vector>
#include <map>

namespace EngineBuildingBlocks
{
	class PathHandler;

	namespace Graphics
	{
		struct Image2DDescription;
	}
}

namespace DirectX12Render
{
	class UploadBuffer;
	class TransferBufferManager;
	class IDescriptorHeap;

	struct TextureGettingResult
	{
		Texture2D* Texture;
		bool IsTextureCreated;
	};

	struct CombinedTextureNames
	{
		std::string DiffuseTextureName;
		std::string OpacityTextureName;

		bool operator==(const CombinedTextureNames& other) const;
		bool operator!=(const CombinedTextureNames& other) const;
		bool operator<(const CombinedTextureNames& other) const;
	};

	class ConstantTextureManager
	{
		TransferBufferManager* m_UploadBufferManager;
		EngineBuildingBlocks::PathHandler* m_PathHandler;

		std::map<std::string, Texture2D*> m_Texture2DFromFileMap;
		std::map<CombinedTextureNames, Texture2D*> m_CombinedTextures;

		Core::ResourcePoolU<Texture2D> m_Texture2Ds;

		std::vector<Core::ByteVectorU> m_LoadBuffers;
		Core::SimpleTypeVectorU<unsigned char*> m_SetBufferPointers;

		std::string GetCompletePath(const std::string& fileName);

		TextureGettingResult CreateTextureFromLoadResult(ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			DirectX12Render::IDescriptorHeap* srvDescHeap,
			const EngineBuildingBlocks::Graphics::Image2DDescription& loadDescription,
			TextureMipmapGenerationMode mipmapMode);

	public:

		ConstantTextureManager(TransferBufferManager* uploadBufferManager,
			EngineBuildingBlocks::PathHandler* pathHandler);

		TextureGettingResult GetTexture2DFromMemory(ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			IDescriptorHeap* srvDescHeap,
			const EngineBuildingBlocks::Graphics::ImageRawData& textureRawData,
			TextureMipmapGenerationMode mipmapMode = TextureMipmapGenerationMode::Default);

		TextureGettingResult GetTexture2DFromFile(ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			IDescriptorHeap* srvDescHeap,
			const std::string& fileName,
			TextureMipmapGenerationMode mipmapMode = TextureMipmapGenerationMode::Default);

		// This function creates the texture from a diffuse (xyz->xyz)
		// and an opacity texture (? -> w).
		TextureGettingResult GetDiffuseTexture2DWithOpacity(ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			IDescriptorHeap* srvDescHeap,
			const std::string& diffuseTextureName, const std::string& opacityTextureName,
			unsigned opacityChannelIndex);
	};
}

#endif