// OpenGLRender/Managers.h

#ifndef _OPENGLRENDER_MANAGERS_H_INCLUDED_
#define _OPENGLRENDER_MANAGERS_H_INCLUDED_

#include <EngineBuildingBlocks/PathHandler.h>
#include <OpenGLRender/Primitive.h>
#include <OpenGLRender/Resources/Shader.h>
//#include <OpenGLRender/Resources/PipelineState.h>
//#include <OpenGLRender/Resources/ResourceState.h>
#include <OpenGLRender/Resources/ConstantTextureManager.h>

namespace OpenGLRender
{
	class Managers
	{
	public: // Resource handling.

		ShaderManager ShaderManager;
		//PipelineStateManager PipelineStateManager;
		//ResourceStateManager ResourceStateManager;
		ConstantTextureManager ConstantTextureManager;
		PrimitiveManager PrimitiveManager;

	public:

		Managers(EngineBuildingBlocks::PathHandler& pathHandler);
	};
}

#endif