// EngineBuildingBlocks/Input/QtKeys.h

#ifndef _ENGINEBUILDINGBLOCKS_QTKEYS_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_QTKEYS_H_INCLUDED_

#include <EngineBuildingBlocks/Settings.h>

#if (IS_BUILDING_WITH_QT)

#include <EngineBuildingBlocks/Input/Keys.h>

namespace EngineBuildingBlocks
{
	namespace Input
	{
		class QtKeyMap : public KeyMap
		{
		public:

			QtKeyMap();
		};
	}
}

#endif

#endif