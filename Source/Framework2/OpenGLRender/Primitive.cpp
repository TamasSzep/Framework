// OpenGLRender/Primitive.cpp

#include <OpenGLRender/Primitive.h>

using namespace OpenGLRender;
using namespace EngineBuildingBlocks::Graphics;

VertexArrayObject* PrimitiveManager::CreateVertexArrayObject()
{
	auto pVertexArrayObject = m_VertexArrayObjects.Request();
	pVertexArrayObject->Initialize();
	return pVertexArrayObject;
}

VertexBuffer* PrimitiveManager::CreateVertexBuffer(const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& vertexData)
{
	auto pVertexBuffer = m_VertexBuffers.Request();
	pVertexBuffer->Initialize(BufferUsage::StaticDraw, vertexData);
	return pVertexBuffer;
}

IndexBuffer* PrimitiveManager::CreateIndexBuffer(const IndexData& indexData)
{
	auto pIndexBuffer = m_IndexBuffers.Request();
	pIndexBuffer->Initialize(BufferUsage::StaticDraw, indexData);
	return pIndexBuffer;
}

unsigned PrimitiveManager::GetCountPrimitives() const
{
	return m_Primitives.GetSize();
}

unsigned PrimitiveManager::GetCountIndexedPrimitives() const
{
	return m_IndexedPrimitives.GetSize();
}

const Primitive& PrimitiveManager::GetPrimitive(unsigned index) const
{
	return m_Primitives[index];
}

const IndexedPrimitive& PrimitiveManager::GetIndexedPrimitive(unsigned index) const
{
	return m_IndexedPrimitives[index];
}

unsigned PrimitiveManager::AddIndexedPrimitive(
	const EngineBuildingBlocks::Graphics::MeshGeometryData& mesh,
	const EngineBuildingBlocks::Graphics::VertexInputLayout* pInputLayout,
	const EngineBuildingBlocks::Graphics::PrimitiveTopology topology,
	VertexArrayObject* pVertexArrayObject,
	VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer)
{
	auto primitiveIndex = m_IndexedPrimitives.Add();
	auto& primitive = m_IndexedPrimitives[primitiveIndex];
	primitive.PVertexArrayObject = pVertexArrayObject;
	primitive.PVertexBuffer = pVertexBuffer;
	primitive.PIndexBuffer = pIndexBuffer;
	primitive.PInputLayout = pInputLayout;
	primitive.Topology = topology;
	primitive.CountVertices = mesh.CountVertices;
	primitive.CountIndices = mesh.CountIndices;
	primitive.BaseVertex = mesh.BaseVertex;
	primitive.BaseIndex = mesh.BaseIndex;
	return primitiveIndex;
}

unsigned PrimitiveManager::AddIndexedPrimitive(const MeshGeometryData& mesh,
	const Vertex_SOA_Data& vertexData, const IndexData& indexData)
{
	auto pVertexArrayObject = CreateVertexArrayObject();
	pVertexArrayObject->Bind();
	auto pVertexBuffer = CreateVertexBuffer(vertexData);
	auto pIndexBuffer = CreateIndexBuffer(indexData);
	auto pInputLayout = m_InputLayouts.Request(vertexData.InputLayout);
	return AddIndexedPrimitive(mesh, pInputLayout, indexData.Topology,
		pVertexArrayObject, pVertexBuffer, pIndexBuffer);
}

namespace OpenGLRender
{
	void DrawPrimitive(const IndexedPrimitive& primitive)
	{
		glDrawElementsBaseVertex(c_PrimitiveTopologyMap[(int)primitive.Topology], primitive.CountIndices,
			GL_UNSIGNED_INT, (void*)(size_t)(primitive.BaseIndex << 2), primitive.BaseVertex);
	}
}

unsigned PrimitiveManager::AddIndexedPrimitive(
	const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description)
{
	auto it = m_SimpleIndexedPrimitiveIndices.find(description);
	if (it == m_SimpleIndexedPrimitiveIndices.end())
	{
		Vertex_SOA_Data vertexData;
		IndexData indexData;
		CreatePrimitive(description, vertexData, indexData);

		MeshGeometryData mesh;
		mesh.BaseVertex = 0;
		mesh.BaseIndex = 0;
		mesh.CountVertices = vertexData.GetCountVertices();
		mesh.CountIndices = indexData.GetCountIndices();
		auto primitiveIndex = AddIndexedPrimitive(mesh, vertexData, indexData);
		
		m_SimpleIndexedPrimitiveIndices[description] = primitiveIndex;
		return primitiveIndex;
	}
	return it->second;
}

const IndexedPrimitive& PrimitiveManager::GetIndexedPrimitive(
	const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description)
{
	return GetIndexedPrimitive(AddIndexedPrimitive(description));
}