// DirectX12Render/Resources/TextureBase.cpp

#include <DirectX12Render/Resources/Texture2dBase.h>

#include <Core/Comparison.h>
#include <Core/Utility.hpp>
#include <EngineBuildingBlocks/Graphics/Resources/ResourceUtility.h>
#include <DirectX12Render/Resources/DescriptorHeap.h>
#include <DirectX12Render/Resources/TransferBuffer.h>

#include <set>

using namespace DirectXRender;
using namespace DirectX12Render;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2DDescription::Texture2DDescription()
	: Width(0)
	, Height(0)
	, Format(DXGI_FORMAT_UNKNOWN)
	, ArraySize(1)
	, HasMipmaps(false)
	, IsCubeTexture(false)
	, SampleCount(1)
	, Flags(D3D12_RESOURCE_FLAG_NONE)
{
}

void Texture2DDescription::SetToTextureCubeDescription()
{
	ArraySize = 6;
	IsCubeTexture = true;
}

CD3DX12_RESOURCE_DESC Texture2DDescription::ToResourceDescription() const
{
	return CD3DX12_RESOURCE_DESC::Tex2D(Format, Width, Height, ArraySize, GetCountMipmapLevels(), SampleCount, 0, Flags);
}

bool Texture2DDescription::operator==(const Texture2DDescription& other) const
{
	NumericalEqualCompareBlock(Width);
	NumericalEqualCompareBlock(Height);
	NumericalEqualCompareBlock(Format);
	NumericalEqualCompareBlock(ArraySize);
	NumericalEqualCompareBlock(HasMipmaps);
	NumericalEqualCompareBlock(IsCubeTexture);
	NumericalEqualCompareBlock(SampleCount);
	NumericalEqualCompareBlock(Flags);
	return true;
}

bool Texture2DDescription::operator!=(const Texture2DDescription& other) const
{
	return !(*this == other);
}

bool Texture2DDescription::operator<(const Texture2DDescription& other) const
{
	NumericalLessCompareBlock(Width);
	NumericalLessCompareBlock(Height);
	NumericalLessCompareBlock(Format);
	NumericalLessCompareBlock(ArraySize);
	NumericalLessCompareBlock(HasMipmaps);
	NumericalLessCompareBlock(IsCubeTexture);
	NumericalLessCompareBlock(SampleCount);
	NumericalLessCompareBlock(Flags);
	return false;
}

bool Texture2DDescription::IsCopyableTo(const Texture2DDescription& destinationDesc) const
{
	auto& other = destinationDesc;
	NumericalEqualCompareBlock(Width);
	NumericalEqualCompareBlock(Height);
	NumericalEqualCompareBlock(Format);		// TODO: formats only have to be compatible not equal.
	NumericalEqualCompareBlock(ArraySize);
	NumericalEqualCompareBlock(HasMipmaps);
	NumericalEqualCompareBlock(IsCubeTexture);
	NumericalEqualCompareBlock(SampleCount);
	return true;
}

unsigned Texture2DDescription::GetCountPixelsInSlice() const
{
	return Width * Height;
}

unsigned Texture2DDescription::GetCountPixels() const
{
	return Width * Height * ArraySize;
}

unsigned Texture2DDescription::GetPixelSizeInBytes() const
{
	return ::GetSizeInBytes(Format);
}

unsigned Texture2DDescription::GetCountChannels() const
{
	return ::GetCountChannels(Format);
}

PixelType Texture2DDescription::GetPixelType() const
{
	return ::GetPixelType(Format);
}

unsigned Texture2DDescription::GetSizeInBytes() const
{
	return Width * Height * ArraySize * ::GetSizeInBytes(Format);
}

unsigned Texture2DDescription::GetCountMipmapLevels() const
{
	return (HasMipmaps ? EngineBuildingBlocks::Graphics::GetMipmapLevelCount(Width, Height) : 1);
}

void Texture2DDescription::Validate() const
{
	if (SampleCount > 1 && GetCountMipmapLevels() > 1)
	{
		EngineBuildingBlocks::RaiseException("Cannot use multisampling and mipmapping together in a texture 2D.");
	}
	if (SampleCount > 1 && IsCubeTexture)
	{
		EngineBuildingBlocks::RaiseException("Cannot use multisampling in a texture cube.");
	}
}

bool Texture2DDescription::IsResolvableWithAPICall(const Texture2DDescription& sourceDesc,
	const Texture2DDescription& targetDesc)
{
	return (sourceDesc.Width == targetDesc.Width && sourceDesc.Height == targetDesc.Height
		&& sourceDesc.ArraySize == targetDesc.ArraySize
		&& ::IsResolvableWithAPICall(sourceDesc.Format, targetDesc.Format)
		&& ((sourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) == 0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

const Texture2DDescription& Texture2DBase::GetDescription() const
{
	return m_Description;
}

D3D12_RESOURCE_STATES Texture2DBase::GetState() const
{
	return m_State;
}

D3D12_RESOURCE_STATES Texture2DBase::Transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES targetState)
{
	auto oldState = m_State;
	if (m_State != targetState)
	{
		commandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), m_State, targetState));

		m_State = targetState;
	}
	return oldState;
}

D3D12_RESOURCE_STATES Texture2DBase::TransitionToShaderResource(ID3D12GraphicsCommandList* commandList)
{
	return Transition(commandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		| D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

D3D12_RESOURCE_STATES Texture2DBase::TransitionToPixelShaderResource(ID3D12GraphicsCommandList* commandList)
{
	return Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

D3D12_RESOURCE_STATES Texture2DBase::TransitionToNonPixelShaderResource(ID3D12GraphicsCommandList* commandList)
{
	return Transition(commandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

D3D12_RESOURCE_STATES Texture2DBase::TransitionToUAV(ID3D12GraphicsCommandList* commandList)
{
	return Transition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void Texture2DBase::IndicateTransitioned(D3D12_RESOURCE_STATES targetState)
{
	m_State = targetState;
}

void Texture2DBase::CopyData(ID3D12GraphicsCommandList* commandList,
	ReadbackBuffer* readbackBuffer)
{
	unsigned width = m_Description.Width;
	unsigned height = m_Description.Height;
	unsigned arraySize = m_Description.ArraySize;
	auto format = m_Description.Format;

	assert(arraySize == 1);

	unsigned rowPitch = width * GetSizeInBytes(m_Description.Format);
	unsigned totalSize = rowPitch * height;

	auto readbackResource = readbackBuffer->GetResource();

	auto oldState = Transition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);

	D3D12_TEXTURE_COPY_LOCATION copySource, copyDestination;
	copySource = copyDestination = {};
	copySource.pResource = m_Resource.Get();
	copySource.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	copySource.SubresourceIndex = 0;
	copyDestination.pResource = readbackResource;
	copyDestination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	copyDestination.PlacedFootprint.Offset = 0;
	copyDestination.PlacedFootprint.Footprint.Width = width;
	copyDestination.PlacedFootprint.Footprint.Height = height;
	copyDestination.PlacedFootprint.Footprint.Depth = arraySize;
	copyDestination.PlacedFootprint.Footprint.RowPitch = Core::AlignSize<unsigned>(
		width * GetSizeInBytes(format), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	copyDestination.PlacedFootprint.Footprint.Format = format;
	commandList->CopyTextureRegion(&copyDestination, 0, 0, 0, &copySource, nullptr);

	Transition(commandList, oldState);
}

ID3D12Resource* Texture2DBase::GetResource()
{
	return m_Resource.Get();
}

void Texture2DBase::CreateResource(ID3D12Device* device, const D3D12_CLEAR_VALUE* clearValue)
{
	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&m_Description.ToResourceDescription(),
		m_State,
		clearValue,
		IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a texture 2D.");
	}
}

const Texture2DBase::ShaderResourceView& Texture2DBase::GetShaderResourceView(unsigned index) const
{
	return m_ShaderResourceViews[index];
}

unsigned Texture2DBase::CreateShaderResourceView(ID3D12Device* device, IDescriptorHeap* srvDescHeap,
	DXGI_FORMAT srvFormat)
{
	auto srvIndex = m_ShaderResourceViews.GetSize();
	auto& srv = m_ShaderResourceViews.PushBackPlaceHolder();

	srv.Heap = srvDescHeap;
	srv.Format = srvFormat;

	// Getting descriptor handles on the heap.
	srv.IndexOnDescriptorHeap
		= srvDescHeap->RequestHandles(srv.CPUDescriptorHandle, srv.GPUDescriptorHandle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = srvFormat;
	if (m_Description.IsCubeTexture)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = m_Description.GetCountMipmapLevels();
	}
	else
	{
		if (m_Description.SampleCount == 1)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = m_Description.GetCountMipmapLevels();
		}
		else
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
		}
	}
	device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, srv.CPUDescriptorHandle);

	return srvIndex;
}

unsigned Texture2DBase::CreateShaderResourceView(ID3D12Device* device, IDescriptorHeap* srvDescHeap)
{
	return CreateShaderResourceView(device, srvDescHeap, GetAccessFormat());
}

unsigned Texture2DBase::GetCountShaderResourceViews() const
{
	return m_ShaderResourceViews.GetSize();
}

bool Texture2DBase::HasShaderResourceView() const
{
	return (GetCountShaderResourceViews() > 0);
}

const UnorderedAccessView& Texture2DBase::GetUnorderedAccessView(unsigned index) const
{
	return m_UnorderedAccessViews[index];
}

unsigned Texture2DBase::CreateUnorderedAccessView(ID3D12Device* device, IDescriptorHeap* uavDescHeap,
	DXGI_FORMAT uavFormat)
{
	if (m_Description.IsCubeTexture)
	{
		EngineBuildingBlocks::RaiseException("Cube textures cannot have unordered access views.");
	}
	if (m_Description.SampleCount > 1)
	{
		EngineBuildingBlocks::RaiseException("Multisampled textures cannot have unordered access views.");
	}
	if ((m_Description.Flags | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
	{
		EngineBuildingBlocks::RaiseException("Unordered access flag is not set on the texture: cannot create UAV.");
	}

	auto uavIndex = m_UnorderedAccessViews.GetSize();
	auto& uav = m_UnorderedAccessViews.PushBackPlaceHolder();

	uav.Heap = uavDescHeap;
	uav.Format = uavFormat;

	// Getting descriptor handles on the heap.
	uav.IndexOnDescriptorHeap
		= uavDescHeap->RequestHandles(uav.CPUDescriptorHandle, uav.GPUDescriptorHandle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = uavFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(m_Resource.Get(), nullptr, &uavDesc, uav.CPUDescriptorHandle);

	return uavIndex;
}

unsigned Texture2DBase::CreateUnorderedAccessView(ID3D12Device* device, IDescriptorHeap* uavDescHeap)
{
	return CreateUnorderedAccessView(device, uavDescHeap, GetAccessFormat());
}

unsigned Texture2DBase::GetCountUnorderedAccessViews() const
{
	return m_UnorderedAccessViews.GetSize();
}

bool Texture2DBase::HasUnorderedAccessView() const
{
	return (GetCountUnorderedAccessViews() > 0);
}

///////////////////////////////////////// Texture formats. /////////////////////////////////////////

DXGI_FORMAT Texture2DBase::GetAccessFormat() const
{
	auto format = m_Description.Format;
	if (IsTyped(format)) return format;
	return ResolveTypelessFormat();
}

DXGI_FORMAT Texture2DBase::ResolveTypelessFormat() const
{
	EngineBuildingBlocks::RaiseException("Format resolving is not implemented.");
	return DXGI_FORMAT_UNKNOWN;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////