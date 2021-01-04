// DirectX11Render/Resources/ConstantTextureManager.h

#ifndef _DIRECTX11RENDER_CONSTANTTEXTUREMANAGER_H_INCLUDED_
#define _DIRECTX11RENDER_CONSTANTTEXTUREMANAGER_H_INCLUDED_

#include <DirectX11Render/Resources/Texture2D.h>

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

namespace DirectX11Render
{
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

	enum class TextureMipmapGenerationMode
	{
		Default, Pow2Box
	};

	class ConstantTextureManager
	{
		EngineBuildingBlocks::PathHandler* m_PathHandler;

		std::map<std::string, Texture2D*> m_Texture2DFromFileMap;
		std::map<CombinedTextureNames, Texture2D*> m_CombinedTextures;

		Core::ResourcePoolU<Texture2D> m_Texture2Ds;

		std::vector<Core::ByteVectorU> m_LoadBuffers;
		std::vector<Core::ByteVector> m_AdditionalBuffers;
		Core::SimpleTypeVectorU<D3D11_SUBRESOURCE_DATA> m_SubResourceData;

		std::string GetCompletePath(const std::string& fileName);

		TextureGettingResult CreateTextureFromLoadResult(ID3D11Device* device,
			const EngineBuildingBlocks::Graphics::Image2DDescription& loadDescription,
			TextureMipmapGenerationMode mipmapMode);

		void SetSubResourceData(const Texture2DDescription& texDesc,
			TextureMipmapGenerationMode mipmapMode);

	public:

		ConstantTextureManager(EngineBuildingBlocks::PathHandler* pathHandler);

		TextureGettingResult GetTexture2DFromMemory(ID3D11Device* device,
			const EngineBuildingBlocks::Graphics::ImageRawData& textureRawData,
			TextureMipmapGenerationMode mipmapMode = TextureMipmapGenerationMode::Default);

		TextureGettingResult GetTexture2DFromFile(ID3D11Device* device,
			const std::string& fileName,
			TextureMipmapGenerationMode mipmapMode = TextureMipmapGenerationMode::Default);

		// This function creates the texture from a diffuse (xyz->xyz)
		// and an opacity texture (? -> w).
		TextureGettingResult GetDiffuseTexture2DWithOpacity(ID3D11Device* device,
			const std::string& diffuseTextureName, const std::string& opacityTextureName,
			unsigned opacityChannelIndex);
	};
}

#endif