// DirectX12Render/Primitive.h

#ifndef _DIRECTX12RENDER_PRIMITIVE_H_INCLUDED_
#define _DIRECTX12RENDER_PRIMITIVE_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeUnorderedVector.hpp>
#include <Core/DataStructures/ResourceUnorderedVector.hpp>
#include <Core/DataStructures/Pool.hpp>
#include <EngineBuildingBlocks/Graphics/Primitives/PrimitiveCreation.h>
#include <EngineBuildingBlocks/Graphics/Primitives/ModelLoader.h>
#include <DirectX12Render/Resources/VertexBuffer.h>
#include <DirectX12Render/Resources/IndexBuffer.h>

#include <map>

namespace DirectX12Render
{
	class TransferBufferManager;

	struct Primitive
	{
		VertexBuffer* PVertexBuffer;
		unsigned CountVertices;
		unsigned BaseVertex;
	};

	struct IndexedPrimitive : public Primitive
	{
		IndexBuffer* PIndexBuffer;
		unsigned CountIndices;
		unsigned BaseIndex;
	};

	void SetIndexedPrimitiveForInputAssembler(
		const IndexedPrimitive& primitive,
		ID3D12GraphicsCommandList* commandList);
	
	class PrimitiveManager
	{
	private: // Primitive and buffer manager functionality.

		Core::ResourcePoolU<VertexBuffer> m_VertexBuffers;
		Core::ResourcePoolU<IndexBuffer> m_IndexBuffers;

		Core::SimpleTypeUnorderedVectorU<Primitive> m_Primitives;
		Core::SimpleTypeUnorderedVectorU<IndexedPrimitive> m_IndexedPrimitives;

	public:

		VertexBuffer* CreateVertexBuffer(
			const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& vertexData,
			TransferBufferManager* transferBufferManager,
			ID3D12GraphicsCommandList* commandList,
			ID3D12Device* device);
		IndexBuffer* CreateIndexBuffer(
			const EngineBuildingBlocks::Graphics::IndexData& indexData,
			TransferBufferManager* transferBufferManager,
			ID3D12GraphicsCommandList* commandList,
			ID3D12Device* device);

		unsigned GetCountPrimitives() const;
		unsigned GetCountIndexedPrimitives() const;

		const Primitive& GetPrimitive(unsigned index) const;
		const IndexedPrimitive& GetIndexedPrimitive(unsigned index) const;

		unsigned AddIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::MeshGeometryData& mesh,
			VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer);

		unsigned AddIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::MeshGeometryData& mesh,
			const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& vertexData,
			const EngineBuildingBlocks::Graphics::IndexData& indexData,
			TransferBufferManager* transferBufferManager,
			ID3D12GraphicsCommandList* commandList,
			ID3D12Device* device);

		void SetIndexedPrimitiveForInputAssembler(unsigned ipIndex,
			ID3D12GraphicsCommandList* commandList);

	private: // Simple primitive manager functionality.

		std::map<EngineBuildingBlocks::Graphics::CodedPrimitiveDescription, unsigned> m_SimpleIndexedPrimitiveIndices;

	public:

		unsigned AddIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description,
			TransferBufferManager* transferBufferManager,
			ID3D12GraphicsCommandList* commandList,
			ID3D12Device* device);

		// Convinience function for registering and getting the primitive.
		const IndexedPrimitive& GetIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description,
			TransferBufferManager* transferBufferManager,
			ID3D12GraphicsCommandList* commandList,
			ID3D12Device* device);
	};
}

#endif