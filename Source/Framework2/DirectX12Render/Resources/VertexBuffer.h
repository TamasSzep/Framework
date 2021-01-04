// DirectX12Render/Resources/VertexBuffer.h

#ifndef _DIRECTX12RENDER_VERTEXBUFFER_H_INCLUDED_
#define _DIRECTX12RENDER_VERTEXBUFFER_H_INCLUDED_

#include <EngineBuildingBlocks/Graphics/Primitives/Primitive.h>

#include <DirectX12Render/DirectX12Includes.h>

namespace DirectX12Render
{
	class TransferBufferManager;

	struct VertexInputLayout
	{
		EngineBuildingBlocks::Graphics::VertexInputLayout InputLayout;
		Core::SimpleTypeVectorU<D3D12_INPUT_ELEMENT_DESC> InputLayoutElements;
		D3D12_INPUT_LAYOUT_DESC InputLayoutDescription;

		void Initialize(
			const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
			unsigned inputSlot, bool isPerVertex);

		void Initialize(
			const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
			unsigned* inputSlots, bool* isPerVertexArray);

		bool operator==(const VertexInputLayout& other) const;
		bool operator!=(const VertexInputLayout& other) const;
		bool operator<(const VertexInputLayout& other) const;
	};

	class VertexBuffer
	{
		ComPtr<ID3D12Resource> m_Resource;
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

		VertexInputLayout m_InputLayout;

		unsigned m_CountVertices;
		unsigned m_CPUSize;

		void InitializeResource(ID3D12Device* device, D3D12_HEAP_TYPE heapType);
		void InitializeVertexBufferView();

		void UploadData(TransferBufferManager* transferBufferManager,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			const unsigned char* pData, unsigned uploadCPUSize);

	public:

		VertexBuffer();

		unsigned GetCountVertices() const;
		unsigned GetCPUSize() const;
		unsigned GetGPUSize() const;

		static unsigned AlignForVertexBuffer(unsigned size);

		const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const;
		const VertexInputLayout& GetInputLayout() const;

		void Initialize(TransferBufferManager* transferBufferManager,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			const EngineBuildingBlocks::Graphics::Vertex_AOS_Data& aosData,
			unsigned inputSlot = 0, bool isPerVertex = true);

		void Initialize(TransferBufferManager* transferBufferManager,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& soaData,
			unsigned inputSlot = 0, bool isPerVertex = true);

		void Initialize(ID3D12Device* device,
			const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
			D3D12_HEAP_TYPE heapType,
			unsigned countVertices, unsigned inputSlot = 0, bool isPerVertex = true);
	
		void SetData(unsigned char* data, size_t size);

		void SetData(TransferBufferManager* transferBufferManager,
			ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList,
			unsigned char* data, unsigned size);
	};
}

#endif