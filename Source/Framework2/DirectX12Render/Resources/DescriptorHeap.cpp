// DirectX12Render/Resources/DescriptorHeap.cpp

#include <DirectX12Render/Resources/DescriptorHeap.h>

using namespace DirectX12Render;

IDescriptorHeap::~IDescriptorHeap()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

StreamedDescriptorHeap::~StreamedDescriptorHeap()
{
}

void StreamedDescriptorHeap::Initialize(ID3D12Device* device, DescriptorHeapType heapType,
	unsigned maxElementCount)
{
	m_MaxElementCount = maxElementCount;
	m_NextNewElementIndex = 0;

	D3D12_DESCRIPTOR_HEAP_TYPE d3dHeapType;
	D3D12_DESCRIPTOR_HEAP_FLAGS flags;

	switch (heapType)
	{
	case DescriptorHeapType::RTV:
		d3dHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		break;
	case DescriptorHeapType::DSV:
		d3dHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		break;
	case DescriptorHeapType::CBV_SRV_UAV:
		d3dHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		break;
	case DescriptorHeapType::Sampler:
		d3dHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		break;
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDescription = {};
	heapDescription.NumDescriptors = maxElementCount;
	heapDescription.Flags = flags;
	heapDescription.Type = d3dHeapType;
	auto hr = device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&m_Heap));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a descriptor heap.");
	}

	m_StartCPUDescriptorHandle = m_Heap->GetCPUDescriptorHandleForHeapStart();
	m_StartGPUDescriptorHandle = m_Heap->GetGPUDescriptorHandleForHeapStart();

	m_DescriptorSize = device->GetDescriptorHandleIncrementSize(d3dHeapType);
}

unsigned StreamedDescriptorHeap::RequestHandles(
	CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuDescriptorHandle,
	CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuDescriptorHandle)
{
	unsigned index;
	if (m_UnusedIndices.GetSize() == 0)
	{
		index = m_NextNewElementIndex++;
	}
	else
	{
		index = m_UnusedIndices.PopBackReturn();
	}

	cpuDescriptorHandle.InitOffsetted(m_StartCPUDescriptorHandle, index, m_DescriptorSize);
	gpuDescriptorHandle.InitOffsetted(m_StartGPUDescriptorHandle, index, m_DescriptorSize);

	return index;
}

void StreamedDescriptorHeap::ReleaseHandles(unsigned index)
{
	m_UnusedIndices.PushBack(index);
}

ID3D12DescriptorHeap* StreamedDescriptorHeap::GetHeap()
{
	return m_Heap.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE StreamedDescriptorHeap::GetStartCPUDescriptorHandle()
{
	return m_StartCPUDescriptorHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE StreamedDescriptorHeap::GetStartGPUDescriptorHandle()
{
	return m_StartGPUDescriptorHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE StreamedDescriptorHeap::GetCPUDescriptorHandle(unsigned index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(handle, m_StartCPUDescriptorHandle, index, m_DescriptorSize);
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE StreamedDescriptorHeap::GetGPUDescriptorHandle(unsigned index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE::InitOffsetted(handle, m_StartGPUDescriptorHandle, index, m_DescriptorSize);
	return handle;
}