// OpenGLRender/Managers.cpp

#include <OpenGLRender/Managers.h>

using namespace OpenGLRender;

Managers::Managers(EngineBuildingBlocks::PathHandler& pathHandler)
	: ShaderManager(&pathHandler)
	, ConstantTextureManager(&pathHandler)
{
}