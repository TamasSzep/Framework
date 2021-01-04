// DirectX11Render/Resources/ConstantTextureManager.cpp

#include <DirectX11Render/Resources/ConstantTextureManager.h>

#include <Core/Comparison.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/Graphics/Resources/ImageHelper.h>
#include <WindowsApplication/DirectX/Format.h>

#include <External/Encoding/Base64Encoding.h>

// Implementation is already compiled to EngineBuildingBlocks.
#include <stb/stb_image_resize.h>

#include <filesystem>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace DirectXRender;
using namespace DirectX11Render;

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

	~FormatSupportHandler() override {}

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

void ConstantTextureManager::SetSubResourceData(const DirectX11Render::Texture2DDescription& texDesc,
	TextureMipmapGenerationMode mipmapMode)
{
	auto width = texDesc.Width;
	auto height = texDesc.Height;
	auto arraySize = texDesc.ArraySize;
	auto pixelByteCount = texDesc.GetPixelFormatSizeInBytes();
	auto channelCount = texDesc.GetCountChannels();
	auto rowPitch = texDesc.GetCompactRowPitch();
	auto slicePitch = texDesc.GetCompactSlicePitch();
	auto countMipmapLevels = texDesc.GetCountMipmapLevels();
	auto pixelType = texDesc.GetPixelType();

	m_SubResourceData.ClearAndReserve(arraySize * countMipmapLevels);
	m_AdditionalBuffers.clear();
	m_AdditionalBuffers.reserve(arraySize * (countMipmapLevels - 1));

	for (unsigned arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
	{
		auto sourceBuffer = m_LoadBuffers[arrayIndex].GetArray();

		// Creating first subresource description.		
		auto& firstSubresourceData = m_SubResourceData.PushBackPlaceHolder();
		firstSubresourceData.pSysMem = sourceBuffer;
		firstSubresourceData.SysMemPitch = rowPitch;
		firstSubresourceData.SysMemSlicePitch = slicePitch;

		if (countMipmapLevels > 1)
		{
			unsigned sourceWidth = width;
			unsigned sourceHeight = height;

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
			else RaiseException("Pixel type is not handled when generating mipmaps.");

			int hasAlphaChannel = (pixelType == PixelType::Unorm || pixelType == PixelType::UnormSRGB);
			int alphaChannelIndex = (hasAlphaChannel == 0 ? 0 : 3);

			stbir_colorspace colorSpace = (pixelType == PixelType::Unorm
				? STBIR_COLORSPACE_SRGB
				: STBIR_COLORSPACE_LINEAR);

			for (unsigned mipmapIndex = 1; mipmapIndex < countMipmapLevels; mipmapIndex++)
			{
				unsigned targetWidth = std::max(sourceWidth >> 1, 1U);
				unsigned targetHeight = std::max(sourceHeight >> 1, 1U);
				m_AdditionalBuffers.emplace_back();
				auto& targetVector = m_AdditionalBuffers.back();
				auto targetSize = targetWidth * targetHeight * pixelByteCount;
				targetVector.Resize(targetSize);
				auto targetBuffer = targetVector.GetArray();

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

				// Creating additional subresource description.
				auto& currentSubresourceData = m_SubResourceData.PushBackPlaceHolder();
				currentSubresourceData.pSysMem = targetBuffer;
				currentSubresourceData.SysMemPitch = targetWidth * pixelByteCount;
				currentSubresourceData.SysMemSlicePitch = targetSize;

				sourceWidth = targetWidth;
				sourceHeight = targetHeight;
				sourceBuffer = targetBuffer;
			}
		}
	}
}

TextureGettingResult ConstantTextureManager::CreateTextureFromLoadResult(ID3D11Device* device,
	const Image2DDescription& loadDescription,
	TextureMipmapGenerationMode mipmapMode)
{
	// Creating texture description. Immutable, mipmapped, non-multisampled texture with SRV.
	auto format = ToDXGIFormat(loadDescription, true);
	Texture2DDescription description(loadDescription.Width, loadDescription.Height, format, 1, 1,
		D3D11_USAGE_IMMUTABLE, TextureBindFlag::ShaderResource, TextureMiscFlag::None, TextureExtraFlag::HasMipmaps);

	if (loadDescription.IsCubeMap)
	{
		description.SetToTextureCube();
	}

	SetSubResourceData(description, mipmapMode);

	// Creating texture.
	auto texture = m_Texture2Ds.Request();
	texture->Initialize(device, description, m_SubResourceData.GetArray());

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

TextureGettingResult ConstantTextureManager::GetTexture2DFromMemory(ID3D11Device* device,
	const ImageRawData& textureRawData, TextureMipmapGenerationMode mipmapMode)
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

	auto result = CreateTextureFromLoadResult(device, loadDescription, mipmapMode);
	m_Texture2DFromFileMap[name] = result.Texture;

	return result;
}

TextureGettingResult ConstantTextureManager::GetTexture2DFromFile(ID3D11Device* device,
	const std::string& fileName,
	TextureMipmapGenerationMode mipmapMode)
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

	auto result = CreateTextureFromLoadResult(device, loadDescription, mipmapMode);

	m_Texture2DFromFileMap[fileName] = result.Texture;

	return result;
}

TextureGettingResult ConstantTextureManager::GetDiffuseTexture2DWithOpacity(ID3D11Device* device,
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

	auto result = CreateTextureFromLoadResult(device, loadDescription, TextureMipmapGenerationMode::Default);

	m_CombinedTextures[textureNames] = result.Texture;

	return result;
}