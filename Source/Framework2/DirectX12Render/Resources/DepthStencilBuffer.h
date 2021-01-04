// DirectX12Render/Resources/DepthStencilBuffer.h

#ifndef _DIRECTX12RENDER_DEPTHSTENCILBUFFER_H_INCLUDED_
#define _DIRECTX12RENDER_DEPTHSTENCILBUFFER_H_INCLUDED_

#include <DirectX12Render/Resources/Texture2DBase.h>

namespace DirectX12Render
{
	class DepthStencilBuffer : public Texture2DBase
	{
		DXGI_FORMAT m_DSVFormat;
		D3D12_CLEAR_VALUE m_ClearValue;

		void SetDescription(const Texture2DDescription& description);

	protected:

		DXGI_FORMAT ResolveTypelessFormat() const;

	private: // Depth stencil view.
		
		void CreateDepthStencilView(ID3D12Device* device,
			IDescriptorHeap* rtvDescHeap);

	public:

		DepthStencilBuffer();

		void Initialize(ID3D12Device* device,
			const Texture2DDescription& description,
			IDescriptorHeap* dsvDescHeap,
			DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN,
			const D3D12_CLEAR_VALUE* pClearValue = nullptr);
	};

	const D3D12_CLEAR_VALUE c_DefaultDepthStencilBufferClearValue
		= CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);
}

#endif