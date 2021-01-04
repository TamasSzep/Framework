// DirectX12Render/Resources/VertexBuffer.cpp

#include <DirectX12Render/Resources/VertexBuffer.h>

#include <Core/Comparison.h>
#include <Core/Utility.hpp>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <WindowsApplication/DirectX/VertexSemantics.h>
#include <DirectX12Render/Resources/TransferBuffer.h>

#include <cassert>

using namespace DirectXRender;
using namespace DirectX12Render;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void VertexInputLayout::Initialize(
	const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
	unsigned inputSlot, bool isPerVertex)
{
	auto countElements = static_cast<unsigned>(inputLayout.Elements.size());
	Core::IndexVectorU inputSlots(countElements, inputSlot);
	Core::SimpleTypeVectorU<bool> isPerVertexArray(countElements, isPerVertex);
	Initialize(inputLayout, inputSlots.GetArray(), isPerVertexArray.GetArray());
}

void VertexInputLayout::Initialize(
	const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
	unsigned* inputSlots, bool* isPerVertexArray)
{
	this->InputLayout = inputLayout;

	auto& elements = inputLayout.Elements;
	unsigned countElements = static_cast<unsigned>(elements.size());
	InputLayoutElements.Resize(countElements);
	std::map<std::string, unsigned> semanticIndices;
	for (unsigned i = 0; i < countElements; i++)
	{
		auto& data = elements[i];
		auto& element = InputLayoutElements[i];
		bool isPerVertex = isPerVertexArray[i];
		element.SemanticName = GetD3DSemanticName(data.Name);
		element.SemanticIndex = GetD3DVertexElementSemanticIndex(element.SemanticName, semanticIndices);
		element.Format = ToDXGIFormat(data.Type, data.TypeSize, data.Count);
		element.InputSlot = inputSlots[i];
		element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		element.InputSlotClass = (isPerVertex
			? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
			: D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA);
		element.InstanceDataStepRate = (isPerVertex ? 0 : 1);
	}
	InputLayoutDescription.pInputElementDescs = InputLayoutElements.GetArray();
	InputLayoutDescription.NumElements = countElements;
}

bool VertexInputLayout::operator==(const VertexInputLayout& other) const
{
	StructureEqualCompareBlock(InputLayout);
	unsigned countElements = InputLayoutElements.GetSize();
	for (unsigned i = 0; i < countElements; i++)
	{
		NumericalEqualCompareBlock(InputLayoutElements[i].InputSlot);
		NumericalEqualCompareBlock(InputLayoutElements[i].InputSlotClass);
		NumericalEqualCompareBlock(InputLayoutElements[i].InstanceDataStepRate);
	}
	return true;
}

bool VertexInputLayout::operator!=(const VertexInputLayout& other) const
{
	return !(*this == other);
}

bool VertexInputLayout::operator<(const VertexInputLayout& other) const
{
	StructureLessCompareBlock(InputLayout);
	unsigned countElements = InputLayoutElements.GetSize();
	for (unsigned i = 0; i < countElements; i++)
	{
		NumericalLessCompareBlock(InputLayoutElements[i].InputSlot);
		NumericalLessCompareBlock(InputLayoutElements[i].InputSlotClass);
		NumericalLessCompareBlock(InputLayoutElements[i].InstanceDataStepRate);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

VertexBuffer::VertexBuffer()
{
}

unsigned VertexBuffer::GetCountVertices() const
{
	return m_CountVertices;
}

unsigned VertexBuffer::GetCPUSize() const
{
	return m_CPUSize;
}

unsigned VertexBuffer::AlignForVertexBuffer(unsigned size)
{
	return Core::AlignSize(size, 16U);
}

unsigned VertexBuffer::GetGPUSize() const
{
	return AlignForVertexBuffer(m_CPUSize);
}

const D3D12_VERTEX_BUFFER_VIEW& VertexBuffer::GetVertexBufferView() const
{
	return m_VertexBufferView;
}

const VertexInputLayout& VertexBuffer::GetInputLayout() const
{
	return m_InputLayout;
}

void VertexBuffer::InitializeResource(ID3D12Device* device, D3D12_HEAP_TYPE heapType)
{
	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(heapType),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(GetGPUSize()),
		D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER*/,
		nullptr,
		IID_PPV_ARGS(&m_Resource));

	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a vertex buffer.");
	}
}

void VertexBuffer::InitializeVertexBufferView()
{
	m_VertexBufferView.BufferLocation = m_Resource->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = m_InputLayout.InputLayout.GetVertexStride();
	m_VertexBufferView.SizeInBytes = m_CPUSize;
}

void VertexBuffer::UploadData(TransferBufferManager* transferBufferManager,
	ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const unsigned char* pData, unsigned uploadCPUSize)
{
	unsigned uploadGPUSize = AlignForVertexBuffer(uploadCPUSize);

	// Getting an upload buffer and using it in order to upload the data.

	UINT64 requiredSize = 0;
	device->GetCopyableFootprints(&CD3DX12_RESOURCE_DESC::Buffer(uploadGPUSize),
		0, 1, 0, nullptr, nullptr, nullptr, &requiredSize);

	UploadBufferDescription uploadBufferDescription;
	uploadBufferDescription.Size = requiredSize;
	auto uploadBuffer = transferBufferManager->RequestUploadBuffer(device, uploadBufferDescription);

	D3D12_SUBRESOURCE_DATA subresourceData;
	subresourceData.pData = pData;
	subresourceData.RowPitch = uploadCPUSize;
	subresourceData.SlicePitch = uploadCPUSize;
	UpdateSubresources(commandList, m_Resource.Get(), uploadBuffer->GetResource(), 0, 0, 1, &subresourceData);
}

void VertexBuffer::Initialize(TransferBufferManager* transferBufferManager,
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	const EngineBuildingBlocks::Graphics::Vertex_AOS_Data& aosData,
	unsigned inputSlot, bool isPerVertex)
{
	m_CountVertices = aosData.GetCountVertices();
	m_CPUSize = aosData.GetSize();
	InitializeResource(device, D3D12_HEAP_TYPE_DEFAULT);
	m_InputLayout.Initialize(aosData.InputLayout, inputSlot, isPerVertex);
	InitializeVertexBufferView();
	UploadData(transferBufferManager, device, commandList, aosData.Data.GetArray(), aosData.GetSize());
}

void VertexBuffer::Initialize(TransferBufferManager* transferBufferManager,
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& soaData,
	unsigned inputSlot, bool isPerVertex)
{
	m_CountVertices = soaData.GetCountVertices();
	m_CPUSize = soaData.GetSize();
	InitializeResource(device, D3D12_HEAP_TYPE_DEFAULT);
	m_InputLayout.Initialize(soaData.InputLayout, inputSlot, isPerVertex);
	InitializeVertexBufferView();

	// Converting to AOS data.
	Core::ByteVectorU data;
	soaData.As_AOS_Data(data);
	UploadData(transferBufferManager, device, commandList, data.GetArray(), data.GetSize());
}

void VertexBuffer::Initialize(ID3D12Device* device,
	const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout, D3D12_HEAP_TYPE heapType,
	unsigned countVertices,  unsigned inputSlot, bool isPerVertex)
{
	m_CountVertices = countVertices;
	m_CPUSize = inputLayout.GetVertexStride() * countVertices;
	InitializeResource(device, heapType);
	m_InputLayout.Initialize(inputLayout, inputSlot, isPerVertex);
	InitializeVertexBufferView();
}

void VertexBuffer::SetData(unsigned char* data, size_t size)
{
	D3D12_RANGE readRange = { 0, 0 };
	D3D12_RANGE writeRange = { 0, size };

	unsigned char* target;
	auto hr = m_Resource->Map(0, &readRange, reinterpret_cast<void**>(&target));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to map the vertex buffer.");
	}
	memcpy(target, data, size);
	m_Resource->Unmap(0, &writeRange);
}

void VertexBuffer::SetData(TransferBufferManager* transferBufferManager,
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	unsigned char* data, unsigned size)
{
	UploadData(transferBufferManager, device, commandList, data, size);
}