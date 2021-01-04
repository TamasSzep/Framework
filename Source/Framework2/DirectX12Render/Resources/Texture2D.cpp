// DirectX12Render/Resources/Texture2D.cpp

#include <DirectX12Render/Resources/Texture2D.h>

#include <EngineBuildingBlocks/Graphics/Resources/ImageHelper.h>
#include <WindowsApplication/DirectX/Format.h>
#include <DirectX12Render/Resources/DescriptorHeap.h>
#include <DirectX12Render/Resources/TransferBuffer.h>

// Implementation is already compiled to EngineBuildingBlocks.
#include <stb/stb_image_resize.h>

#include <cassert>

using namespace DirectXRender;
using namespace DirectX12Render;

void Texture2D::Initialize(ID3D12Device* device, const Texture2DDescription& description,
	D3D12_RESOURCE_STATES startState, IDescriptorHeap* srvDescHeap)
{
	description.Validate();

	m_Description = description;
	m_State = startState;

	CreateResource(device, nullptr);
	if (srvDescHeap != nullptr)
	{
		CreateShaderResourceView(device, srvDescHeap);
	}
}

void Texture2D::SetData(ID3D12GraphicsCommandList* commandList, UploadBuffer* uploadBuffer,
	Core::SimpleTypeVectorU<unsigned char*> data, TextureMipmapGenerationMode mipmapMode)
{
	assert(m_Description.ArraySize = data.GetSize());

	Transition(commandList, D3D12_RESOURCE_STATE_COPY_DEST);

	unsigned width = m_Description.Width;
	unsigned height = m_Description.Height;
	unsigned arraySize = m_Description.ArraySize;
	unsigned pixelByteCount = m_Description.GetPixelSizeInBytes();
	unsigned channelCount = m_Description.GetCountChannels();

	UINT64 rowPitch = width * pixelByteCount;
	size_t slicePitch = rowPitch * height;

	unsigned countMipmapLevels = m_Description.GetCountMipmapLevels();

	Core::SimpleTypeVectorU<D3D12_SUBRESOURCE_DATA> subresourceData;
	std::vector<std::vector<Core::ByteVector>> mipDatas(arraySize);

	for (unsigned arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
	{
		auto& mipData = mipDatas[arrayIndex];
		mipData.resize(countMipmapLevels - 1);

		// Creating first subresource description.		
		auto& firstSubresourceData = subresourceData.PushBackPlaceHolder();
		firstSubresourceData.pData = data[arrayIndex];
		firstSubresourceData.RowPitch = rowPitch;
		firstSubresourceData.SlicePitch = slicePitch;

		if (countMipmapLevels > 1)
		{
			unsigned sourceWidth = width;
			unsigned sourceHeight = height;

			stbir_datatype dataType;
			auto pixelType = m_Description.GetPixelType();
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
			else EngineBuildingBlocks::RaiseException("Pixel type is not handled when generating mipmaps.");

			int hasAlphaChannel = (pixelType == PixelType::Unorm || pixelType == PixelType::UnormSRGB);
			int alphaChannelIndex = (hasAlphaChannel == 0 ? 0 : 3);

			stbir_colorspace colorSpace = (pixelType == PixelType::Unorm
				? STBIR_COLORSPACE_SRGB
				: STBIR_COLORSPACE_LINEAR);

			auto sourceBuffer = data[arrayIndex];

			for (unsigned i = 1; i < countMipmapLevels; i++)
			{
				unsigned targetWidth = std::max(sourceWidth >> 1, 1U);
				unsigned targetHeight = std::max(sourceHeight >> 1, 1U);
				auto& targetVector = mipData[i - 1];
				auto targetSize = targetWidth * targetHeight * pixelByteCount;
				targetVector.Resize(targetSize);
				auto targetBuffer = targetVector.GetArray();

				if (mipmapMode == TextureMipmapGenerationMode::Pow2Box)
				{
					assert(pixelType == PixelType::Unorm);
					assert(pixelByteCount == channelCount);
					EngineBuildingBlocks::Graphics::DownsampleLinearUnormWithBoxFilterPow2(
						sourceBuffer, sourceWidth, sourceHeight, targetBuffer, channelCount);
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
				auto& currentSubresourceData = subresourceData.PushBackPlaceHolder();
				currentSubresourceData.pData = targetBuffer;
				currentSubresourceData.RowPitch = targetWidth * pixelByteCount;
				currentSubresourceData.SlicePitch = targetSize;

				sourceWidth = targetWidth;
				sourceHeight = targetHeight;
				sourceBuffer = targetBuffer;
			}
		}
	}

	UpdateSubresources(commandList, m_Resource.Get(), uploadBuffer->GetResource(),
		0, 0, subresourceData.GetSize(), subresourceData.GetArray());
}