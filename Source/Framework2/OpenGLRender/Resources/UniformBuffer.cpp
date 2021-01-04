// OpenGLRender/Resources/UniformBuffer.cpp

#include <OpenGLRender/Resources/UniformBuffer.h>

using namespace OpenGLRender;

void UniformBuffer::Initialize(BufferUsage usage, unsigned size)
{
	m_Buffer.Initialize(BufferTarget::UniformBuffer, size, usage, nullptr);
}

void UniformBuffer::Delete()
{
	m_Buffer.Delete();
}

GLuint UniformBuffer::GetHandle() const
{
	return m_Buffer.GetHandle();
}

void UniformBuffer::Bind(unsigned bindingIndex)
{
	glBindBufferBase((GLenum)BufferTarget::UniformBuffer, bindingIndex, GetHandle());
}

void* UniformBuffer::Map(BufferAccessFlags accessFlags, unsigned size, unsigned offset)
{
	return m_Buffer.Map(BufferTarget::UniformBuffer, accessFlags, size, offset);
}

void UniformBuffer::Unmap()
{
	m_Buffer.Unmap(BufferTarget::UniformBuffer);
}

void UniformBuffer::Read(unsigned* pData, unsigned size, unsigned offset)
{
	m_Buffer.Read(BufferTarget::UniformBuffer, pData, size, offset);
}

void UniformBuffer::Write(const void* pData, unsigned size, unsigned offset)
{
	m_Buffer.Write(BufferTarget::UniformBuffer, pData, size, offset);
}