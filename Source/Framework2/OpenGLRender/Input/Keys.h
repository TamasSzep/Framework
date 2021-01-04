// OpenGLRender/Keys.h

#ifndef _OPENGLRENDER_KEYS_H_
#define _OPENGLRENDER_KEYS_H_

#include <EngineBuildingBlocks/Input/Keys.h>

namespace OpenGLRender
{
	namespace Input
	{
		class GLFWKeyMap : public EngineBuildingBlocks::Input::KeyMap
		{
		public:

			GLFWKeyMap();
		};
	}
}

#endif