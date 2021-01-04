// OpenGLRender/Window.hpp

#ifndef _OPENGLRENDER_WINDOW_HPP_INCLUDED_
#define _OPENGLRENDER_WINDOW_HPP_INCLUDED_

#include <EngineBuildingBlocks/Window.hpp>

#include <Core/String.hpp>
#include <EngineBuildingBlocks/Input/KeyHandler.h>
#include <EngineBuildingBlocks/Input/MouseHandler.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <EngineBuildingBlocks/Math/GLM.h>
#include <OpenGLRender/Input/Keys.h>
#include <OpenGLRender/GLFW.h>

#include <cstdlib>

namespace OpenGLRender
{
	namespace detail
	{
		void AddWindow(GLFWwindow* window, void* owner);
		void RemoveWindow(GLFWwindow* window);
		void* GetWindowOwner(GLFWwindow* window);
		size_t GetCountWindows();

		EngineBuildingBlocks::Input::KeyState GetGeneralKeyStateForGLFW(int specificKeyState);
		int GetSpecificKeyStateForGLFW(EngineBuildingBlocks::Input::KeyState generalKeyState);
		EngineBuildingBlocks::Input::MouseButton GetGeneralMouseButtonForGLFW(int specificMouseButton);
		int GetSpecificMouseButtonForGLFW(EngineBuildingBlocks::Input::MouseButton generalMouseButton);
		EngineBuildingBlocks::Input::MouseButtonState GetGeneralMouseButtonStateForGLFW(int specificMouseState);
		int GetSpecificMouseButtonStateForGLFW(EngineBuildingBlocks::Input::MouseButtonState generalMouseState);

		void InitializeGLFW();
		void OnGLFWError(int error, const char* description);

		template<typename WindowType>
		void OnGLFWMouseButtonAction(GLFWwindow* window, int button, int action, int mods)
		{
			auto owner = reinterpret_cast<WindowType*>(GetWindowOwner(window));
			if (owner != nullptr)
			{
				owner->OnMouseButtonAction(button, action, mods);
			}
		}

		template<typename WindowType>
		void OnGLFWCursorPositionChanged(GLFWwindow* window, double x, double y)
		{
			auto owner = reinterpret_cast<WindowType*>(GetWindowOwner(window));
			if (owner != nullptr)
			{
				owner->OnCursorPositionChanged({ (float)x, (float)y });
			}
		}

		template<typename WindowType>
		void OnGLFWKeyAction(GLFWwindow* window, int key, int scanCode, int action, int mods)
		{
			auto owner = reinterpret_cast<WindowType*>(GetWindowOwner(window));
			if (owner != nullptr)
			{
				owner->OnKeyAction(key, scanCode, action, mods);
			}
		}
	}

	template <typename ApplicationType>
	class Window : public EngineBuildingBlocks::Window<ApplicationType>
	{
		GLFWwindow* m_Window;

	public:

		Window(ApplicationType* ownerApplication)
			: EngineBuildingBlocks::Window<ApplicationType>(ownerApplication)
			, m_Window(nullptr){}

		~Window()
		{
			glfwDestroyWindow(m_Window);
			detail::RemoveWindow(m_Window);
			if (detail::GetCountWindows() == 0) glfwTerminate();
		}

		GLFWwindow* GetWindow() { return m_Window; }

		void Initialize(unsigned width, unsigned height,
			unsigned countSamples = 1,
			bool isVisible = true, bool isFullScreen = false,
			unsigned openGLMajorVersion = 4, unsigned openGLMinorVersion = 5,
			const glm::uvec4& channelBitCounts = glm::uvec4(8),
			unsigned depthBitCount = 32, unsigned stencilBitCount = 0)
		{
			detail::InitializeGLFW();

			// Registering GLFW error callback.
			glfwSetErrorCallback(detail::OnGLFWError);

			// Making window hints.
			{
				// Setting non-resizable.
				glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

				// Setting decorated.
				glfwWindowHint(GLFW_DECORATED, GL_TRUE);

				// Setting count samples.
				glfwWindowHint(GLFW_SAMPLES, countSamples);

				// Setting channel bits.
				glfwWindowHint(GLFW_RED_BITS, channelBitCounts.x);
				glfwWindowHint(GLFW_GREEN_BITS, channelBitCounts.y);
				glfwWindowHint(GLFW_BLUE_BITS, channelBitCounts.z);
				glfwWindowHint(GLFW_ALPHA_BITS, channelBitCounts.w);

				// Setting depth stencil bits.
				glfwWindowHint(GLFW_DEPTH_BITS, depthBitCount);
				glfwWindowHint(GLFW_STENCIL_BITS, stencilBitCount);

				// Setting visibility.
				glfwWindowHint(GLFW_VISIBLE, isVisible);

				// Setting OpenGL version.
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGLMajorVersion);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGLMinorVersion);
				glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
				if (openGLMajorVersion * 100 + openGLMinorVersion >= 302)
				{
					glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
				}
				if (openGLMajorVersion >= 3)
				{
					glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
				}

				// Setting debug context.
#if defined(_DEBUG)
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#else
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
#endif
			}

			auto monitor = (isFullScreen ? glfwGetPrimaryMonitor() : nullptr);

			m_Window = glfwCreateWindow(width, height, Core::ToString(this->m_Title).c_str(), monitor, nullptr);
			if (m_Window == nullptr)
			{
				EngineBuildingBlocks::RaiseException("Failed to create GLFW window.");
			}

			if (!isFullScreen)
			{
				glfwSetWindowPos(m_Window, 64, 64);
			}

			glfwSetWindowUserPointer(m_Window, this);
			glfwSetMouseButtonCallback(m_Window, detail::OnGLFWMouseButtonAction<Window>);
			glfwSetCursorPosCallback(m_Window, detail::OnGLFWCursorPositionChanged<Window>);
			glfwSetKeyCallback(m_Window, detail::OnGLFWKeyAction<Window>);
			glfwMakeContextCurrent(m_Window);

			detail::AddWindow(m_Window, this);

			this->m_Width = width;
			this->m_Height = height;
			this->m_IsInitialized = true;
		}

		int EnterMainEventLoop()
		{
			ShowWindow();

			// Main event loop.
			while (true)
			{
				// Polling GLFW events.
				glfwPollEvents();	

				if (this->m_OwnerApplication->IsExitingRequested() || (glfwWindowShouldClose(m_Window) != 0))
				{
					break;
				}

				this->m_OwnerApplication->OnPaintEvent();
			}

			return 0;
		}

		void SetTitle(const std::string& title)
		{
			SetTitle(Core::ToWString(title));
		}

		void SetTitle(const std::wstring& title)
		{
			if (title != this->m_Title)
			{
				this->m_Title = title;
				if (this->m_IsInitialized)
				{
					glfwSetWindowTitle(m_Window, Core::ToString(this->m_Title).c_str());
				}
			}
		}

		void ShowWindow()
		{
			glfwShowWindow(m_Window);
		}

		void HideWindow()
		{
			glfwHideWindow(m_Window);
		}

		void SetVerticalSyncEnabled(bool enabled)
		{
			if (enabled) glfwSwapInterval(1);
			else glfwSwapInterval(0);
		}

		void Present()
		{
			glfwSwapBuffers((GLFWwindow*)m_Window);
		}

		static inline glm::uvec2 GetPrimaryMonitorResolution()
		{
			detail::InitializeGLFW();

			auto videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			return glm::uvec2(videoMode->width, videoMode->height);
		}

	protected: // Input handling.

		Input::GLFWKeyMap m_KeyMap;

	public:

		void OnKeyAction(int key, int scanCode, int action, int mods)
		{
			if (action != GLFW_REPEAT)
			{
				auto generalKeyState = detail::GetGeneralKeyStateForGLFW(action);
				auto generalKey = m_KeyMap.GetGeneralKey(key);
				this->m_OwnerApplication->GetKeyHandler()->OnKeyAction(generalKey, generalKeyState);
			}
		}

		void OnMouseButtonAction(int button, int action, int mods)
		{
			auto generalMouseButton = detail::GetGeneralMouseButtonForGLFW(button);
			auto generalMouseButtonState = detail::GetGeneralMouseButtonStateForGLFW(action);
			if (generalMouseButton != EngineBuildingBlocks::Input::MouseButton::Unknown)
			{
				this->m_OwnerApplication->GetMouseHandler()->OnMouseButtonAction(generalMouseButton, generalMouseButtonState);
			}
		}

		void OnCursorPositionChanged(const glm::vec2& position)
		{
			this->m_OwnerApplication->GetMouseHandler()->OnCursorPositionChanged(position);
		}

		EngineBuildingBlocks::Input::KeyState GetKeyState(EngineBuildingBlocks::Input::Keys key) const
		{
			int specificKey = m_KeyMap.GetSpecificKey(key);
			int glfwKeyState = glfwGetKey(m_Window, specificKey);
			if (glfwKeyState == GLFW_PRESS) return EngineBuildingBlocks::Input::KeyState::Pressed;
			assert(glfwKeyState == GLFW_RELEASE);
			return EngineBuildingBlocks::Input::KeyState::Released;
		}

		EngineBuildingBlocks::Input::MouseButtonState GetMouseButtonState(EngineBuildingBlocks::Input::MouseButton button) const
		{
			return detail::GetGeneralMouseButtonStateForGLFW(glfwGetMouseButton(m_Window,
				detail::GetSpecificMouseButtonForGLFW(button)));
		}

		glm::vec2 GetCursorPosition() const
		{
			double x, y;
			glfwGetCursorPos(m_Window, &x, &y);
			return{ static_cast<float>(x), static_cast<float>(y) };
		}

		void MakeGLCurrent()
		{
			glfwMakeContextCurrent(m_Window);
		}

		void UnmakeGLCurrent()
		{
			glfwMakeContextCurrent(nullptr);
		}
	};
}

#endif