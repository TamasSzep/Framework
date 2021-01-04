// OpenGLRender/Resources/ConstantTextureManager.cpp

#include <OpenGLRender/Resources/ConstantTextureManager.h>

#include <Core/Comparison.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/Graphics/Resources/ImageHelper.h>

#include <External/Encoding/Base64Encoding.h>

// Implementation is already compiled to EngineBuildingBlocks.
#include <stb/stb_image_resize.h>

#include <filesystem>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace OpenGLRender;

bool CombinedTextureNames::operator==(const CombinedTextureNames& other) const
{
	StringEqualCompareBlock(DiffuseTextureName);
	StringEqualCompareBlock(OpacityTextureName);
	return true;
}

bool CombinedTextureNames::operator!=(const CombinedTextureNames& other) const
{
	return !(*this == other);
}

bool CombinedTextureNames::operator<(const CombinedTextureNames& other) const
{
	StringLessCompareBlock(DiffuseTextureName);
	StringLessCompareBlock(OpacityTextureName);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ConstantTextureManager::ConstantTextureManager(PathHandler* pathHandler)
	: m_PathHandler(pathHandler)
{
}

class FormatSupportHandler : public EngineBuildingBlocks::Graphics::FormatSupportHandler
{
public:

	void HandleFormatSupport(Image2DDescription& description) const
	{
		if (description.ChannelCount == 3 && description.PixelByteCount < 4)
		{
			assert(description.PixelByteCount % 3 == 0);

			description.ChannelCount = 4;
			description.PixelByteCount = description.PixelByteCount / 3 * 4;
		}
	}
};

std::string ConstantTextureManager::GetCompletePath(const std::string& fileName)
{
	if (std::filesystem::path(fileName).is_absolute())
	{
		return fileName;
	}
	return m_PathHandler->GetPathFromResourcesDirectory("Textures/" + fileName);
}

PixelDataFormat GetPixelDataFormat(const Image2DDescription& loadDescription)
{
	switch (loadDescription.ChannelCount)
	{
	case 1: return PixelDataFormat::Red;
	case 2: return PixelDataFormat::RG;
	case 3: return PixelDataFormat::RGB;
	case 4: return PixelDataFormat::RGBA;
	}
	assert(false);
	return PixelDataFormat::RGBA;
}

PixelDataType GetPixelDataType(const Image2DDescription& loadDescription)
{
	switch (loadDescription.Type)
	{
	case ImagePixelType::UnsignedByte: return PixelDataType::Uint8;
	case ImagePixelType::Float: return PixelDataType::Float;
	}
	assert(false);
	return PixelDataType::Uint8;
}

void ConstantTextureManager::SetTextureData(Texture2D* texture,
	const Image2DDescription& loadDescription,
	const OpenGLRender::Texture2DDescription& texDesc,
	TextureMipmapGenerationMode mipmapMode)
{
	auto width = loadDescription.Width;
	auto height = loadDescription.Height;
	auto arraySize = loadDescription.ArraySize;
	auto pixelByteCount = loadDescription.PixelByteCount;
	auto channelCount = loadDescription.ChannelCount;
	auto countMipmapLevels = texDesc.GetCountMipmapLevels();
	auto pixelType = texDesc.GetPixelType();
	auto pixelDataFormat = GetPixelDataFormat(loadDescription);
	auto pixelDataType = GetPixelDataType(loadDescription);

	stbir_datatype dataType;
	if (pixelType == PixelType::Uint || pixelType == PixelType::Unorm)
	{
		switch (pixelByteCount / channelCount)
		{
		case 1: dataType = STBIR_TYPE_UINT8; break;
		case 2: dataType = STBIR_TYPE_UINT16; break;
		case 4: dataType = STBIR_TYPE_UINT32; break;
		}
	}
	else if (pixelType == PixelType::Float) dataType = STBIR_TYPE_FLOAT;
	else if (countMipmapLevels > 1) RaiseException("Pixel type is not handled when generating mipmaps.");

	int hasAlphaChannel = (pixelType == PixelType::Unorm || pixelType == PixelType::UnormSRGB);
	int alphaChannelIndex = (hasAlphaChannel == 0 ? 0 : 3);

	stbir_colorspace colorSpace = (pixelType == PixelType::Unorm
		? STBIR_COLORSPACE_SRGB
		: STBIR_COLORSPACE_LINEAR);

	m_AdditionalBuffer.Resize(m_LoadBuffers[0].GetSize());

	texture->Bind();

	for (unsigned arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
	{
		auto sourceBuffer = m_LoadBuffers[arrayIndex].GetArray();
		auto targetBuffer = m_AdditionalBuffer.GetArray();

		unsigned sourceWidth = width;
		unsigned sourceHeight = height;

		for (unsigned mipmapIndex = 0; mipmapIndex < countMipmapLevels; mipmapIndex++)
		{
			if (mipmapIndex > 0)
			{
				unsigned targetWidth = std::max(sourceWidth >> 1, 1U);
				unsigned targetHeight = std::max(sourceHeight >> 1, 1U);

				if (mipmapMode == TextureMipmapGenerationMode::Pow2Box)
				{
					assert(pixelType == PixelType::Unorm);
					assert(pixelByteCount == channelCount);
					DownsampleLinearUnormWithBoxFilterPow2(
						reinterpret_cast<const unsigned char*>(sourceBuffer), sourceWidth, sourceHeight,
						targetBuffer, channelCount);
				}
				else
				{
					stbir_resize(
						sourceBuffer, sourceWidth, sourceHeight, 0,
						targetBuffer, targetWidth, targetHeight, 0,
						dataType, channelCount,
						alphaChannelIndex, 0, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
						STBIR_FILTER_BOX, STBIR_FILTER_BOX, colorSpace,
						nullptr);
				}

				sourceWidth = targetWidth;
				sourceHeight = targetHeight;
				std::swap(sourceBuffer, targetBuffer);
			}

			// Setting texture data.
			glm::uvec3 tOffset(0, 0, arrayIndex);
			glm::uvec3 tSize(sourceWidth, sourceHeight, 1);
			texture->SetData(sourceBuffer, pixelDataFormat, pixelDataType,
				glm::value_ptr(tOffset), glm::value_ptr(tSize), mipmapIndex);
		}
	}

	texture->Unbind();
}

TextureGettingResult ConstantTextureManager::CreateTextureFromLoadResult(
	const Image2DDescription& loadDescription,
	TextureMipmapGenerationMode mipmapMode,
	const TextureSamplingDescription* pSamlingDesc)
{
	// Creating texture description. Immutable, mipmapped, non-multisampled texture.

	assert(loadDescription.Type == ImagePixelType::UnsignedByte && loadDescription.ChannelCount == 4);
	auto format = TextureFormat::RGBA8;

	Texture2DDescription description(loadDescription.Width, loadDescription.Height, format, 1, 1,
		TextureTarget::Texture2D, true);

	if (loadDescription.IsCubeMap)
	{
		description.SetToCubemap();
	}

	// Setting texture filtering to maximal anisotropic.
	TextureSamplingDescription samplingDescription;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &samplingDescription.MaximumAnisotropy);

	if (pSamlingDesc == nullptr) pSamlingDesc = &samplingDescription;

	// Creating texture.
	auto texture = m_Texture2Ds.Request();
	texture->Initialize(description, *pSamlingDesc);

	// Setting texture data.
	SetTextureData(texture, loadDescription, description, mipmapMode);

	TextureGettingResult result;
	result.Texture = texture;
	result.IsTextureCreated = true;
	return result;
}

inline std::string GenerateName(const unsigned char* data, unsigned dataSize)
{
	assert(dataSize >= 8);
	return "_from_memory_" + base64_encode(data, 8);
}

TextureGettingResult ConstantTextureManager::GetTexture2DFromMemory(
	const ImageRawData& textureRawData, TextureMipmapGenerationMode mipmapMode,
	const TextureSamplingDescription* pSamlingDesc)
{
	auto name = GenerateName(textureRawData.Data.GetArray(), textureRawData.Data.GetSize());
	auto it = m_Texture2DFromFileMap.find(name);
	if (it != m_Texture2DFromFileMap.end())
	{
		TextureGettingResult result;
		result.Texture = it->second;
		result.IsTextureCreated = false;
		return result;
	}

	Image2DDescription loadDescription;
	LoadImageFromMemory(m_LoadBuffers, loadDescription, textureRawData, ::FormatSupportHandler());

	auto result = CreateTextureFromLoadResult(loadDescription, mipmapMode, pSamlingDesc);
	m_Texture2DFromFileMap[name] = result.Texture;

	return result;
}

TextureGettingResult ConstantTextureManager::GetTexture2DFromFile(
	const std::string& fileName,
	TextureMipmapGenerationMode mipmapMode,
	const TextureSamplingDescription* pSamlingDesc)
{
	auto it = m_Texture2DFromFileMap.find(fileName);
	if (it != m_Texture2DFromFileMap.end())
	{
		TextureGettingResult result;
		result.Texture = it->second;
		result.IsTextureCreated = false;
		return result;
	}

	auto path = GetCompletePath(fileName);

	// Loading data from file.
	Image2DDescription loadDescription;
	LoadImageFromFile(m_LoadBuffers, loadDescription, path, ::FormatSupportHandler());

	auto result = CreateTextureFromLoadResult(loadDescription, mipmapMode, pSamlingDesc);

	m_Texture2DFromFileMap[fileName] = result.Texture;

	return result;
}

TextureGettingResult ConstantTextureManager::GetDiffuseTexture2DWithOpacity(
	const std::string& diffuseTextureName, const std::string& opacityTextureName,
	unsigned opacityChannelIndex)
{
	CombinedTextureNames textureNames = { diffuseTextureName, opacityTextureName };
	auto it = m_CombinedTextures.find(textureNames);
	if (it != m_CombinedTextures.end())
	{
		TextureGettingResult result;
		result.Texture = it->second;
		result.IsTextureCreated = false;
		return result;
	}

	auto path1 = GetCompletePath(diffuseTextureName);
	auto path2 = GetCompletePath(opacityTextureName);

	// Loading data from file.
	Image2DLoadRequestDescription loadRequest;
	loadRequest.FileNames = { path1, path2 };
	loadRequest.Sources = GetChannelSourceForDiffuseAndOpacityMixture(opacityChannelIndex);
	Image2DDescription loadDescription;
	LoadImageFromFile(m_LoadBuffers, loadDescription, loadRequest, ::FormatSupportHandler());

	auto result = CreateTextureFromLoadResult(loadDescription, TextureMipmapGenerationMode::Default, nullptr);

	m_CombinedTextures[textureNames] = result.Texture;

	return result;
}