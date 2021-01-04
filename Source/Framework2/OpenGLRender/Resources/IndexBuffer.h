// OpenGLRender/Resources/IndexBuffer.h

#ifndef OPENGLRENDER_INDEXBUFFER_H_INCLUDED_
#define OPENGLRENDER_INDEXBUFFER_H_INCLUDED_

#include <EngineBuildingBlocks/Graphics/Primitives/Primitive.h>
#include <OpenGLRender/Resources/Buffer.h>

namespace OpenGLRender
{
	class IndexBuffer
	{
		Buffer m_Buffer;

	public:

		void Initialize(BufferUsage usage, unsigned countIndices,
			const unsigned* pData = nullptr);
		void Initialize(BufferUsage usage,
			const EngineBuildingBlocks::Graphics::IndexData& indexData);
		void Delete();

		GLuint GetHandle() const;

		void Bind();
		void Unbind();

		void* Map(BufferAccessFlags accessFlags, unsigned size, unsigned offset = 0);
		void Unmap();

		void Read(unsigned* pData, unsigned size, unsigned offset = 0);
		void Write(const unsigned* pData, unsigned size, unsigned offset = 0);
	};
}

#endif