// OpenGLRender/Resources/VertexBuffer.cpp

#include <OpenGLRender/Resources/VertexBuffer.h>

using namespace EngineBuildingBlocks::Graphics;
using namespace OpenGLRender;

void VertexBuffer::Initialize(BufferUsage usage, unsigned size, const void* pData)
{
	m_Buffer.Initialize(BufferTarget::VertexBuffer, size, usage, pData);
}

void VertexBuffer::Initialize(BufferUsage usage, unsigned size)
{
	Initialize(usage, size, nullptr);
}

void VertexBuffer::Initialize(BufferUsage usage, unsigned countVertices, const VertexInputLayout& inputLayout)
{
	Initialize(usage, inputLayout.GetVertexStride() * countVertices, nullptr);
}

void VertexBuffer::Initialize(BufferUsage usage, const Vertex_AOS_Data& aosData)
{
	Initialize(usage, aosData.GetSize(), aosData.Data.GetArray());
}

void VertexBuffer::Initialize(BufferUsage usage, const Vertex_SOA_Data& soaData)
{
	Initialize(usage, soaData.As_AOS_Data());
}

void VertexBuffer::Delete()
{
	m_Buffer.Delete();
}

GLuint VertexBuffer::GetHandle() const
{
	return m_Buffer.GetHandle();
}

void VertexBuffer::Bind()
{
	m_Buffer.Bind(BufferTarget::VertexBuffer);
}

void VertexBuffer::Unbind()
{
	m_Buffer.Unbind(BufferTarget::VertexBuffer);
}

void* VertexBuffer::Map(BufferAccessFlags accessFlags, unsigned size, unsigned offset)
{
	return m_Buffer.Map(BufferTarget::VertexBuffer, accessFlags, size, offset);
}

void VertexBuffer::Unmap()
{
	m_Buffer.Unmap(BufferTarget::VertexBuffer);
}

void VertexBuffer::Read(void* pData, unsigned size, unsigned offset)
{
	m_Buffer.Read(BufferTarget::VertexBuffer, pData, size, offset);
}

void VertexBuffer::Write(const void* pData, unsigned size, unsigned offset)
{
	m_Buffer.Write(BufferTarget::VertexBuffer, pData, size, offset);
}