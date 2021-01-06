// WindowsApplication/Window.h

#ifndef _WINDOWSAPPLICATION_WINDOWS_H_INCLUDED_
#define _WINDOWSAPPLICATION_WINDOWS_H_INCLUDED_

#include <EngineBuildingBlocks/Window.hpp>

#include <Core/String.hpp>
#include <WindowsApplication/WindowsIncludes.h>
#include <WindowsApplication/Input/Keys.h>

#include <mutex>

namespace WindowsApplication
{
	template <typename ApplicationType>
	class Window : public EngineBuildingBlocks::Window<ApplicationType>
	{
		HWND m_Handle;
		mutable std::mutex m_Mutex;

	public:

		Window(ApplicationType* ownerApplication)
			: EngineBuildingBlocks::Window<ApplicationType>(ownerApplication)
			, m_Handle(nullptr)
		{
			InitializeInputHandling();
		}

		HWND GetHandle() { return m_Handle; }

		void Initialize(unsigned width, unsigned height)
		{
			auto hInstance = (HINSTANCE)GetModuleHandle(nullptr);

			// Initialize the window class.
			WNDCLASSEX windowClass = { 0 };
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = WindowProc;
			windowClass.hInstance = hInstance;
			windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			windowClass.lpszClassName = L"WindowsApplication_WindowClass";
			RegisterClassEx(&windowClass);

			RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
			AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

			// Create the window and store a handle to it.
			m_Handle = CreateWindow(
				windowClass.lpszClassName,
				m_Title.c_str(),
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				windowRect.right - windowRect.left,
				windowRect.bottom - windowRect.top,
				nullptr,		// We have no parent window.
				nullptr,		// We aren't using menus.
				nullptr,		// We have no instance handle.
				this);

			m_Width = width;
			m_Height = height;
			m_IsInitialized = true;
		}
		
		int EnterMainEventLoop()
		{
			ShowWindow();

			// Main event loop.
			MSG msg = {};
			while (msg.message != WM_QUIT)
			{
				// Process any messages in the queue.
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				if (m_OwnerApplication->IsExitingRequested())
				{
					PostQuitMessage(0);
				}
			}

			// Return this part of the WM_QUIT message to Windows.
			return static_cast<char>(msg.wParam);
		}

		void SetTitle(const std::string& title)
		{
			SetTitle(Core::ToWString(title));
		}

		void SetTitle(const std::wstring& title)
		{
			if (title != m_Title)
			{
				this->m_Title = title;
				if (m_IsInitialized)
				{
					SetWindowText(m_Handle, title.c_str());
				}
			}
		}

		void ShowWindow()
		{
			::ShowWindow(m_Handle, SW_SHOW);
		}

		void HideWindow()
		{
			::ShowWindow(m_Handle, SW_HIDE);
		}

		EngineBuildingBlocks::Input::Keys GetGeneralKey(unsigned key) const
		{
			return m_KeyMap.GetGeneralKey(key);
		}
		
	protected:

		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			auto window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

			auto onKeyEvent = [message, wParam, lParam](auto&& handlerFunction)
			{
				bool bKeyDown = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
				DWORD dwMask = (1 << 29);
				bool bAltDown = ((lParam & dwMask) != 0);

				handlerFunction((unsigned)wParam, bKeyDown, bAltDown);
			};

			// Executing unconditional actions.
			switch (message)
			{
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
				case WM_KEYUP:
				case WM_SYSKEYUP:
				{
					onKeyEvent([window](unsigned key, bool isDown, bool isAltDown) {
						window->UnconditionalKeyAction(key, isDown, isAltDown);
					});
					break;
				}
				case WM_LBUTTONDOWN:
					window->UnconditionalMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Left,
						EngineBuildingBlocks::Input::MouseButtonState::Pressed); break;
				case WM_MBUTTONDOWN:
					window->UnconditionalMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Middle,
						EngineBuildingBlocks::Input::MouseButtonState::Pressed); break;
				case WM_RBUTTONDOWN:
					window->UnconditionalMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Right,
						EngineBuildingBlocks::Input::MouseButtonState::Pressed); break;
				case WM_LBUTTONUP:
					window->UnconditionalMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Left,
						EngineBuildingBlocks::Input::MouseButtonState::Released); break;
				case WM_MBUTTONUP:
					window->UnconditionalMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Middle,
						EngineBuildingBlocks::Input::MouseButtonState::Released); break;
				case WM_RBUTTONUP:
					window->UnconditionalMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Right,
						EngineBuildingBlocks::Input::MouseButtonState::Released); break;
			}

			// First the application gets notified about the event. It may reports that the event is already handled.
			if (window != nullptr)
			{
				if (window->m_OwnerApplication->OnEvent(message, wParam, lParam)) return 0;
			}
			
			bool isMessageHandled = true;
			switch (message)
			{
			case WM_CREATE:
			{
				// Save the window pointer passed in to CreateWindow.
				LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
				return 0;
			}

			case WM_PAINT:
				if (window != nullptr)
				{
					window->m_OwnerApplication->OnPaintEvent();
					return 0;
				}
				break;
			case WM_DESTROY:
				if (window != nullptr)
				{
					PostQuitMessage(0);
					return 0;
				}
				break;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				onKeyEvent([window](unsigned key, bool isDown, bool isAltDown) {
					window->OnKeyAction(key, isDown, isAltDown);
				});
				break;
			}

			case WM_LBUTTONDOWN:
				window->OnMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Left,
					EngineBuildingBlocks::Input::MouseButtonState::Pressed); break;
			case WM_MBUTTONDOWN:
				window->OnMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Middle,
					EngineBuildingBlocks::Input::MouseButtonState::Pressed); break;
			case WM_RBUTTONDOWN:
				window->OnMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Right,
					EngineBuildingBlocks::Input::MouseButtonState::Pressed); break;
			case WM_LBUTTONUP:
				window->OnMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Left,
					EngineBuildingBlocks::Input::MouseButtonState::Released); break;
			case WM_MBUTTONUP:
				window->OnMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Middle,
					EngineBuildingBlocks::Input::MouseButtonState::Released); break;
			case WM_RBUTTONUP:
				window->OnMouseButtonAction(EngineBuildingBlocks::Input::MouseButton::Right,
					EngineBuildingBlocks::Input::MouseButtonState::Released); break;

			case WM_MOUSEMOVE:
			{
				float mouseX = (float)GET_X_LPARAM(lParam);
				float mouseY = (float)GET_Y_LPARAM(lParam);
				window->OnCursorPositionChanged({ mouseX, mouseY });
				break;
			}

			case WM_MOUSEWHEEL:
			{
				auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
				window->OnMouseWheelAction(zDelta);
				break;
			}

			default:
			{
				isMessageHandled = false;
			}
			}
			if (isMessageHandled) return 0;

			// Handle any messages the switch statement didn't.
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

	protected: // Input handling.

		Input::KeyMap m_KeyMap;
		EngineBuildingBlocks::Input::MouseButtonState m_ButtonStates[(const int)EngineBuildingBlocks::Input::MouseButton::COUNT];
		std::map<EngineBuildingBlocks::Input::Keys, EngineBuildingBlocks::Input::KeyState> m_KeyStates;

		void InitializeInputHandling()
		{
			for (size_t i = 0; i < (int)EngineBuildingBlocks::Input::MouseButton::COUNT; i++)
			{
				m_ButtonStates[i] = EngineBuildingBlocks::Input::MouseButtonState::Released;
			}
		}

		void UnconditionalKeyAction(unsigned key, bool isDown, bool isAltDown)
		{
			auto generalKey = m_KeyMap.GetGeneralKey(key);
			auto keyState = isDown
				? EngineBuildingBlocks::Input::KeyState::Pressed
				: EngineBuildingBlocks::Input::KeyState::Released;

			m_Mutex.lock();
			m_KeyStates[generalKey] = keyState;
			m_Mutex.unlock();
		}

		void OnKeyAction(unsigned key, bool isDown, bool isAltDown)
		{
			auto generalKey = m_KeyMap.GetGeneralKey(key);
			auto keyState = isDown
				? EngineBuildingBlocks::Input::KeyState::Pressed
				: EngineBuildingBlocks::Input::KeyState::Released;

			m_OwnerApplication->GetKeyHandler()->OnKeyAction(generalKey, keyState);
		}

		void UnconditionalMouseButtonAction(EngineBuildingBlocks::Input::MouseButton button, EngineBuildingBlocks::Input::MouseButtonState state)
		{
			if (button != EngineBuildingBlocks::Input::MouseButton::Unknown)
			{
				m_Mutex.lock();
				m_ButtonStates[(int)button] = state;
				m_Mutex.unlock();
			}
		}

		void OnMouseButtonAction(EngineBuildingBlocks::Input::MouseButton button, EngineBuildingBlocks::Input::MouseButtonState state)
		{
			if (button != EngineBuildingBlocks::Input::MouseButton::Unknown)
			{
				m_OwnerApplication->GetMouseHandler()->OnMouseButtonAction(button, state);
			}
		}

		void OnMouseWheelAction(int scrolls)
		{
			m_OwnerApplication->GetMouseHandler()->OnMouseWheelAction(scrolls);
		}

		void OnCursorPositionChanged(const glm::vec2& position)
		{
			m_OwnerApplication->GetMouseHandler()->OnCursorPositionChanged(position);
		}

	public:

		EngineBuildingBlocks::Input::KeyState GetKeyState(EngineBuildingBlocks::Input::Keys key) const
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			auto it = m_KeyStates.find(key);
			if (it == m_KeyStates.end())
			{
				return EngineBuildingBlocks::Input::KeyState::Released;
			}
			return it->second;
		}

		EngineBuildingBlocks::Input::MouseButtonState GetMouseButtonState(EngineBuildingBlocks::Input::MouseButton button) const
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			if (button != EngineBuildingBlocks::Input::MouseButton::Unknown)
			{
				return m_ButtonStates[(int)button];
			}
			return EngineBuildingBlocks::Input::MouseButtonState::Released;
		}

		glm::vec2 GetCursorPosition() const
		{
			POINT cursorPos;
			GetCursorPos(&cursorPos);
			ScreenToClient(m_Handle, &cursorPos);
			cursorPos.x = std::min(std::max(cursorPos.x, 0L), (long)m_Width - 1);
			cursorPos.y = std::min(std::max(cursorPos.y, 0L), (long)m_Height - 1);
			return{ (float)cursorPos.x, (float)cursorPos.y };
		}
	};

	inline glm::uvec2 GetPrimaryMonitorResolution()
	{
		return glm::uvec2((unsigned)GetSystemMetrics(SM_CXSCREEN), (unsigned)GetSystemMetrics(SM_CYSCREEN));
	}

	inline bool IsInputMessage(UINT message)
	{
		switch (message)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE: return true;
		}
		return false;
	}

	inline bool IsMouseButtonDownMessage(UINT message)
	{
		switch (message)
		{
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN: return true;
		}
		return false;
	}
}

#endif