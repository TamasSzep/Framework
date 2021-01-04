// OpenGLRender/Resources/ConstantTextureManager.h

#ifndef _OPENGLRENDER_CONSTANTTEXTUREMANAGER_H_INCLUDED_
#define _OPENGLRENDER_CONSTANTTEXTUREMANAGER_H_INCLUDED_

#include <OpenGLRender/Resources/Texture2D.h>

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

namespace OpenGLRender
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

	struct SubResourceData
	{
		const void* Data;
		unsigned    RowPitch;
		unsigned    SlicePitch;
	};

	class ConstantTextureManager
	{
		EngineBuildingBlocks::PathHandler* m_PathHandler;

		std::map<std::string, Texture2D*> m_Texture2DFromFileMap;
		std::map<CombinedTextureNames, Texture2D*> m_CombinedTextures;

		Core::ResourcePoolU<Texture2D> m_Texture2Ds;

		std::vector<Core::ByteVectorU> m_LoadBuffers;
		Core::ByteVector m_AdditionalBuffer;

		std::string GetCompletePath(const std::string& fileName);

		TextureGettingResult CreateTextureFromLoadResult(
			const EngineBuildingBlocks::Graphics::Image2DDescription& loadDescription,
			TextureMipmapGenerationMode mipmapMode,
			const TextureSamplingDescription* pSamlingDesc);

		void SetTextureData(Texture2D* texture,
			const EngineBuildingBlocks::Graphics::Image2DDescription& loadDescription,
			const Texture2DDescription& texDesc,
			TextureMipmapGenerationMode mipmapMode);

	public:

		ConstantTextureManager(EngineBuildingBlocks::PathHandler* pathHandler);

		TextureGettingResult GetTexture2DFromMemory(
			const EngineBuildingBlocks::Graphics::ImageRawData& textureRawData,
			TextureMipmapGenerationMode mipmapMode = TextureMipmapGenerationMode::Default,
			const TextureSamplingDescription* pSamlingDesc = nullptr);

		TextureGettingResult GetTexture2DFromFile(
			const std::string& fileName,
			TextureMipmapGenerationMode mipmapMode = TextureMipmapGenerationMode::Default,
			const TextureSamplingDescription* pSamlingDesc = nullptr);

		// This function creates the texture from a diffuse (xyz->xyz)
		// and an opacity texture (? -> w).
		TextureGettingResult GetDiffuseTexture2DWithOpacity(
			const std::string& diffuseTextureName, const std::string& opacityTextureName,
			unsigned opacityChannelIndex);
	};
}

#endif