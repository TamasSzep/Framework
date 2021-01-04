// OpenGLRender/Resources/Buffer.cpp

#include <OpenGLRender/Resources/Buffer.h>

#include <cassert>

using namespace OpenGLRender;

void Buffer::Initialize(BufferTarget target, unsigned size, BufferUsage usage, const void* pData)
{
	glGenBuffers(1, m_Handle.GetPointer());
	glBindBuffer((GLenum)target, m_Handle);
	glBufferData((GLenum)target, size, pData, (GLenum)usage);
}

void Buffer::Delete()
{
	m_Handle.Delete();
}

GLuint Buffer::GetHandle() const
{
	return m_Handle;
}

void Buffer::DeleteResource(GLuint handle)
{
	glDeleteBuffers(1, &handle);
}

void Buffer::Bind(BufferTarget target)
{
	glBindBuffer((GLenum)target, m_Handle);
}

void Buffer::Unbind(BufferTarget target)
{
	glBindBuffer((GLenum)target, 0);
}

void* Buffer::Map(BufferTarget target, BufferAccessFlags accessFlags, unsigned size, unsigned offset)
{
	assert(size > 0);
	glBindBuffer((GLenum)target, m_Handle);
	auto ptr = glMapBufferRange((GLenum)target, offset, size, (GLbitfield)accessFlags);
	assert(ptr != nullptr && "Failed to map the buffer.");
	return ptr;
}

void Buffer::Unmap(BufferTarget target)
{
	glUnmapBuffer((GLenum)target);
}

void Buffer::Read(BufferTarget target, void* pData, unsigned size, unsigned offset)
{
	assert(size > 0);
	glBindBuffer((GLenum)target, m_Handle);
	glGetBufferSubData((GLenum)target, offset, size, pData);
}

void Buffer::Write(BufferTarget target, const void* pData, unsigned size, unsigned offset)
{
	assert(size > 0);
	glBindBuffer((GLenum)target, m_Handle);
	glBufferSubData((GLenum)target, offset, size, pData);
}