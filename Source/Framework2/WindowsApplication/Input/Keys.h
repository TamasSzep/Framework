// WindowsApplication/Input/Keys.h

#ifndef _WINDOWSAPPLICATION_KEYS_H_INCLUDED_
#define _WINDOWSAPPLICATION_KEYS_H_INCLUDED_

#include <EngineBuildingBlocks/Input/Keys.h>

namespace WindowsApplication
{
	namespace Input
	{
		class KeyMap : public EngineBuildingBlocks::Input::KeyMap
		{
		public:

			KeyMap();
		};
	}
}

#endif