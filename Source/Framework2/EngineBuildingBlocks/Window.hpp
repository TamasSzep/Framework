// EngineBuildingBlocks/Window.hpp

#ifndef _ENGINEBUILDINGBLOCKS_WINDOW_HPP_
#define _ENGINEBUILDINGBLOCKS_WINDOW_HPP_

#include <EngineBuildingBlocks/Input/KeyHandler.h>
#include <EngineBuildingBlocks/Input/MouseHandler.h>

#include <string>
#include <cassert>

namespace EngineBuildingBlocks
{
	template <typename ApplicationType>
	class Window
		: public EngineBuildingBlocks::Input::IKeyStateProvider
		, public EngineBuildingBlocks::Input::IMouseStateProvider
	{
	protected:

		ApplicationType* m_OwnerApplication;
		std::wstring m_Title;
		bool m_IsInitialized;

		unsigned m_Width;
		unsigned m_Height;

	public:

		Window(ApplicationType* ownerApplication)
			: m_OwnerApplication(ownerApplication)
			, m_IsInitialized(false)
			, m_Width(0)
			, m_Height(0)
		{
			assert(ownerApplication != nullptr);
		}

		~Window() override {}

		unsigned GetWidth() const { return m_Width; }
		unsigned GetHeight() const { return m_Height; }
		float GetAspectRatio() const { return (float)m_Width / (float)m_Height; }

		const std::wstring& GetTitle() const
		{
			return m_Title;
		}
	};
}

#endif