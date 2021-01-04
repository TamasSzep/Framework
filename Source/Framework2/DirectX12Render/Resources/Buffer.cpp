// DirectX12Render/Resources/Buffer.cpp

#include <DirectX12Render/Resources/Buffer.h>

#include <EngineBuildingBlocks/ErrorHandling.h>
#include <DirectX12Render/Resources/DescriptorHeap.h>

using namespace DirectX12Render;

BufferDescription::BufferDescription()
	: Size(0)
	, HeapType(D3D12_HEAP_TYPE_DEFAULT)
	, HeapFlags(D3D12_HEAP_FLAG_NONE)
	, ResourceFlags(D3D12_RESOURCE_FLAG_NONE)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Buffer::Buffer()
{
}

const BufferDescription& Buffer::GetDescription() const
{
	return m_Description;
}

unsigned Buffer::GetSize() const
{
	return m_Description.Size;
}

ID3D12Resource* Buffer::GetResource()
{
	return m_Resource.Get();
}

void Buffer::CreateResource(ID3D12Device* device, D3D12_RESOURCE_STATES startState)
{
	m_State = startState;

	if (m_Description.HeapType == D3D12_HEAP_TYPE_UPLOAD)
		startState = D3D12_RESOURCE_STATE_GENERIC_READ;
	else if (m_Description.HeapType == D3D12_HEAP_TYPE_READBACK)
		startState = D3D12_RESOURCE_STATE_COPY_DEST;

	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(m_Description.HeapType),
		m_Description.HeapFlags,
		&CD3DX12_RESOURCE_DESC::Buffer(m_Description.Size, m_Description.ResourceFlags),
		startState,
		nullptr,
		IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a buffer.");
	}
}

void Buffer::IndicateTransitioned(D3D12_RESOURCE_STATES targetState)
{
	m_State = targetState;
}

void Buffer::Initialize(ID3D12Device* device, const BufferDescription& description,
	D3D12_RESOURCE_STATES startState)
{
	this->m_Description = description;
	CreateResource(device, startState);
}

D3D12_RESOURCE_STATES Buffer::GetState()
{
	return m_State;
}

D3D12_RESOURCE_STATES Buffer::Transition(ID3D12GraphicsCommandList* commandList,
	D3D12_RESOURCE_STATES targetState)
{
	auto oldState = m_State;
	if (m_State != targetState)
	{
		commandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), m_State, targetState));

		m_State = targetState;
	}
	return oldState;
}

D3D12_RESOURCE_STATES Buffer::TransitionToUAV(ID3D12GraphicsCommandList* commandList)
{
	return Transition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void* Buffer::MapForReading(unsigned start, unsigned end)
{
	D3D12_RANGE readRange;
	readRange.Begin = start;
	readRange.End = (end == 0 ? m_Description.Size : end);
	void* mappedPtr;
	m_Resource->Map(0, &readRange, &mappedPtr);
	return mappedPtr;
}

void* Buffer::MapForWriting()
{
	D3D12_RANGE readRange;
	readRange.Begin = 0;
	readRange.End = 0;
	void* mappedPtr;
	auto hr = m_Resource->Map(0, &readRange, &mappedPtr);
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to map a buffer.");
	}
	return mappedPtr;
}

void Buffer::UnmapAfterReading()
{
	D3D12_RANGE writtenRange;
	writtenRange.Begin = 0;
	writtenRange.End = 0;
	m_Resource->Unmap(0, &writtenRange);
}

void Buffer::UnmapAfterWriting(unsigned start, unsigned end)
{
	D3D12_RANGE writtenRange;
	writtenRange.Begin = start;
	writtenRange.End = (end == 0 ? m_Description.Size : end);
	m_Resource->Unmap(0, &writtenRange);
}

unsigned Buffer::CreateRawUnorderedAccessView(ID3D12Device* device, IDescriptorHeap* uavDescHeap)
{
	if ((m_Description.ResourceFlags | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
		EngineBuildingBlocks::RaiseException(
			"Unordered access flag is not set on the buffer: cannot create UAV.");

	auto uavFormat = DXGI_FORMAT_R32_TYPELESS; // This is the only valid value for raw unordered access views.

	auto uavIndex = m_UnorderedAccessViews.GetSize();
	auto& uav = m_UnorderedAccessViews.PushBackPlaceHolder();

	uav.Heap = uavDescHeap;
	uav.Format = uavFormat;

	// Getting descriptor handles on the heap.
	uav.IndexOnDescriptorHeap
		= uavDescHeap->RequestHandles(uav.CPUDescriptorHandle, uav.GPUDescriptorHandle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = uavFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = m_Description.Size / sizeof(unsigned);
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	device->CreateUnorderedAccessView(m_Resource.Get(), nullptr, &uavDesc, uav.CPUDescriptorHandle);

	return uavIndex;
}

const Core::SimpleTypeVectorU<UnorderedAccessView>& Buffer::GetUnorderedAccessViews() const
{
	return m_UnorderedAccessViews;
}