// DirectX12Render/Resources/Texture2DBase.h

#ifndef _DIRECTX12RENDER_TEXTURE2DBASE_H_INCLUDED_
#define _DIRECTX12RENDER_TEXTURE2DBASE_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <WindowsApplication/DirectX/Format.h>
#include <DirectX12Render/Resources/View.h>

namespace DirectX12Render
{
	class IDescriptorHeap;
	class UploadBuffer;
	class ReadbackBuffer;

	struct Texture2DDescription
	{
		unsigned Width;
		unsigned Height;
		DXGI_FORMAT Format;
		unsigned ArraySize;

		bool IsCubeTexture;
		bool HasMipmaps;
		unsigned SampleCount;
		D3D12_RESOURCE_FLAGS Flags;

		Texture2DDescription();

		void SetToTextureCubeDescription();

		CD3DX12_RESOURCE_DESC ToResourceDescription() const;

		bool operator==(const Texture2DDescription& other) const;
		bool operator!=(const Texture2DDescription& other) const;
		bool operator<(const Texture2DDescription& other) const;

		bool IsCopyableTo(const Texture2DDescription& destinationDesc) const;

		unsigned GetCountPixelsInSlice() const;
		unsigned GetCountPixels() const;
		unsigned GetPixelSizeInBytes() const;
		unsigned GetSizeInBytes() const;
		unsigned GetCountChannels() const;
		DirectXRender::PixelType GetPixelType() const;

		unsigned GetCountMipmapLevels() const;

		void Validate() const;

		static bool IsResolvableWithAPICall(
			const Texture2DDescription& sourceDesc,
			const Texture2DDescription& targetDesc);
	};

	class Texture2DBase
	{
	protected:

		Texture2DDescription m_Description;
		D3D12_RESOURCE_STATES m_State;

		ComPtr<ID3D12Resource> m_Resource;

		void CreateResource(ID3D12Device* device,
			const D3D12_CLEAR_VALUE* clearValue);

	public:

		const Texture2DDescription& GetDescription() const;
		D3D12_RESOURCE_STATES GetState() const;

		ID3D12Resource* GetResource();

		D3D12_RESOURCE_STATES Transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES targetState);
		D3D12_RESOURCE_STATES TransitionToShaderResource(ID3D12GraphicsCommandList* commandList);
		D3D12_RESOURCE_STATES TransitionToPixelShaderResource(ID3D12GraphicsCommandList* commandList);
		D3D12_RESOURCE_STATES TransitionToNonPixelShaderResource(ID3D12GraphicsCommandList* commandList);
		D3D12_RESOURCE_STATES TransitionToUAV(ID3D12GraphicsCommandList* commandList);

		// This function should be called if the transition is handled
		// by the user code.
		void IndicateTransitioned(D3D12_RESOURCE_STATES targetState);

		void CopyData(ID3D12GraphicsCommandList* commandList,
			ReadbackBuffer* readbackBuffer);

	protected:

		DXGI_FORMAT GetAccessFormat() const;
		virtual DXGI_FORMAT ResolveTypelessFormat() const;

	public: // Shader resource view.

		struct ShaderResourceView
		{
			IDescriptorHeap* Heap;
			unsigned IndexOnDescriptorHeap;
			CD3DX12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle;
			DXGI_FORMAT Format;
		};

		unsigned CreateShaderResourceView(ID3D12Device* device,
			IDescriptorHeap* srvDescHeap,
			DXGI_FORMAT srvFormat);
		unsigned CreateShaderResourceView(ID3D12Device* device,
			IDescriptorHeap* srvDescHeap);

		unsigned GetCountShaderResourceViews() const;
		bool HasShaderResourceView() const;

	protected: 

		Core::SimpleTypeVectorU<ShaderResourceView> m_ShaderResourceViews;

	public:

		const ShaderResourceView& GetShaderResourceView(unsigned index = 0) const;
	
	public: // Unordered access view.

		unsigned CreateUnorderedAccessView(ID3D12Device* device,
			IDescriptorHeap* uavDescHeap,
			DXGI_FORMAT uavFormat);
		unsigned CreateUnorderedAccessView(ID3D12Device* device,
			IDescriptorHeap* uavDescHeap);

		unsigned GetCountUnorderedAccessViews() const;
		bool HasUnorderedAccessView() const;

	protected:

		Core::SimpleTypeVectorU<UnorderedAccessView> m_UnorderedAccessViews;

	public:

		const UnorderedAccessView& GetUnorderedAccessView(unsigned index = 0) const;
	};
}

#endif