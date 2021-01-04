// DirectX11Render/Resources/Texture2D.h

#ifndef _DIRECTX11RENDER_TEXTURE2D_H_INCLUDED_
#define _DIRECTX11RENDER_TEXTURE2D_H_INCLUDED_

#include <Core/Enum.h>
#include <WindowsApplication/DirectX/Format.h>
#include <DirectX11Render/DirectX11Includes.h>

#include <vector>

namespace DirectX11Render
{
	enum class TextureBindFlag : unsigned char
	{
		None = 0x00,
		ShaderResource = 0x01,
		UnorderedAccess = 0x02,
		RenderTarget = 0x04,
		DepthStencil = 0x08,
	};

	UseEnumAsFlagSet(TextureBindFlag);

	enum class TextureMiscFlag : unsigned char
	{
		None = 0x00,
		IsTextureCube = 0x01,
		IsGDICompatible = 0x02
	};

	UseEnumAsFlagSet(TextureMiscFlag);

	enum class TextureExtraFlag : unsigned char
	{
		None = 0x00,
		HasMipmaps = 0x01
	};

	UseEnumAsFlagSet(TextureExtraFlag);

	struct Texture2DDescription
	{
		unsigned Width;
		unsigned Height;
		DXGI_FORMAT Format;
		unsigned ArraySize;
		unsigned SampleCount;

		D3D11_USAGE Usage;
		TextureBindFlag BindFlags;
		TextureMiscFlag MiscFlags;
		TextureExtraFlag ExtraFlags;

		Texture2DDescription();
		Texture2DDescription(unsigned width, unsigned height,
			DXGI_FORMAT format,
			unsigned arraySize = 1,
			unsigned sampleCount = 1,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
			TextureBindFlag bindFlags = TextureBindFlag::None,
			TextureMiscFlag miscFlags = TextureMiscFlag::None,
			TextureExtraFlag extraFlags = TextureExtraFlag::None);

		bool IsTextureCube() const;
		bool IsGDICompatible() const;
		bool HasMipmaps() const;

		void SetToTextureCube();
		void SetToGDICompatible();
		void SetToHaveMipmaps();

		D3D11_TEXTURE2D_DESC ToResourceDescription() const;

		bool operator==(const Texture2DDescription& other) const;
		bool operator!=(const Texture2DDescription& other) const;
		bool operator<(const Texture2DDescription& other) const;

		bool IsCopyableTo(const Texture2DDescription& destinationDesc) const;

		unsigned GetCountPixelsInSlice() const;
		unsigned GetCountPixels() const;
		unsigned GetCountSubpixels() const;
		unsigned GetPixelFormatSizeInBytes() const;
		unsigned GetSizeInBytes() const;
		unsigned GetCountChannels() const;
		unsigned GetCompactRowPitch() const;
		unsigned GetCompactSlicePitch() const;
		DirectXRender::PixelType GetPixelType() const;

		unsigned GetCountMipmapLevels() const;

		void Validate() const;

		static bool IsResolvableWithAPICall(
			const Texture2DDescription& sourceDesc,
			const Texture2DDescription& targetDesc);
	};

	class Texture2D
	{
		ComPtr<ID3D11Texture2D> m_Resource;
		ComPtr<ID3D11ShaderResourceView> m_SRV;
		ComPtr<ID3D11UnorderedAccessView> m_UAV;
		ComPtr<ID3D11RenderTargetView> m_RTV;
		ComPtr<ID3D11DepthStencilView> m_DSV;

		Texture2DDescription m_Description;

		void CreateResource(ID3D11Device* device,
			const D3D11_SUBRESOURCE_DATA* subResourceData);
		void CreateShaderResourceView(ID3D11Device* device,
			DXGI_FORMAT srvFormat);
		void CreateUnorderedAccessView(ID3D11Device* device,
			DXGI_FORMAT uavFormat);
		void CreateRenderTargetView(ID3D11Device* device,
			DXGI_FORMAT rtvFormat);
		void CreateDepthStencilView(ID3D11Device* device,
			DXGI_FORMAT dsvFormat);

	public:

		const Texture2DDescription& GetDescription() const;

		ID3D11Texture2D* GetResource();
		ID3D11ShaderResourceView* GetSRV();
		ID3D11UnorderedAccessView* GetUAV();
		ID3D11RenderTargetView* GetRTV();
		ID3D11DepthStencilView* GetDSV();

		void Initialize(ID3D11Device* device,
			const Texture2DDescription& description,
			const D3D11_SUBRESOURCE_DATA* subResourceData = nullptr,
			DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT uavFormat = DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT rtvFormat = DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN);

		void GetData(ID3D11DeviceContext* context, void* pData, unsigned subResourceIndex = 0);
		void SetData(ID3D11DeviceContext* context, const void* pData, unsigned subResourceIndex = 0);
	};
}

#endif