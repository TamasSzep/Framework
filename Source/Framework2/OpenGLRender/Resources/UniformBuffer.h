// OpenGLRender/Resources/UniformBuffer.h

#ifndef OPENGLRENDER_UNIFORMBUFFER_H_INCLUDED_
#define OPENGLRENDER_UNIFORMBUFFER_H_INCLUDED_

#include <OpenGLRender/Resources/Buffer.h>

namespace OpenGLRender
{
	class UniformBuffer
	{
		Buffer m_Buffer;

	public:

		void Initialize(BufferUsage usage, unsigned size);
		void Delete();

		GLuint GetHandle() const;

		void Bind(unsigned bindingIndex);

		void* Map(BufferAccessFlags accessFlags, unsigned size, unsigned offset = 0);
		void Unmap();

		void Read(unsigned* pData, unsigned size, unsigned offset = 0);
		void Write(const void* pData, unsigned size, unsigned offset = 0);
	};
}

#endif