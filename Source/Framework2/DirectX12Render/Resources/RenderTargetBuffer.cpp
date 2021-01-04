// DirectX12Render/Resources/RenderTargetBuffer.cpp

#include <DirectX12Render/Resources/RenderTargetBuffer.h>

#include <Core/Comparison.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <DirectX12Render/Resources/DescriptorHeap.h>

using namespace DirectX12Render;

RenderTargetBuffer::RenderTargetBuffer()
{
	m_ClearValue = c_DefaultRenderTargetBufferClearValue;
}

void RenderTargetBuffer::SetDescription(const Texture2DDescription& description)
{
	m_Description = description;
	if (description.Format == DXGI_FORMAT_UNKNOWN)
	{
		m_Description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	m_Description.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
}

D3D12_CLEAR_VALUE RenderTargetBuffer::GetClearValue() const
{
	return m_ClearValue;
}

void RenderTargetBuffer::Initialize(ID3D12Device* device, const Texture2DDescription& description,
	IDescriptorHeap* rtvDescHeap, DXGI_FORMAT rtvFormat, const D3D12_CLEAR_VALUE* pClearValue)
{
	SetDescription(description);

	m_Description.Validate();

	m_State = D3D12_RESOURCE_STATE_RENDER_TARGET;

	if (pClearValue != nullptr)
	{
		m_ClearValue = *pClearValue;
	}

	CreateResource(device, &m_ClearValue);
	CreateRenderTargetView(device, rtvDescHeap, rtvFormat);
}

void RenderTargetBuffer::CreateRenderTargetView(ID3D12Device* device, IDescriptorHeap* rtvDescHeap,
	DXGI_FORMAT rtvFormat)
{
	if (rtvFormat == DXGI_FORMAT_UNKNOWN) rtvFormat = m_Description.Format;

	// Getting descriptor heap handles.
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
	rtvDescHeap->RequestHandles(cpuDescriptorHandle, gpuDescriptorHandle);

	// Creating the depth stencil view.
	D3D12_RENDER_TARGET_VIEW_DESC viewDescription = {};
	viewDescription.Format = rtvFormat;
	if (m_Description.SampleCount == 1)
	{
		viewDescription.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDescription.Texture2D = {};
	}
	else
	{
		viewDescription.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
		viewDescription.Texture2DMS = {};
	}

	device->CreateRenderTargetView(m_Resource.Get(), &viewDescription, cpuDescriptorHandle);
}