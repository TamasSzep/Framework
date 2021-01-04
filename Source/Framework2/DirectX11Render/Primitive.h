// DirectX11Render/Primitive.h

#ifndef _DIRECTX11RENDER_PRIMITIVE_H_INCLUDED_
#define _DIRECTX11RENDER_PRIMITIVE_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeUnorderedVector.hpp>
#include <Core/DataStructures/ResourceUnorderedVector.hpp>
#include <Core/DataStructures/Pool.hpp>
#include <EngineBuildingBlocks/Graphics/Primitives/PrimitiveCreation.h>
#include <EngineBuildingBlocks/Graphics/Primitives/ModelLoader.h>
#include <DirectX11Render/Resources/VertexBuffer.h>
#include <DirectX11Render/Resources/IndexBuffer.h>

#include <map>

namespace DirectX11Render
{
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
		ID3D11DeviceContext* context);

	void DrawPrimitive(
		const IndexedPrimitive& primitive,
		ID3D11DeviceContext* context);
	
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
			ID3D11Device* device);
		IndexBuffer* CreateIndexBuffer(
			const EngineBuildingBlocks::Graphics::IndexData& indexData,
			ID3D11Device* device);

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
			ID3D11Device* device);

		void SetIndexedPrimitiveForInputAssembler(unsigned ipIndex,
			ID3D11DeviceContext* context);

	private: // Simple primitive manager functionality.

		std::map<EngineBuildingBlocks::Graphics::CodedPrimitiveDescription, unsigned> m_SimpleIndexedPrimitiveIndices;

	public:

		unsigned AddIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description,
			ID3D11Device* device);

		// Convinience function for registering and getting the primitive.
		const IndexedPrimitive& GetIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description,
			ID3D11Device* device);
	};
}

#endif