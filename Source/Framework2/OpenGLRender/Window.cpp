// OpenGLRender/Window.cpp

#include <OpenGLRender/Window.hpp>

#include <mutex>

namespace OpenGLRender
{
	namespace detail
	{
		std::map<GLFWwindow*, void*> s_WindowOwners;
		std::mutex s_WindowMutex;
		bool IsGLFWInitialized = false;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////

		void AddWindow(GLFWwindow* window, void* owner)
		{
			std::lock_guard<std::mutex> lock(s_WindowMutex);
			s_WindowOwners[window] = owner;
		}

		void* GetWindowOwner(GLFWwindow* window)
		{
			std::lock_guard<std::mutex> lock(s_WindowMutex);
			auto it = s_WindowOwners.find(window);
			if (it != s_WindowOwners.end())
			{
				return it->second;
			}
			return nullptr;
		}

		size_t GetCountWindows()
		{
			std::lock_guard<std::mutex> lock(s_WindowMutex);
			return s_WindowOwners.size();
		}

		void RemoveWindow(GLFWwindow* window)
		{
			std::lock_guard<std::mutex> lock(s_WindowMutex);
			s_WindowOwners.erase(window);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////

		EngineBuildingBlocks::Input::KeyState GetGeneralKeyStateForGLFW(int specificKeyState)
		{
			switch (specificKeyState)
			{
			case GLFW_PRESS: return EngineBuildingBlocks::Input::KeyState::Pressed;
			case GLFW_RELEASE: return EngineBuildingBlocks::Input::KeyState::Released;
			}
			EngineBuildingBlocks::RaiseException("Unknown key state was requested in GetGeneralKeyStateForGLFW(...).");
			return EngineBuildingBlocks::Input::KeyState::Released;
		}

		int GetSpecificKeyStateForGLFW(EngineBuildingBlocks::Input::KeyState generalKeyState)
		{
			switch (generalKeyState)
			{
			case EngineBuildingBlocks::Input::KeyState::Pressed: return GLFW_PRESS;
			case EngineBuildingBlocks::Input::KeyState::Released: return GLFW_RELEASE;
			}
			EngineBuildingBlocks::RaiseException("Unknown key state was requested in GetSpecificKeyStateForGLFW(...).");
			return 0;
		}

		EngineBuildingBlocks::Input::MouseButton GetGeneralMouseButtonForGLFW(int specificMouseButton)
		{
			switch (specificMouseButton)
			{
			case GLFW_MOUSE_BUTTON_LEFT: return EngineBuildingBlocks::Input::MouseButton::Left;
			case GLFW_MOUSE_BUTTON_MIDDLE: return EngineBuildingBlocks::Input::MouseButton::Middle;
			case GLFW_MOUSE_BUTTON_RIGHT: return EngineBuildingBlocks::Input::MouseButton::Right;
			default: return EngineBuildingBlocks::Input::MouseButton::Unknown;
			}
		}

		int GetSpecificMouseButtonForGLFW(EngineBuildingBlocks::Input::MouseButton generalMouseButton)
		{
			switch (generalMouseButton)
			{
			case EngineBuildingBlocks::Input::MouseButton::Left: return GLFW_MOUSE_BUTTON_LEFT;
			case EngineBuildingBlocks::Input::MouseButton::Middle: return GLFW_MOUSE_BUTTON_MIDDLE;
			case EngineBuildingBlocks::Input::MouseButton::Right: return GLFW_MOUSE_BUTTON_RIGHT;
			default: EngineBuildingBlocks::RaiseException("Unknown button's state was requested at GetSpecificMouseButtonForGLFW(...).");
			}
			return 0;
		}

		EngineBuildingBlocks::Input::MouseButtonState GetGeneralMouseButtonStateForGLFW(int specificMouseState)
		{
			switch (specificMouseState)
			{
			case GLFW_PRESS: return EngineBuildingBlocks::Input::MouseButtonState::Pressed;
			case GLFW_RELEASE: return EngineBuildingBlocks::Input::MouseButtonState::Released;
			default: EngineBuildingBlocks::RaiseException("Unknown button state was received at GetGeneralMouseButtonStateForGLFW(...).");
			}
			return EngineBuildingBlocks::Input::MouseButtonState::Released;
		}

		int GetSpecificMouseButtonStateForGLFW(EngineBuildingBlocks::Input::MouseButtonState generalMouseState)
		{
			switch (generalMouseState)
			{
			case EngineBuildingBlocks::Input::MouseButtonState::Pressed: return GLFW_PRESS;
			case EngineBuildingBlocks::Input::MouseButtonState::Released: return GLFW_RELEASE;
			default: EngineBuildingBlocks::RaiseException("Unknown button state was received at GetSpecificMouseButtonStateForGLFW(...).");
			}
			return 0;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////

		void InitializeGLFW()
		{
			std::lock_guard<std::mutex> lock(s_WindowMutex);
			if (!IsGLFWInitialized)
			{
				if (!glfwInit())
				{
					EngineBuildingBlocks::RaiseException("Initializing GLFW has failed.");
				}
				IsGLFWInitialized = true;
			}
		}

		void OnGLFWError(int error, const char* description)
		{
			EngineBuildingBlocks::RaiseException(description);
		}
	}
}