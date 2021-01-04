// OpenGLRender/Utilities/Utility.h

#ifndef _OPENGLRENDER_UTILITY_H_
#define _OPENGLRENDER_UTILITY_H_

#include <EngineBuildingBlocks/PathHandler.h>
#include <OpenGLRender/Managers.h>

namespace OpenGLRender
{
	struct UtilityInitializationContext
	{
		EngineBuildingBlocks::PathHandler* PathHandler;
		Managers* OGLM;

		unsigned MultisampleCount;
	};

	struct UtilityRenderContext
	{
		Managers* OGLM;
	};

	struct UtilityUpdateContext
	{
	};
}

#endif