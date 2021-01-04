// OpenGLRender/Resources/VertexBuffer.h

#ifndef OPENGLRENDER_VERTEXBUFFER_H_INCLUDED_
#define OPENGLRENDER_VERTEXBUFFER_H_INCLUDED_

#include <EngineBuildingBlocks/Graphics/Primitives/Primitive.h>
#include <OpenGLRender/Resources/Buffer.h>

namespace OpenGLRender
{
	class VertexBuffer
	{
		Buffer m_Buffer;

		void Initialize(BufferUsage usage, unsigned size, const void* pData);

	public:

		void Initialize(BufferUsage usage, unsigned size);
		void Initialize(BufferUsage usage, unsigned countVertices,
			const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout);
		void Initialize(BufferUsage usage,
			const EngineBuildingBlocks::Graphics::Vertex_AOS_Data& aosData);
		void Initialize(BufferUsage usage,
			const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& soaData);
		void Delete();

		GLuint GetHandle() const;

		void Bind();
		void Unbind();

		void* Map(BufferAccessFlags accessFlags, unsigned size, unsigned offset = 0);
		void Unmap();

		void Read(void* pData, unsigned size, unsigned offset = 0);
		void Write(const void* pData, unsigned size, unsigned offset = 0);
	};
}

#endif