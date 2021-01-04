// DirectX12Render/Resources/TransferBuffer.h

#include <DirectX12Render/Resources/TransferBuffer.h>

#include <Core/Utility.hpp>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <WindowsApplication/DirectX/Format.h>

using namespace DirectXRender;
using namespace DirectX12Render;

UploadBufferDescription::UploadBufferDescription()
	: Size(0)
	, Flags(D3D12_RESOURCE_FLAG_NONE)
	, Alignment(0)
{
}

CD3DX12_RESOURCE_DESC UploadBufferDescription::ToResourceDescription() const
{
	return CD3DX12_RESOURCE_DESC::Buffer(Size, Flags, Alignment);
}

bool UploadBufferDescription::IsSufficient(const UploadBufferDescription& other)
{
	return (Size >= other.Size && Flags == other.Flags && Alignment == other.Alignment);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ID3D12Resource* TransferBuffer::GetResource()
{
	return m_Resource.Get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void UploadBuffer::Initialize(ID3D12Device* device, const UploadBufferDescription& description)
{
	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&description.ToResourceDescription(),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create an upload buffer.");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void ReadbackBuffer::Initialize(ID3D12Device* device, const Texture2DDescription& description)
{
	auto bufferSize = Core::AlignSize<unsigned>(
		description.Width * GetSizeInBytes(description.Format),
		D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
		* description.Height * description.ArraySize;	
	auto d3dDescription = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE, 0);

	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&d3dDescription,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create an readback buffer.");
	}
}

void ReadbackBuffer::GetData(unsigned char* buffer, unsigned size)
{
	unsigned char* pDataBegin;
	D3D12_RANGE readRange;
	readRange.Begin = 0;
	readRange.End = size;
	auto hr = m_Resource->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to map texture for reading.");
	}
	memcpy(buffer, pDataBegin, size);
	const D3D12_RANGE emptyRange = {};
	m_Resource->Unmap(0, &emptyRange);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

TransferBufferManager::UploadBufferData& TransferBufferManager::_CreateUploadBuffer(ID3D12Device* device,
	const UploadBufferDescription& description)
{
	auto uploadBuffer = m_UploadBuffers.Request();
	uploadBuffer->Initialize(device, description);
	m_UploadBufferData.emplace_back(UploadBufferData{ description, uploadBuffer, false });
	return m_UploadBufferData.back();
}

TransferBufferManager::ReadbackBufferData& TransferBufferManager::_CreateReadbackBuffer(ID3D12Device* device,
	const Texture2DDescription& description)
{
	auto readbackBuffer = m_ReadbackBuffers.Request();
	readbackBuffer->Initialize(device, description);
	m_ReadbackBufferData.emplace_back(ReadbackBufferData{ description, readbackBuffer, false });
	return m_ReadbackBufferData.back();
}

void TransferBufferManager::CreateUploadBuffer(ID3D12Device* device, const UploadBufferDescription& description)
{
	_CreateUploadBuffer(device, description);
}

void TransferBufferManager::CreateReadbackBuffer(ID3D12Device* device, const Texture2DDescription& description)
{
	_CreateReadbackBuffer(device, description);
}

UploadBuffer* TransferBufferManager::RequestUploadBuffer(ID3D12Device* device,
	const UploadBufferDescription& description)
{
	for (auto& it : m_UploadBufferData)
	{
		if (!it.IsUsed && it.Description.IsSufficient(description))
		{
			it.IsUsed = true;
			return it.UploadBuffer;
		}
	}

	// Creating an update buffer on demand.
	auto& data = _CreateUploadBuffer(device, description);
	data.IsUsed = true;
	return data.UploadBuffer;
}

ReadbackBuffer* TransferBufferManager::RequestReadbackBuffer(ID3D12Device* device,
	const Texture2DDescription& description)
{
	for (auto& it : m_ReadbackBufferData)
	{
		if (!it.IsUsed && description.IsCopyableTo(it.Description))
		{
			it.IsUsed = true;
			return it.ReadbackBuffer;
		}
	}

	// Creating a readback buffer on demand.
	auto& data = _CreateReadbackBuffer(device, description);
	data.IsUsed = true;
	return data.ReadbackBuffer;
}

void TransferBufferManager::ReleaseUploadBuffer(UploadBuffer* uploadBuffer)
{
	for (auto& it : m_UploadBufferData)
	{
		if (it.UploadBuffer == uploadBuffer)
		{
			assert(it.IsUsed);

			it.IsUsed = false;
			return;
		}
	}
	assert(false);
}

void TransferBufferManager::ReleaseReadbackBuffer(ReadbackBuffer* readbackBuffer)
{
	for (auto& it : m_ReadbackBufferData)
	{
		if (it.ReadbackBuffer == readbackBuffer)
		{
			assert(it.IsUsed);

			it.IsUsed = false;
			return;
		}
	}
	assert(false);
}

void TransferBufferManager::ReleaseAllUploadBuffers()
{
	for (auto& it : m_UploadBufferData)
	{
		if (it.IsUsed)
		{
			it.IsUsed = false;
		}
	}
}

void TransferBufferManager::ReleaseAllReadbackBuffers()
{
	for (auto& it : m_ReadbackBufferData)
	{
		if (it.IsUsed)
		{
			it.IsUsed = false;
		}
	}
}