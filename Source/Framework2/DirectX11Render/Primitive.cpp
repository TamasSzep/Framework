// DirectX11Render/Primitive.cpp

#include <DirectX11Render/Primitive.h>

using namespace DirectXRender;
using namespace DirectX11Render;
using namespace EngineBuildingBlocks::Graphics;

VertexBuffer* PrimitiveManager::CreateVertexBuffer(const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& vertexData,
	ID3D11Device* device)
{
	auto pVertexBuffer = m_VertexBuffers.Request();
	pVertexBuffer->Initialize(device, vertexData);
	return pVertexBuffer;
}

IndexBuffer* PrimitiveManager::CreateIndexBuffer(const IndexData& indexData,
	ID3D11Device* device)
{
	auto pIndexBuffer = m_IndexBuffers.Request();
	pIndexBuffer->Initialize(device, indexData);
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
	VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer)
{
	auto primitiveIndex = m_IndexedPrimitives.Add();
	auto& primitive = m_IndexedPrimitives[primitiveIndex];
	primitive.PVertexBuffer = pVertexBuffer;
	primitive.PIndexBuffer = pIndexBuffer;
	primitive.CountVertices = mesh.CountVertices;
	primitive.CountIndices = mesh.CountIndices;
	primitive.BaseVertex = mesh.BaseVertex;
	primitive.BaseIndex = mesh.BaseIndex;
	return primitiveIndex;
}

unsigned PrimitiveManager::AddIndexedPrimitive(const MeshGeometryData& mesh,
	const Vertex_SOA_Data& vertexData, const IndexData& indexData, ID3D11Device* device)
{
	auto pVertexBuffer = CreateVertexBuffer(vertexData, device);
	auto pIndexBuffer = CreateIndexBuffer(indexData, device);
	return AddIndexedPrimitive(mesh, pVertexBuffer, pIndexBuffer);
}

namespace DirectX11Render
{
	void SetIndexedPrimitiveForInputAssembler(const IndexedPrimitive& primitive, ID3D11DeviceContext* context)
	{
		ID3D11Buffer* vbs[] = { primitive.PVertexBuffer->GetBuffer() };
		unsigned strides[] = { primitive.PVertexBuffer->GetVertexStride() };
		unsigned offsets[] = { 0U };
		context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
		context->IASetIndexBuffer(primitive.PIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(c_PrimitiveTopologyMap[(int)primitive.PIndexBuffer->GetTopology()]);
	}

	void DrawPrimitive(const IndexedPrimitive& primitive, ID3D11DeviceContext* context)
	{
		SetIndexedPrimitiveForInputAssembler(primitive, context);
		context->DrawIndexed(primitive.CountIndices, primitive.BaseIndex, primitive.BaseVertex);
	}
}

void PrimitiveManager::SetIndexedPrimitiveForInputAssembler(unsigned ipIndex, ID3D11DeviceContext* context)
{
	DirectX11Render::SetIndexedPrimitiveForInputAssembler(m_IndexedPrimitives[ipIndex], context);
}

unsigned PrimitiveManager::AddIndexedPrimitive(
	const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description, ID3D11Device* device)
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
		auto primitiveIndex = AddIndexedPrimitive(mesh, vertexData, indexData, device);
		
		m_SimpleIndexedPrimitiveIndices[description] = primitiveIndex;
		return primitiveIndex;
	}
	return it->second;
}

const IndexedPrimitive& PrimitiveManager::GetIndexedPrimitive(
	const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description, ID3D11Device* device)
{
	return GetIndexedPrimitive(AddIndexedPrimitive(description, device));
}