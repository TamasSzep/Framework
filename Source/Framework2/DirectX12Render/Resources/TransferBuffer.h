// DirectX12Render/Resources/TransferBuffer.h

#ifndef _DIRECTX12RENDER_TRANSFERBUFFER_H_INCLUDED_
#define _DIRECTX12RENDER_TRANSFERBUFFER_H_INCLUDED_

#include <Core/DataStructures/Pool.hpp>
#include <DirectX12Render/Resources/Texture2DBase.h>

#include <vector>
#include <utility>

namespace DirectX12Render
{
	struct UploadBufferDescription
	{
		UINT64 Size;
		D3D12_RESOURCE_FLAGS Flags;
		UINT64 Alignment;

		UploadBufferDescription();

		CD3DX12_RESOURCE_DESC ToResourceDescription() const;

		bool IsSufficient(const UploadBufferDescription& other);
	};

	class TransferBuffer
	{
	protected:

		ComPtr<ID3D12Resource> m_Resource;

	public:

		ID3D12Resource* GetResource();
	};

	class UploadBuffer : public TransferBuffer
	{
	public:

		void Initialize(ID3D12Device* device,
			const UploadBufferDescription& description);
	};

	class ReadbackBuffer : public TransferBuffer
	{
	public:

		void Initialize(ID3D12Device* device,
			const Texture2DDescription& description);
		void GetData(unsigned char* buffer, unsigned size);
	};

	class TransferBufferManager
	{
		Core::ResourcePoolU<UploadBuffer> m_UploadBuffers;
		Core::ResourcePoolU<ReadbackBuffer> m_ReadbackBuffers;

		struct UploadBufferData
		{
			UploadBufferDescription Description;
			UploadBuffer* UploadBuffer;
			bool IsUsed;
		};

		struct ReadbackBufferData
		{
			Texture2DDescription Description;
			ReadbackBuffer* ReadbackBuffer;
			bool IsUsed;
		};

		std::vector<UploadBufferData> m_UploadBufferData;
		std::vector<ReadbackBufferData> m_ReadbackBufferData;

		UploadBufferData& _CreateUploadBuffer(ID3D12Device* device,
			const UploadBufferDescription& description);
		ReadbackBufferData& _CreateReadbackBuffer(ID3D12Device* device,
			const Texture2DDescription& description);

	public:

		void CreateUploadBuffer(ID3D12Device* device,
			const UploadBufferDescription& description);
		void CreateReadbackBuffer(ID3D12Device* device,
			const Texture2DDescription& description);

		UploadBuffer* RequestUploadBuffer(ID3D12Device* device, 
			const UploadBufferDescription& description);
		ReadbackBuffer* RequestReadbackBuffer(ID3D12Device* device,
			const Texture2DDescription& description);

		void ReleaseUploadBuffer(UploadBuffer* uploadBuffer);
		void ReleaseReadbackBuffer(ReadbackBuffer* readbackBuffer);

		void ReleaseAllUploadBuffers();
		void ReleaseAllReadbackBuffers();
	};
}

#endif