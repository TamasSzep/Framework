// OpenGLRender/Primitive.h

#ifndef _OPENGLRENDER_PRIMITIVE_H_INCLUDED_
#define _OPENGLRENDER_PRIMITIVE_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeUnorderedVector.hpp>
#include <Core/DataStructures/ResourceUnorderedVector.hpp>
#include <Core/DataStructures/Pool.hpp>
#include <EngineBuildingBlocks/Graphics/Primitives/PrimitiveCreation.h>
#include <EngineBuildingBlocks/Graphics/Primitives/ModelLoader.h>
#include <OpenGLRender/Resources/VertexArrayObject.h>
#include <OpenGLRender/Resources/VertexBuffer.h>
#include <OpenGLRender/Resources/IndexBuffer.h>

#include <map>

namespace OpenGLRender
{
	struct Primitive
	{
		VertexArrayObject* PVertexArrayObject;
		VertexBuffer* PVertexBuffer;
		const EngineBuildingBlocks::Graphics::VertexInputLayout* PInputLayout;
		unsigned CountVertices;
		unsigned BaseVertex;
	};

	struct IndexedPrimitive : public Primitive
	{
		IndexBuffer* PIndexBuffer;
		EngineBuildingBlocks::Graphics::PrimitiveTopology Topology;
		unsigned CountIndices;
		unsigned BaseIndex;
	};

	// Draws the primitive. Note that the function ONLY contains a single draw and does no settings.
	void DrawPrimitive(const IndexedPrimitive& primitive);
	
	class PrimitiveManager
	{
	private: // Primitive and buffer manager functionality.

		Core::ResourcePoolU<VertexArrayObject> m_VertexArrayObjects;
		Core::ResourcePoolU<VertexBuffer> m_VertexBuffers;
		Core::ResourcePoolU<IndexBuffer> m_IndexBuffers;

		Core::ResourcePoolU<EngineBuildingBlocks::Graphics::VertexInputLayout> m_InputLayouts;

		Core::SimpleTypeUnorderedVectorU<Primitive> m_Primitives;
		Core::SimpleTypeUnorderedVectorU<IndexedPrimitive> m_IndexedPrimitives;

	public:

		VertexArrayObject* CreateVertexArrayObject();
		VertexBuffer* CreateVertexBuffer(
			const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& vertexData);
		IndexBuffer* CreateIndexBuffer(
			const EngineBuildingBlocks::Graphics::IndexData& indexData);

		unsigned GetCountPrimitives() const;
		unsigned GetCountIndexedPrimitives() const;

		const Primitive& GetPrimitive(unsigned index) const;
		const IndexedPrimitive& GetIndexedPrimitive(unsigned index) const;

		unsigned AddIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::MeshGeometryData& mesh,
			const EngineBuildingBlocks::Graphics::VertexInputLayout* pInputLayout,
			const EngineBuildingBlocks::Graphics::PrimitiveTopology topology,
			VertexArrayObject* pVertexArrayObject,
			VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer);

		unsigned AddIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::MeshGeometryData& mesh,
			const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& vertexData,
			const EngineBuildingBlocks::Graphics::IndexData& indexData);

	private: // Simple primitive manager functionality.

		std::map<EngineBuildingBlocks::Graphics::CodedPrimitiveDescription, unsigned> m_SimpleIndexedPrimitiveIndices;

	public:

		unsigned AddIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description);

		// Convinience function for registering and getting the primitive.
		const IndexedPrimitive& GetIndexedPrimitive(
			const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description);
	};

	const GLenum c_PrimitiveTopologyMap[] =
	{
		Core::c_InvalidIndexU,
		GL_POINTS,
		GL_LINES,
		GL_LINE_STRIP,
		GL_TRIANGLES,
		GL_TRIANGLE_STRIP,
		GL_LINES_ADJACENCY,
		GL_LINE_STRIP_ADJACENCY,
		GL_TRIANGLES_ADJACENCY,
		GL_TRIANGLE_STRIP_ADJACENCY,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU,
		Core::c_InvalidIndexU
	};
}

#endif