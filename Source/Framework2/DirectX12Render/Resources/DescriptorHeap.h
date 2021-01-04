// DirectX12Render/Resources/DescriptorHeap.h

#ifndef _DIRECTX12RENDER_DESCRIPTORHEAP_H_INCLUDED_
#define _DIRECTX12RENDER_DESCRIPTORHEAP_H_INCLUDED_

#include <Core/DataStructures/ResourceUnorderedVector.hpp>
#include <DirectX12Render/DirectX12Includes.h>

namespace DirectX12Render
{
	class IDescriptorHeap
	{
	public:

		virtual ~IDescriptorHeap();

		virtual unsigned RequestHandles(
			CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuDescriptorHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuDescriptorHandle) = 0;

		virtual void ReleaseHandles(unsigned index) = 0;
	};

	enum class DescriptorHeapType
	{
		RTV,
		DSV,
		CBV_SRV_UAV,
		Sampler
	};

	class StreamedDescriptorHeap : public IDescriptorHeap
	{
		unsigned m_MaxElementCount;
		unsigned m_NextNewElementIndex;
		Core::IndexVectorU m_UnusedIndices;

		unsigned m_DescriptorSize;

		ComPtr<ID3D12DescriptorHeap> m_Heap;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_StartCPUDescriptorHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_StartGPUDescriptorHandle;

	public:

		~StreamedDescriptorHeap();

		void Initialize(ID3D12Device* device, DescriptorHeapType heapType,
			unsigned maxElementCount);

		unsigned RequestHandles(
			CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuDescriptorHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuDescriptorHandle);

		void ReleaseHandles(unsigned index);

		ID3D12DescriptorHeap* GetHeap();

		D3D12_CPU_DESCRIPTOR_HANDLE GetStartCPUDescriptorHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE GetStartGPUDescriptorHandle();

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(unsigned index);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(unsigned index);
	};
}

#endif