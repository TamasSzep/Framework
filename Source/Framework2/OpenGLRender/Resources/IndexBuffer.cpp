// OpenGLRender/Resources/IndexBuffer.cpp

#include <OpenGLRender/Resources/IndexBuffer.h>

using namespace OpenGLRender;

void IndexBuffer::Initialize(BufferUsage usage, unsigned countIndices, const unsigned* pData)
{
	auto size = countIndices << 2; // Indices are always unsigned values.
	m_Buffer.Initialize(BufferTarget::IndexBuffer, size, usage, pData);
}

void IndexBuffer::Initialize(BufferUsage usage, const EngineBuildingBlocks::Graphics::IndexData& indexData)
{
	Initialize(usage, indexData.GetCountIndices(), indexData.Data.GetArray());
}

void IndexBuffer::Delete()
{
	m_Buffer.Delete();
}

GLuint IndexBuffer::GetHandle() const
{
	return m_Buffer.GetHandle();
}

void IndexBuffer::Bind()
{
	m_Buffer.Bind(BufferTarget::IndexBuffer);
}

void IndexBuffer::Unbind()
{
	m_Buffer.Unbind(BufferTarget::IndexBuffer);
}

void* IndexBuffer::Map(BufferAccessFlags accessFlags, unsigned size, unsigned offset)
{
	return m_Buffer.Map(BufferTarget::IndexBuffer, accessFlags, size, offset);
}

void IndexBuffer::Unmap()
{
	m_Buffer.Unmap(BufferTarget::IndexBuffer);
}

void IndexBuffer::Read(unsigned* pData, unsigned size, unsigned offset)
{
	m_Buffer.Read(BufferTarget::IndexBuffer, pData, size, offset);
}

void IndexBuffer::Write(const unsigned* pData, unsigned size, unsigned offset)
{
	m_Buffer.Write(BufferTarget::IndexBuffer, pData, size, offset);
}