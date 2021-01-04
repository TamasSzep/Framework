// DirectX12Render/Resources/DepthStencilBuffer.cpp

#include <DirectX12Render/Resources/DepthStencilBuffer.h>

#include <Core/Comparison.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <DirectX12Render/Resources/DescriptorHeap.h>

using namespace DirectX12Render;

DepthStencilBuffer::DepthStencilBuffer()
{
	m_DSVFormat = DXGI_FORMAT_D32_FLOAT;
	m_ClearValue = c_DefaultDepthStencilBufferClearValue;
}

void DepthStencilBuffer::SetDescription(const Texture2DDescription& description)
{
	m_Description = description;
	if (description.Format == DXGI_FORMAT_UNKNOWN)
	{
		m_Description.Format = DXGI_FORMAT_R32_TYPELESS;
	}
	m_Description.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
}

void DepthStencilBuffer::Initialize(ID3D12Device* device, const Texture2DDescription& description,
	IDescriptorHeap* dsvDescHeap, DXGI_FORMAT dsvFormat, const D3D12_CLEAR_VALUE* pClearValue)
{
	SetDescription(description);
	
	m_Description.Validate();
	
	m_State = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	
	if (dsvFormat != DXGI_FORMAT_UNKNOWN)
	{
		m_DSVFormat = dsvFormat;
	}

	if (pClearValue != nullptr)
	{
		m_ClearValue = *pClearValue;
	}

	CreateResource(device, &m_ClearValue);
	CreateDepthStencilView(device, dsvDescHeap);
}

void DepthStencilBuffer::CreateDepthStencilView(ID3D12Device* device, IDescriptorHeap* rtvDescHeap)
{
	// Getting descriptor heap handles.
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
	rtvDescHeap->RequestHandles(cpuDescriptorHandle, gpuDescriptorHandle);

	// Creating the depth stencil view.
	D3D12_DEPTH_STENCIL_VIEW_DESC viewDescription = {};
	viewDescription.Format = m_DSVFormat;
	viewDescription.ViewDimension = (m_Description.SampleCount == 1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS);
	viewDescription.Flags = D3D12_DSV_FLAG_NONE;
	device->CreateDepthStencilView(m_Resource.Get(), &viewDescription, cpuDescriptorHandle);
}

DXGI_FORMAT DepthStencilBuffer::ResolveTypelessFormat() const
{
	auto format = m_Description.Format;

	if (format == DXGI_FORMAT_R32_TYPELESS)
	{
		return DXGI_FORMAT_R32_FLOAT;
	}

	EngineBuildingBlocks::RaiseException("Typeless format resolving is not implemented for current format.");
	return DXGI_FORMAT_UNKNOWN;
}