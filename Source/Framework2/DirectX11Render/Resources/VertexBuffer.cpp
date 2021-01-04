// DirectX11Render/Resources/VertexBuffer.cpp

#include <DirectX11Render/Resources/VertexBuffer.h>

#include <Core/Comparison.h>
#include <Core/Utility.hpp>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <WindowsApplication/DirectX/VertexSemantics.h>
#include <WindowsApplication/DirectX/Format.h>

#include <cassert>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace DirectXRender;
using namespace DirectX11Render;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

DirectX11Render::VertexInputLayout DirectX11Render::VertexInputLayout::Create(
	const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
	const unsigned inputSlot, const bool isPerVertexArray)
{
	auto countElements = static_cast<unsigned>(inputLayout.Elements.size());
	return Create(inputLayout,
		Core::IndexVectorU(countElements, inputSlot).GetArray(),
		Core::SimpleTypeVectorU<bool>(countElements, isPerVertexArray).GetArray());
}

DirectX11Render::VertexInputLayout DirectX11Render::VertexInputLayout::Create(
	const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
	const unsigned* inputSlots, const bool* isPerVertexArray)
{
	auto& elements = inputLayout.Elements;
	auto countElements = static_cast<unsigned>(elements.size());

	VertexInputLayout resLayout;
	resLayout.InputLayout = inputLayout;

	resLayout.Elements.Resize(countElements);
	std::map<std::string, unsigned> semanticIndices;
	for (unsigned i = 0; i < countElements; i++)
	{
		auto& element = elements[i];
		auto& resElement = resLayout.Elements[i];
		bool isPerVertex = isPerVertexArray[i];
		resElement.SemanticName = GetD3DSemanticName(element.Name);
		resElement.SemanticIndex = GetD3DVertexElementSemanticIndex(resElement.SemanticName, semanticIndices);
		resElement.Format = ToDXGIFormat(element.Type, element.TypeSize, element.Count);
		resElement.InputSlot = inputSlots[i];
		resElement.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		resElement.InputSlotClass = (isPerVertex
			? D3D11_INPUT_PER_VERTEX_DATA
			: D3D11_INPUT_PER_INSTANCE_DATA);
		resElement.InstanceDataStepRate = (isPerVertex ? 0 : 1);
	}

	return resLayout;
}

bool DirectX11Render::VertexInputLayout::operator==(const VertexInputLayout& other) const
{
	StructureEqualCompareBlock(InputLayout);
	unsigned countElements = Elements.GetSize();
	for (unsigned i = 0; i < countElements; i++)
	{
		NumericalEqualCompareBlock(Elements[i].InputSlot);
		NumericalEqualCompareBlock(Elements[i].InputSlotClass);
		NumericalEqualCompareBlock(Elements[i].InstanceDataStepRate);
	}
	return true;
}

bool DirectX11Render::VertexInputLayout::operator!=(const VertexInputLayout& other) const
{
	return !(*this == other);
}

bool DirectX11Render::VertexInputLayout::operator<(const VertexInputLayout& other) const
{
	StructureLessCompareBlock(InputLayout);
	unsigned countElements = Elements.GetSize();
	for (unsigned i = 0; i < countElements; i++)
	{
		NumericalLessCompareBlock(Elements[i].InputSlot);
		NumericalLessCompareBlock(Elements[i].InputSlotClass);
		NumericalLessCompareBlock(Elements[i].InstanceDataStepRate);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

VertexBuffer::VertexBuffer()
	: m_CountVertices(0)
{
}

unsigned VertexBuffer::GetSizeInBytes() const
{
	return m_CountVertices * GetVertexStride();
}

unsigned VertexBuffer::GetCountVertices() const
{
	return m_CountVertices;
}

unsigned VertexBuffer::GetVertexStride() const
{
	return m_InputLayout.GetVertexStride();
}

const EngineBuildingBlocks::Graphics::VertexInputLayout& VertexBuffer::GetInputLayout() const
{
	return m_InputLayout;
}

ID3D11Buffer* VertexBuffer::GetBuffer()
{
	return m_Buffer.Get();
}

void VertexBuffer::InitializeResource(ID3D11Device* device, D3D11_USAGE usage, const void* pData)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = GetSizeInBytes();
	desc.Usage = usage;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0
		| (usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0)
		| (usage == D3D11_USAGE_STAGING ? D3D11_CPU_ACCESS_READ : 0);
	desc.MiscFlags = 0;
	desc.StructureByteStride = GetVertexStride();

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = pData;
	auto pInitData = (pData == nullptr ? nullptr : &subresourceData);
	auto hr = device->CreateBuffer(&desc, pInitData, &m_Buffer);
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a vertex buffer.");
	}
}

void VertexBuffer::Initialize(ID3D11Device* device, const EngineBuildingBlocks::Graphics::Vertex_AOS_Data& aosData)
{
	m_CountVertices = aosData.GetCountVertices();
	m_InputLayout = aosData.InputLayout;
	InitializeResource(device, D3D11_USAGE_IMMUTABLE, aosData.Data.GetArray());
}

void VertexBuffer::Initialize(ID3D11Device* device, const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& soaData)
{
	m_CountVertices = soaData.GetCountVertices();
	m_InputLayout = soaData.InputLayout;
	InitializeResource(device, D3D11_USAGE_IMMUTABLE, soaData.As_AOS_Data().Data.GetArray());
}

void VertexBuffer::Initialize(ID3D11Device* device, D3D11_USAGE usage,
	const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout, unsigned countVertices)
{
	m_CountVertices = countVertices;
	m_InputLayout = inputLayout;
	InitializeResource(device, usage, nullptr);
}

void VertexBuffer::SetData(ID3D11DeviceContext* context, const void* pData, size_t size)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto hr = context->Map(m_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to map a vertex buffer.");
	}
	memcpy(mappedResource.pData, pData, size);
	context->Unmap(m_Buffer.Get(), 0);
}