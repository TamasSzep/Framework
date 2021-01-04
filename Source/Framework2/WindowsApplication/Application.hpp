// WindowsApplication/Application.hpp

#pragma once

#include <EngineBuildingBlocks/Application/Application.hpp>
#include <WindowsApplication/Window.hpp>

namespace WindowsApplication
{
	template <typename DerivedType>
	class Application
		: public EngineBuildingBlocks::Application<DerivedType>
		, public EngineBuildingBlocks::IEventListener
	{
		WindowsApplicationInitializer m_Initializer;

	protected:

		Window<DerivedType> m_Window;
		
	public:

		Application(int argc, char *argv[])
			: EngineBuildingBlocks::Application<DerivedType>(argc, argv)
			, m_Window(static_cast<DerivedType*>(this)) {}

		~Application() override {}

		int EnterMainEventLoop()
		{
			return m_Window.EnterMainEventLoop();
		}

		Window<DerivedType>& GetWindow() { return m_Window; }
	};
}
