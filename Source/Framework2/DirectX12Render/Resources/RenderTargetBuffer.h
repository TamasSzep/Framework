// DirectX12Render/Resources/RenderTargetBuffer.h

#ifndef _DIRECTX12RENDER_RENDERTARGETBUFFER_H_INCLUDED_
#define _DIRECTX12RENDER_RENDERTARGETBUFFER_H_INCLUDED_

#include <DirectX12Render/Resources/Texture2DBase.h>

namespace DirectX12Render
{
	class RenderTargetBuffer : public Texture2DBase
	{
		D3D12_CLEAR_VALUE m_ClearValue;

		void SetDescription(const Texture2DDescription& description);

	private: // Render target view.

		void CreateRenderTargetView(ID3D12Device* device,
			IDescriptorHeap* rtvDescHeap,
			DXGI_FORMAT rtvFormat);

	public:

		RenderTargetBuffer();

		D3D12_CLEAR_VALUE GetClearValue() const;

		void Initialize(ID3D12Device* device,
			const Texture2DDescription& description,
			IDescriptorHeap* rtvDescHeap,
			DXGI_FORMAT rtvFormat = DXGI_FORMAT_UNKNOWN,
			const D3D12_CLEAR_VALUE* pClearValue = nullptr);
	};

	const float c_DefaultRenderTargetClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const D3D12_CLEAR_VALUE c_DefaultRenderTargetBufferClearValue
		= CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, c_DefaultRenderTargetClearColor);
}

#endif