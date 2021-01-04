// DirectX12Render/Resources/ConstantTextureManager.cpp

#include <DirectX12Render/Resources/ConstantTextureManager.h>

#include <Core/Comparison.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/Graphics/Resources/ImageHelper.h>
#include <WindowsApplication/DirectX/Format.h>
#include <DirectX12Render/Resources/TransferBuffer.h>

#include <External/Encoding/Base64Encoding.h>

#include <filesystem>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace DirectXRender;
using namespace DirectX12Render;

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

ConstantTextureManager::ConstantTextureManager(TransferBufferManager* uploadBufferManager,
	PathHandler* pathHandler)
	: m_UploadBufferManager(uploadBufferManager)
	, m_PathHandler(pathHandler)
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

TextureGettingResult ConstantTextureManager::CreateTextureFromLoadResult(ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList, IDescriptorHeap* srvDescHeap,
	const Image2DDescription& loadDescription, TextureMipmapGenerationMode mipmapMode)
{
	// Acquiring loaded buffers.
	m_SetBufferPointers.Resize(loadDescription.ArraySize);
	for (unsigned i = 0; i < loadDescription.ArraySize; i++)
	{
		m_SetBufferPointers[i] = m_LoadBuffers[i].GetArray();
	}

	// Creating texture description.
	auto format = ToDXGIFormat(loadDescription, true);
	Texture2DDescription description;
	description.Width = static_cast<UINT64>(loadDescription.Width);
	description.Height = loadDescription.Height;
	description.Format = format;
	description.HasMipmaps = true;	// We create the mipmap levels by default.
	description.SampleCount = 1;	// Constant textures are non-multisampled.

	if (loadDescription.IsCubeMap)
	{
		description.SetToTextureCubeDescription();
	}

	// Creating texture.
	auto texture = m_Texture2Ds.Request();
	texture->Initialize(device, description, D3D12_RESOURCE_STATE_COPY_DEST, srvDescHeap);

	// Getting an upload buffer and using it in order to upload the data.
	UploadBufferDescription uploadBufferDescription;
	uploadBufferDescription.Size = GetRequiredIntermediateSize(texture->GetResource(), 0,
		description.GetCountMipmapLevels() * description.ArraySize);
	auto uploadBuffer = m_UploadBufferManager->RequestUploadBuffer(device, uploadBufferDescription);

	// Setting data using the upload buffer.
	texture->SetData(commandList, uploadBuffer, m_SetBufferPointers, mipmapMode);

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

TextureGettingResult ConstantTextureManager::GetTexture2DFromMemory(ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList, IDescriptorHeap* srvDescHeap,
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

	auto result = CreateTextureFromLoadResult(device, commandList, srvDescHeap, loadDescription, mipmapMode);
	m_Texture2DFromFileMap[name] = result.Texture;

	return result;
}

TextureGettingResult ConstantTextureManager::GetTexture2DFromFile(ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList, IDescriptorHeap* srvDescHeap,
	const std::string& fileName, TextureMipmapGenerationMode mipmapMode)
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

	auto result = CreateTextureFromLoadResult(device, commandList, srvDescHeap, loadDescription, mipmapMode);

	m_Texture2DFromFileMap[fileName] = result.Texture;

	return result;
}

TextureGettingResult ConstantTextureManager::GetDiffuseTexture2DWithOpacity(ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	IDescriptorHeap* srvDescHeap,
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

	auto result = CreateTextureFromLoadResult(device, commandList, srvDescHeap, loadDescription,
		TextureMipmapGenerationMode::Default);

	m_CombinedTextures[textureNames] = result.Texture;

	return result;
}