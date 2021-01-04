// DirectX12Render/Resources/View.h

#ifndef _DIRECTX12RENDER_VIEW_H_INCLUDED_
#define _DIRECTX12RENDER_VIEW_H_INCLUDED_

#include <DirectX12Render/DirectX12Includes.h>

namespace DirectX12Render
{
	class IDescriptorHeap;

	struct UnorderedAccessView
	{
		IDescriptorHeap* Heap;
		unsigned IndexOnDescriptorHeap;
		CD3DX12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle;
		DXGI_FORMAT Format;
	};
}

#endif