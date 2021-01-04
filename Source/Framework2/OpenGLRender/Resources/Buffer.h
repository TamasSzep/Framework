// OpenGLRender/Resources/Buffer.h

#ifndef OPENGLRENDER_BUFFER_H_INCLUDED_
#define OPENGLRENDER_BUFFER_H_INCLUDED_

#include <Core/Enum.h>
#include <OpenGLRender/Resources/Resource.h>

namespace OpenGLRender
{
	enum class BufferTarget : unsigned
	{
		VertexBuffer = GL_ARRAY_BUFFER,
		IndexBuffer = GL_ELEMENT_ARRAY_BUFFER,
		AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
		CopyReadBuffer = GL_COPY_READ_BUFFER,
		CopyWriteBuffer = GL_COPY_WRITE_BUFFER,
		DispatchIndirectBuffer = GL_DISPATCH_INDIRECT_BUFFER,
		DrawIndirectBuffer = GL_DRAW_INDIRECT_BUFFER,
		PixelPackBuffer = GL_PIXEL_PACK_BUFFER,
		PixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER,
		QueryBuffer = GL_QUERY_BUFFER,
		ShaderStorageBuffer = GL_SHADER_STORAGE_BUFFER,
		TextureBuffer = GL_TEXTURE_BUFFER,
		TransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER,
		UniformBuffer = GL_UNIFORM_BUFFER
	};

	enum class BufferUsage : unsigned
	{
		StaticDraw = GL_STATIC_DRAW,
		StaticRead = GL_STATIC_READ,
		StaticCopy = GL_STATIC_COPY,
		DynamicDraw = GL_DYNAMIC_DRAW,
		DynamicRead = GL_DYNAMIC_READ,
		DynamicCopy = GL_DYNAMIC_COPY,
		StreamDraw = GL_STREAM_DRAW,
		StreamRead = GL_STREAM_READ,
		StreamCopy = GL_STREAM_COPY
	};

	enum class BufferAccessFlags : unsigned
	{
		Read = GL_MAP_READ_BIT,
		Write = GL_MAP_WRITE_BIT,
		Persistent = GL_MAP_PERSISTENT_BIT,
		Coherent = GL_MAP_COHERENT_BIT,
		InvalidateRange = GL_MAP_INVALIDATE_RANGE_BIT,
		InvalidateBuffer = GL_MAP_INVALIDATE_BUFFER_BIT,
		FlushExplicit = GL_MAP_FLUSH_EXPLICIT_BIT,
		Unsyhnchronized = GL_MAP_UNSYNCHRONIZED_BIT,
	};

	UseEnumAsFlagSet(BufferAccessFlags)

	class Buffer
	{
	public:

		static void DeleteResource(GLuint handle);

	private:

		ResourceHandle<Buffer> m_Handle;

	public:

		void Initialize(BufferTarget target, unsigned size, BufferUsage usage,
			const void* pData = nullptr);
		void Delete();

		GLuint GetHandle() const;

		void Bind(BufferTarget target);
		void Unbind(BufferTarget target);

		void* Map(BufferTarget target, BufferAccessFlags accessFlags, unsigned size, unsigned offset = 0);
		void Unmap(BufferTarget target);

		void Read(BufferTarget target, void* pData, unsigned size, unsigned offset = 0);
		void Write(BufferTarget target, const void* pData, unsigned size, unsigned offset = 0);
	};
}

#endif