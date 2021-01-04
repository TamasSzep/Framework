// DirectX12Render/Resources/IndexBuffer.cpp

#include <DirectX12Render/Resources/IndexBuffer.h>

#include <Core/Utility.hpp>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <WindowsApplication/DirectX/DirectX.h>
#include <DirectX12Render/Resources/TransferBuffer.h>

using namespace DirectXRender;
using namespace DirectX12Render;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

IndexBuffer::IndexBuffer()
	: m_CountIndices(0)
{
}

const D3D12_INDEX_BUFFER_VIEW& IndexBuffer::GetIndexBufferView() const
{
	return m_IndexBufferView;
}

unsigned IndexBuffer::GetCountIndices() const
{
	return m_CountIndices;
}

unsigned IndexBuffer::GetCPUSize() const
{
	return m_CountIndices * sizeof(unsigned);
}

unsigned IndexBuffer::GetGPUSize() const
{
	return Core::AlignSize(GetCPUSize(), 16U);
}

D3D12_PRIMITIVE_TOPOLOGY IndexBuffer::GetTopology() const
{
	return m_Topology;
}

void IndexBuffer::CreateResource(ID3D12Device* device, D3D12_HEAP_TYPE heapType)
{
	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(GetGPUSize()),
		D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_INDEX_BUFFER*/,
		nullptr,
		IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create an index buffer.");
	}
}

void IndexBuffer::InitializeView(ID3D12Device* device)
{
	m_IndexBufferView.BufferLocation = m_Resource->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView.SizeInBytes = GetCPUSize();
}

void IndexBuffer::SetData(TransferBufferManager* transferBufferManager,
	ID3D12GraphicsCommandList* commandList, ID3D12Device* device,
	const EngineBuildingBlocks::Graphics::IndexData& indexData)
{
	assert(c_PrimitiveTopologyMap[(unsigned char)indexData.Topology] == m_Topology);

	unsigned cpuSize = GetCPUSize();
	unsigned gpuSize = GetGPUSize();

	UINT64 requiredSize = 0;
	device->GetCopyableFootprints(&CD3DX12_RESOURCE_DESC::Buffer(gpuSize),
		0, 1, 0, nullptr, nullptr, nullptr, &requiredSize);

	UploadBufferDescription uploadBufferDescription;
	uploadBufferDescription.Size = requiredSize;
	auto uploadBuffer = transferBufferManager->RequestUploadBuffer(device, uploadBufferDescription);

	D3D12_SUBRESOURCE_DATA subresourceData;
	subresourceData.pData = indexData.Data.GetArray();
	subresourceData.RowPitch = cpuSize;
	subresourceData.SlicePitch = cpuSize;
	UpdateSubresources(commandList, m_Resource.Get(), uploadBuffer->GetResource(), 0, 0, 1, &subresourceData);
}

void IndexBuffer::Initialize(TransferBufferManager* transferBufferManager,
	ID3D12GraphicsCommandList* commandList, ID3D12Device* device,
	const EngineBuildingBlocks::Graphics::IndexData& indexData)
{
	m_Topology = c_PrimitiveTopologyMap[(unsigned char)indexData.Topology];
	m_CountIndices = indexData.Data.GetSize();
	CreateResource(device, D3D12_HEAP_TYPE_DEFAULT);
	InitializeView(device);
	SetData(transferBufferManager, commandList, device, indexData);
}

void IndexBuffer::Initialize(ID3D12Device* device, D3D12_HEAP_TYPE heapType,
	unsigned countIndices, EngineBuildingBlocks::Graphics::PrimitiveTopology topology)
{
	m_Topology = c_PrimitiveTopologyMap[(unsigned char)topology];
	m_CountIndices = countIndices;
	CreateResource(device, heapType);
	InitializeView(device);
}