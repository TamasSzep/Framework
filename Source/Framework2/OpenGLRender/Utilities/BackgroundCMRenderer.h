// OpenGLRender/Utilities/BackgroundCMRenderer.h

#ifndef _OPENGLRENDER_BACKGROUNDCMRENDERER_H_
#define _OPENGLRENDER_BACKGROUNDCMRENDERER_H_

#include <OpenGLRender/Primitive.h>
#include <OpenGLRender/Resources/UniformBuffer.h>
#include <OpenGLRender/Utilities/Utility.h>

namespace OpenGLRender
{
	class BackgroundCMRenderer
	{
		ShaderProgram* m_Program;

		IndexedPrimitive m_Primitive;
		Core::SimpleTypeUnorderedVectorU<Texture2D*> m_BackgroundCubemaps;

	public:

		void Initialize(const UtilityInitializationContext& context);	
		unsigned AddTexture(
			const UtilityInitializationContext& context, const char* name);
		void RemoveTexture(unsigned index);
		void ClearTextures();

		void Render(const UtilityRenderContext& context,
			UniformBuffer* renderPassCB,
			unsigned textureIndex);
	};
}

#endif