// DirectX12Render/Primitive.cpp

#include <DirectX12Render/Primitive.h>

using namespace DirectX12Render;
using namespace EngineBuildingBlocks::Graphics;

VertexBuffer*  PrimitiveManager::CreateVertexBuffer(const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& vertexData,
	TransferBufferManager* transferBufferManager,
	ID3D12GraphicsCommandList* commandList,
	ID3D12Device* device)
{
	auto pVertexBuffer = m_VertexBuffers.Request();
	pVertexBuffer->Initialize(transferBufferManager, device, commandList, vertexData);
	return pVertexBuffer;
}

IndexBuffer* PrimitiveManager::CreateIndexBuffer(const IndexData& indexData,
	TransferBufferManager* transferBufferManager,
	ID3D12GraphicsCommandList* commandList,
	ID3D12Device* device)
{
	auto pIndexBuffer = m_IndexBuffers.Request();
	pIndexBuffer->Initialize(transferBufferManager, commandList, device, indexData);
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
	const Vertex_SOA_Data& vertexData, const IndexData& indexData,
	TransferBufferManager* transferBufferManager,
	ID3D12GraphicsCommandList* commandList,
	ID3D12Device* device)
{
	auto pVertexBuffer = CreateVertexBuffer(vertexData, transferBufferManager, commandList, device);
	auto pIndexBuffer = CreateIndexBuffer(indexData, transferBufferManager, commandList, device);
	return AddIndexedPrimitive(mesh, pVertexBuffer, pIndexBuffer);
}

namespace DirectX12Render
{
	void SetIndexedPrimitiveForInputAssembler(const IndexedPrimitive& primitive,
		ID3D12GraphicsCommandList* commandList)
	{
		commandList->IASetPrimitiveTopology(primitive.PIndexBuffer->GetTopology());
		commandList->IASetVertexBuffers(0, 1, &primitive.PVertexBuffer->GetVertexBufferView());
		commandList->IASetIndexBuffer(&primitive.PIndexBuffer->GetIndexBufferView());
	}
}

void PrimitiveManager::SetIndexedPrimitiveForInputAssembler(unsigned ipIndex,
	ID3D12GraphicsCommandList* commandList)
{
	DirectX12Render::SetIndexedPrimitiveForInputAssembler(m_IndexedPrimitives[ipIndex], commandList);
}

unsigned PrimitiveManager::AddIndexedPrimitive(
	const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description,
	TransferBufferManager* transferBufferManager,
	ID3D12GraphicsCommandList* commandList,
	ID3D12Device* device)
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
		auto primitiveIndex = AddIndexedPrimitive(mesh, vertexData, indexData,
			transferBufferManager, commandList, device);
		
		m_SimpleIndexedPrimitiveIndices[description] = primitiveIndex;
		return primitiveIndex;
	}
	return it->second;
}

const IndexedPrimitive& PrimitiveManager::GetIndexedPrimitive(
	const EngineBuildingBlocks::Graphics::CodedPrimitiveDescription& description,
	TransferBufferManager* transferBufferManager,
	ID3D12GraphicsCommandList* commandList,
	ID3D12Device* device)
{
	return GetIndexedPrimitive(AddIndexedPrimitive(description, transferBufferManager, commandList, device));
}