// DirectX11Render/Resources/IndexBuffer.cpp

#include <DirectX11Render/Resources/IndexBuffer.h>

#include <EngineBuildingBlocks/ErrorHandling.h>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace DirectXRender;
using namespace DirectX11Render;

const unsigned c_IndexElementSize = static_cast<unsigned>(sizeof(uint32_t));

IndexBuffer::IndexBuffer()
	: m_CountIndices(0)
{
}

unsigned IndexBuffer::GetSizeInBytes() const
{
	return m_CountIndices * c_IndexElementSize;
}

unsigned IndexBuffer::GetCountIndices() const
{
	return m_CountIndices;
}

PrimitiveTopology IndexBuffer::GetTopology() const
{
	return m_Topology;
}

ID3D11Buffer* IndexBuffer::GetBuffer()
{
	return m_Buffer.Get();
}

void IndexBuffer::InitializeResource(ID3D11Device* device, D3D11_USAGE usage, const unsigned* pIndices)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = GetSizeInBytes();
	desc.Usage = usage;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0
		| (usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0)
		| (usage == D3D11_USAGE_STAGING ? D3D11_CPU_ACCESS_READ : 0);
	desc.MiscFlags = 0;
	desc.StructureByteStride = c_IndexElementSize;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = pIndices;
	auto pInitData = (pIndices == nullptr ? nullptr : &subresourceData);
	auto hr = device->CreateBuffer(&desc, pInitData, &m_Buffer);
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a index buffer.");
	}
}

void IndexBuffer::Initialize(ID3D11Device* device, const IndexData& indexData)
{
	m_CountIndices = indexData.GetCountIndices();
	m_Topology = indexData.Topology;
	InitializeResource(device, D3D11_USAGE_IMMUTABLE, indexData.Data.GetArray());
}

void IndexBuffer::Initialize(ID3D11Device* device, D3D11_USAGE usage, PrimitiveTopology topology,
	unsigned countIndices, const unsigned* pIndices)
{
	m_CountIndices = countIndices;
	m_Topology = topology;
	InitializeResource(device, usage, pIndices);
}

void IndexBuffer::SetData(ID3D11DeviceContext* context, const unsigned* pIndices)
{
	SetData(context, pIndices, 0, m_CountIndices);
}

void IndexBuffer::SetData(ID3D11DeviceContext* context, const unsigned* pIndices,
	unsigned startIndex, unsigned countIndices)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto hr = context->Map(m_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to map an index buffer.");
	}
	memcpy(reinterpret_cast<unsigned*>(mappedResource.pData) + startIndex, pIndices, countIndices * c_IndexElementSize);
	context->Unmap(m_Buffer.Get(), 0);
}
