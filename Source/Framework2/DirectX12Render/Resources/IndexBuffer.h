// DirectX12Render/Resources/IndexBuffer.h

#ifndef _DIRECTX12RENDER_INDEXBUFFER_H_INCLUDED_
#define _DIRECTX12RENDER_INDEXBUFFER_H_INCLUDED_

#include <EngineBuildingBlocks/Graphics/Primitives/Primitive.h>

#include <DirectX12Render/DirectX12Includes.h>

namespace DirectX12Render
{
	class TransferBufferManager;

	class IndexBuffer
	{
		ComPtr<ID3D12Resource> m_Resource;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

		unsigned m_CountIndices;
		D3D12_PRIMITIVE_TOPOLOGY m_Topology;

		void CreateResource(ID3D12Device* device, D3D12_HEAP_TYPE heapType);
		void InitializeView(ID3D12Device* device);

	public:

		IndexBuffer();

		unsigned GetCountIndices() const;
		unsigned GetCPUSize() const;
		unsigned GetGPUSize() const;

		D3D12_PRIMITIVE_TOPOLOGY GetTopology() const;

		const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const;

		void Initialize(
			TransferBufferManager* transferBufferManager,
			ID3D12GraphicsCommandList* commandList,
			ID3D12Device* device,
			const EngineBuildingBlocks::Graphics::IndexData& indexData);

		void Initialize(ID3D12Device* device, D3D12_HEAP_TYPE heapType,
			unsigned countIndices,
			EngineBuildingBlocks::Graphics::PrimitiveTopology topology);

		void SetData(TransferBufferManager* transferBufferManager,
			ID3D12GraphicsCommandList* commandList, ID3D12Device* device,
			const EngineBuildingBlocks::Graphics::IndexData& indexData);
	};
}

#endif