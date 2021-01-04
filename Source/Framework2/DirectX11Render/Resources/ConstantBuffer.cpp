// DirectX11Render/Resources/ConstantBuffer.cpp

#include <DirectX11Render/Resources/ConstantBuffer.h>

#include <Core/Utility.hpp>
#include <EngineBuildingBlocks/ErrorHandling.h>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace DirectXRender;
using namespace DirectX11Render;

ConstantBuffer::ConstantBuffer()
	: m_ElementSize(0)
{
}

unsigned ConstantBuffer::GetCPUSizeInBytes() const
{
	return static_cast<unsigned>(m_Data.size());
}

unsigned ConstantBuffer::GetGPUSizeInBytes() const
{
	return m_ElementSize;
}

unsigned ConstantBuffer::GetCountElements() const
{
	return static_cast<unsigned>(m_Data.size() / m_ElementSize);
}

unsigned ConstantBuffer::GetElementSize() const
{
	return m_ElementSize;
}

ID3D11Buffer* ConstantBuffer::GetBuffer()
{
	return m_Buffer.Get();
}

void ConstantBuffer::InitializeResource(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = GetGPUSizeInBytes();
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	auto hr = device->CreateBuffer(&desc, nullptr, &m_Buffer);
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a constant buffer.");
	}
}

void ConstantBuffer::Initialize(ID3D11Device* device, unsigned elementSize, unsigned maxCountElements)
{
	m_ElementSize = Core::AlignSize(elementSize, 16U);
	m_Data.resize(maxCountElements * m_ElementSize);
	InitializeResource(device);
}

void ConstantBuffer::Update(ID3D11DeviceContext* context, unsigned index)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto hr = context->Map(m_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to map a constant buffer.");
	}
	auto mappedPtr = reinterpret_cast<uint8_t*>(mappedResource.pData);
	memcpy(mappedPtr, m_Data.data() + index * m_ElementSize, m_ElementSize);
	context->Unmap(m_Buffer.Get(), 0);
}
