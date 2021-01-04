// DirectX12Render/Resources/ConstantBuffer.cpp

#include <DirectX12Render/Resources/ConstantBuffer.h>

#include <Core/Utility.hpp>
#include <DirectX12Render/Resources/DescriptorHeap.h>

using namespace DirectX12Render;

ConstantBuffer::ConstantBuffer(
	ConstantBufferMappingType mappingType, unsigned dataSize, unsigned maximumViews)
	: m_MappingType(mappingType)
	, m_DataSize(dataSize)
	, m_MaximumViews(maximumViews)
	, m_CountViews(0)
	, m_WrittenRange({0, 0})
{
	m_AlignedDataSize = Core::AlignSize(dataSize, 256U);
}

ConstantBuffer::~ConstantBuffer()
{
	if (m_MappingType == ConstantBufferMappingType::AlwaysMapped && m_Resource)
	{
		Unmap();
	}
}

unsigned ConstantBuffer::GetSize() const
{
	return m_MaximumViews * m_AlignedDataSize;
}

void ConstantBuffer::Initialize(ID3D12Device* device)
{
	m_DescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(GetSize()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a constant buffer.");
	}

	if (m_MappingType == ConstantBufferMappingType::AlwaysMapped)
	{
		Map();
	}
}

void ConstantBuffer::Map()
{
	CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	auto hr = m_Resource->Map(0, &readRange, reinterpret_cast<void**>(&m_DirectDataPointer));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to map a constant buffer resource.");
	}
}

void ConstantBuffer::Unmap()
{
	m_Resource->Unmap(0, &m_WrittenRange);
}

void ConstantBuffer::StartWrite()
{
	if (m_MappingType == ConstantBufferMappingType::OnWriteMapped)
	{
		Map();
	}

	m_WrittenRange.Begin = 0;
	m_WrittenRange.End = static_cast<size_t>(m_CountViews * m_AlignedDataSize);
}

void ConstantBuffer::EndWrite()
{
	if (m_MappingType == ConstantBufferMappingType::OnWriteMapped)
	{
		Unmap();
	}
}

void ConstantBuffer::IndicateWrittenRangeWithViewIndex(unsigned startIndex, unsigned endIndex)
{
	m_WrittenRange.Begin = static_cast<size_t>(startIndex * m_AlignedDataSize);
	m_WrittenRange.End = static_cast<size_t>(endIndex * m_AlignedDataSize);
}

unsigned ConstantBuffer::AddConstantBuffer()
{
	assert(m_CountViews < m_MaximumViews);

	return m_CountViews++;
}

unsigned ConstantBuffer::AddConstantBuffer(ID3D12Device* device,
	IDescriptorHeap* pDescriptorHeap)
{
	assert(m_CountViews < m_MaximumViews);

	CreateConstantBufferView(device, pDescriptorHeap);

	return m_CountViews++;
}

void ConstantBuffer::CreateConstantBufferView(ID3D12Device* device,
	IDescriptorHeap* pDescriptorHeap)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
	pDescriptorHeap->RequestHandles(cpuDescriptorHandle, gpuDescriptorHandle);

	D3D12_CONSTANT_BUFFER_VIEW_DESC description = {};
	description.BufferLocation = m_Resource->GetGPUVirtualAddress() + m_CountViews * m_AlignedDataSize;
	description.SizeInBytes = m_AlignedDataSize;
	device->CreateConstantBufferView(&description, cpuDescriptorHandle);

	m_GPUDescriptorHandles.PushBack(gpuDescriptorHandle);
}

const D3D12_GPU_DESCRIPTOR_HANDLE& ConstantBuffer::GetConstantBufferViewDescriptorHandle(unsigned index) const
{
	return m_GPUDescriptorHandles[index];
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetConstantBufferGPUVirtualAddress(unsigned index)
{
	return m_Resource->GetGPUVirtualAddress() + index * m_AlignedDataSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////