// DirectX11Render/Resources/Texture2D.cpp

#include <DirectX11Render/Resources/Texture2D.h>

#include <Core/Comparison.h>
#include <EngineBuildingBlocks/Graphics/Resources/ResourceUtility.h>

using namespace EngineBuildingBlocks;
using namespace DirectXRender;
using namespace DirectX11Render;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2DDescription::Texture2DDescription()
	: Width(0)
	, Height(0)
	, Format(DXGI_FORMAT_UNKNOWN)
	, ArraySize(1)
	, SampleCount(1)
	, Usage(D3D11_USAGE_DEFAULT)
	, BindFlags(TextureBindFlag::None)
	, MiscFlags(TextureMiscFlag::None)
	, ExtraFlags(TextureExtraFlag::HasMipmaps)
{
}

Texture2DDescription::Texture2DDescription(unsigned width, unsigned height,
	DXGI_FORMAT format, unsigned arraySize, unsigned sampleCount,
	D3D11_USAGE usage, TextureBindFlag bindFlags, TextureMiscFlag miscFlags, TextureExtraFlag extraFlags)
	: Width(width)
	, Height(height)
	, Format(format)
	, ArraySize(arraySize)
	, SampleCount(sampleCount)
	, Usage(usage)
	, BindFlags(bindFlags)
	, MiscFlags(miscFlags)
	, ExtraFlags(extraFlags)
{
}

bool Texture2DDescription::IsTextureCube() const
{
	return HasFlag(MiscFlags, TextureMiscFlag::IsTextureCube);
}

bool Texture2DDescription::IsGDICompatible() const
{
	return HasFlag(MiscFlags, TextureMiscFlag::IsGDICompatible);
}

bool Texture2DDescription::HasMipmaps() const
{
	return HasFlag(ExtraFlags, TextureExtraFlag::HasMipmaps);
}

void Texture2DDescription::SetToTextureCube()
{
	ArraySize = 6;
	MiscFlags |= TextureMiscFlag::IsTextureCube;
}

void Texture2DDescription::SetToGDICompatible()
{
	MiscFlags |= TextureMiscFlag::IsGDICompatible;
}

void Texture2DDescription::SetToHaveMipmaps()
{
	ExtraFlags |= TextureExtraFlag::HasMipmaps;
}

D3D11_TEXTURE2D_DESC Texture2DDescription::ToResourceDescription() const
{
	auto countMipmapLevels = GetCountMipmapLevels();

	bool isShaderResource = HasFlag(BindFlags, TextureBindFlag::ShaderResource);
	bool isRenderTarget = HasFlag(BindFlags, TextureBindFlag::RenderTarget);

	D3D11_TEXTURE2D_DESC description;
	description.Width = Width;
	description.Height = Height;
	description.MipLevels = countMipmapLevels;
	description.ArraySize = ArraySize;
	description.Format = Format;
	description.SampleDesc.Count = SampleCount;
	description.SampleDesc.Quality = 0;
	description.Usage = Usage;
	description.BindFlags = (0
		| (isShaderResource ? D3D11_BIND_SHADER_RESOURCE : 0)
		| (isRenderTarget ? D3D11_BIND_RENDER_TARGET : 0)
		| (HasFlag(BindFlags, TextureBindFlag::DepthStencil) ? D3D11_BIND_DEPTH_STENCIL : 0)
		| (HasFlag(BindFlags, TextureBindFlag::UnorderedAccess) ? D3D11_BIND_UNORDERED_ACCESS : 0));
	description.CPUAccessFlags = (0
		| (Usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0)
		| (Usage == D3D11_USAGE_STAGING ? D3D11_CPU_ACCESS_READ : 0));
	description.MiscFlags = (0
		| (isShaderResource && isRenderTarget && countMipmapLevels > 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0)
		| (IsTextureCube() ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0)
		| (IsGDICompatible() ? D3D11_RESOURCE_MISC_GDI_COMPATIBLE : 0));

	return description;
}

bool Texture2DDescription::operator==(const Texture2DDescription& other) const
{
	NumericalEqualCompareBlock(Width);
	NumericalEqualCompareBlock(Height);
	NumericalEqualCompareBlock(Format);
	NumericalEqualCompareBlock(ArraySize);
	NumericalEqualCompareBlock(SampleCount);
	NumericalEqualCompareBlock(Usage);
	NumericalEqualCompareBlock(BindFlags);
	NumericalEqualCompareBlock(MiscFlags);
	NumericalEqualCompareBlock(ExtraFlags);
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
	NumericalLessCompareBlock(SampleCount);
	NumericalLessCompareBlock(Usage);
	NumericalLessCompareBlock(BindFlags);
	NumericalLessCompareBlock(MiscFlags);
	NumericalLessCompareBlock(ExtraFlags);
	return false;
}

bool Texture2DDescription::IsCopyableTo(const Texture2DDescription& destinationDesc) const
{
	auto& other = destinationDesc;
	NumericalEqualCompareBlock(Width);
	NumericalEqualCompareBlock(Height);
	NumericalEqualCompareBlock(Format);		// TODO: formats only have to be compatible not equal.
	NumericalEqualCompareBlock(ArraySize);
	NumericalEqualCompareBlock(SampleCount);
	NumericalEqualCompareBlock(HasMipmaps());
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

unsigned Texture2DDescription::GetCountSubpixels() const
{
	return SampleCount * Width * Height * ArraySize;
}

unsigned Texture2DDescription::GetPixelFormatSizeInBytes() const
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
	return SampleCount * Width * Height * ArraySize * ::GetSizeInBytes(Format);
}

unsigned Texture2DDescription::GetCompactRowPitch() const
{
	return SampleCount * Width * ::GetSizeInBytes(Format);
}

unsigned Texture2DDescription::GetCompactSlicePitch() const
{
	return SampleCount * Width * Height * ::GetSizeInBytes(Format);
}

unsigned Texture2DDescription::GetCountMipmapLevels() const
{
	return (HasMipmaps() ? EngineBuildingBlocks::Graphics::GetMipmapLevelCount(Width, Height) : 1);
}

void Texture2DDescription::Validate() const
{
	if (SampleCount > 1 && GetCountMipmapLevels() > 1)
	{
		RaiseException("Cannot use multisampling and mipmapping together in a texture 2D.");
	}
	if (SampleCount > 1 && IsTextureCube())
	{
		RaiseException("Cannot use multisampling in a texture cube.");
	}
}

bool Texture2DDescription::IsResolvableWithAPICall(const Texture2DDescription& sourceDesc,
	const Texture2DDescription& targetDesc)
{
	return (sourceDesc.Width == targetDesc.Width && sourceDesc.Height == targetDesc.Height
		&& sourceDesc.ArraySize == targetDesc.ArraySize
		&& ::IsResolvableWithAPICall(sourceDesc.Format, targetDesc.Format)
		&& !HasFlag(sourceDesc.BindFlags, TextureBindFlag::DepthStencil));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

const Texture2DDescription& Texture2D::GetDescription() const
{
	return m_Description;
}

ID3D11Texture2D* Texture2D::GetResource()
{
	return m_Resource.Get();
}

ID3D11ShaderResourceView* Texture2D::GetSRV()
{
	return m_SRV.Get();
}

ID3D11UnorderedAccessView* Texture2D::GetUAV()
{
	return m_UAV.Get();
}

ID3D11RenderTargetView* Texture2D::GetRTV()
{
	return m_RTV.Get();
}

ID3D11DepthStencilView* Texture2D::GetDSV()
{
	return m_DSV.Get();
}

void Texture2D::Initialize(ID3D11Device* device, const Texture2DDescription& description,
	const D3D11_SUBRESOURCE_DATA* subResourceData,
	DXGI_FORMAT srvFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat)
{
	m_Description = description;
	CreateResource(device, subResourceData);
	if (HasFlag(description.BindFlags, TextureBindFlag::ShaderResource)) CreateShaderResourceView(device, srvFormat);
	if (HasFlag(description.BindFlags, TextureBindFlag::UnorderedAccess)) CreateUnorderedAccessView(device, uavFormat);
	if (HasFlag(description.BindFlags, TextureBindFlag::RenderTarget)) CreateRenderTargetView(device, rtvFormat);
	if (HasFlag(description.BindFlags, TextureBindFlag::DepthStencil)) CreateDepthStencilView(device, dsvFormat);
}

void Texture2D::CreateResource(ID3D11Device* device, const D3D11_SUBRESOURCE_DATA* subResourceData)
{
	auto hr = device->CreateTexture2D(&m_Description.ToResourceDescription(), subResourceData, &m_Resource);
	if (FAILED(hr))
	{
		RaiseException("Failed to create a texture 2D.");
	}
}

void Texture2D::CreateShaderResourceView(ID3D11Device* device, DXGI_FORMAT srvFormat)
{
	auto countMipmapLevels = m_Description.GetCountMipmapLevels();

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = (srvFormat == DXGI_FORMAT_UNKNOWN ? m_Description.Format : srvFormat);
	if (m_Description.IsTextureCube())
	{
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = countMipmapLevels;
	}
	else
	{
		if (m_Description.ArraySize == 1)
		{
			if (m_Description.SampleCount == 1)
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = countMipmapLevels;
			}
			else
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			}
		}
		else
		{
			if (m_Description.SampleCount == 1)
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.MipLevels = countMipmapLevels;
				srvDesc.Texture2DArray.FirstArraySlice = 0;
				srvDesc.Texture2DArray.ArraySize = m_Description.ArraySize;
			}
			else
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
				srvDesc.Texture2DMSArray.FirstArraySlice = 0;
				srvDesc.Texture2DMSArray.ArraySize = m_Description.ArraySize;
			}
		}
	}
	auto hr = device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, &m_SRV);
	if (FAILED(hr))
	{
		RaiseException("Failed to create a shader resource view for a texture 2D.");
	}
}

void Texture2D::CreateUnorderedAccessView(ID3D11Device* device, DXGI_FORMAT uavFormat)
{
	assert(!m_Description.IsTextureCube());
	assert(m_Description.SampleCount == 1);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = (uavFormat == DXGI_FORMAT_UNKNOWN ? m_Description.Format : uavFormat);
	if (m_Description.ArraySize == 1)
	{
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
	}
	else
	{
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = m_Description.ArraySize;
	}
	auto hr = device->CreateUnorderedAccessView(m_Resource.Get(), &uavDesc, &m_UAV);
	if (FAILED(hr))
	{
		RaiseException("Failed to create a unordered access view for a texture 2D.");
	}
}

void Texture2D::CreateRenderTargetView(ID3D11Device* device, DXGI_FORMAT rtvFormat)
{
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = (rtvFormat == DXGI_FORMAT_UNKNOWN ? m_Description.Format : rtvFormat);
	if (m_Description.IsTextureCube())
	{
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;
		rtvDesc.Texture2DArray.ArraySize = m_Description.ArraySize;
	}
	else
	{
		if (m_Description.ArraySize == 1)
		{
			if (m_Description.SampleCount == 1)
			{
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = 0;
			}
			else
			{
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			}
		}
		else
		{
			if (m_Description.SampleCount == 1)
			{
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.MipSlice = 0;
				rtvDesc.Texture2DArray.FirstArraySlice = 0;
				rtvDesc.Texture2DArray.ArraySize = m_Description.ArraySize;
			}
			else
			{
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
				rtvDesc.Texture2DMSArray.FirstArraySlice = 0;
				rtvDesc.Texture2DMSArray.ArraySize = m_Description.ArraySize;
			}
		}
	}
	auto hr = device->CreateRenderTargetView(m_Resource.Get(), &rtvDesc, &m_RTV);
	if (FAILED(hr))
	{
		RaiseException("Failed to create a render target view for a texture 2D.");
	}
}

void Texture2D::CreateDepthStencilView(ID3D11Device* device, DXGI_FORMAT dsvFormat)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = (dsvFormat == DXGI_FORMAT_UNKNOWN ? m_Description.Format : dsvFormat);
	if (m_Description.IsTextureCube())
	{
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = 0;
		dsvDesc.Texture2DArray.FirstArraySlice = 0;
		dsvDesc.Texture2DArray.ArraySize = m_Description.ArraySize;
	}
	else
	{
		if (m_Description.ArraySize == 1)
		{
			if (m_Description.SampleCount == 1)
			{
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				dsvDesc.Texture2D.MipSlice = 0;
			}
			else
			{
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			}
		}
		else
		{
			if (m_Description.SampleCount == 1)
			{
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.MipSlice = 0;
				dsvDesc.Texture2DArray.FirstArraySlice = 0;
				dsvDesc.Texture2DArray.ArraySize = m_Description.ArraySize;
			}
			else
			{
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
				dsvDesc.Texture2DMSArray.FirstArraySlice = 0;
				dsvDesc.Texture2DMSArray.ArraySize = m_Description.ArraySize;
			}
		}
	}
	dsvDesc.Flags = 0;
	auto hr = device->CreateDepthStencilView(m_Resource.Get(), &dsvDesc, &m_DSV);
	if (FAILED(hr))
	{
		RaiseException("Failed to create a depth stencil view for a texture 2D.");
	}
}

void Texture2D::GetData(ID3D11DeviceContext* context, void* pData, unsigned subResourceIndex)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto hr = context->Map(m_Resource.Get(), subResourceIndex, D3D11_MAP_READ, 0, &mappedResource);
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to map a texture 2D for reading.");
	}
	assert(mappedResource.RowPitch == m_Description.GetCompactRowPitch());
	memcpy(pData, mappedResource.pData, m_Description.GetSizeInBytes());
	context->Unmap(m_Resource.Get(), 0);
}

void Texture2D::SetData(ID3D11DeviceContext* context, const void* pData, unsigned subResourceIndex)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto hr = context->Map(m_Resource.Get(), subResourceIndex, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to map a texture 2D for writing.");
	}
	assert(mappedResource.RowPitch == m_Description.GetCompactRowPitch());
	memcpy(mappedResource.pData, pData, m_Description.GetSizeInBytes());
	context->Unmap(m_Resource.Get(), 0);
}