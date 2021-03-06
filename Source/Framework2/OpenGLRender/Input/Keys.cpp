// Keys.cpp

#include <OpenGLRender/Input/Keys.h>

#include <OpenGLRender/GLFW.h>

using namespace OpenGLRender::Input;

#define AddEntry(specificKey, generalKey)				\
	m_SpecificToGeneral[specificKey] = generalKey;		\
	m_GeneralToSpecific[(int)generalKey] = specificKey;

GLFWKeyMap::GLFWKeyMap()
{
	const int size = 512;
	m_GeneralToSpecific.PushBack(-1, static_cast<unsigned>(size));
	for (int i = 0; i < size; i++)
	{
		m_SpecificToGeneral[i] = EngineBuildingBlocks::Input::Keys::Unknown;
	}

	AddEntry(GLFW_KEY_SPACE, EngineBuildingBlocks::Input::Keys::Space)
	AddEntry(GLFW_KEY_APOSTROPHE, EngineBuildingBlocks::Input::Keys::Apostrophe)
	AddEntry(GLFW_KEY_COMMA, EngineBuildingBlocks::Input::Keys::Comma)
	AddEntry(GLFW_KEY_MINUS, EngineBuildingBlocks::Input::Keys::Minus)
	AddEntry(GLFW_KEY_PERIOD, EngineBuildingBlocks::Input::Keys::Period)
	AddEntry(GLFW_KEY_SLASH, EngineBuildingBlocks::Input::Keys::Slash)
	AddEntry(GLFW_KEY_SEMICOLON, EngineBuildingBlocks::Input::Keys::Semicolon)
	AddEntry(GLFW_KEY_EQUAL, EngineBuildingBlocks::Input::Keys::Equal)
	AddEntry(GLFW_KEY_LEFT_BRACKET, EngineBuildingBlocks::Input::Keys::LeftBracket)
	AddEntry(GLFW_KEY_BACKSLASH, EngineBuildingBlocks::Input::Keys::BackSlash)
	AddEntry(GLFW_KEY_RIGHT_BRACKET, EngineBuildingBlocks::Input::Keys::RightBracket)
	AddEntry(GLFW_KEY_GRAVE_ACCENT, EngineBuildingBlocks::Input::Keys::GraveAccent)
	AddEntry(GLFW_KEY_WORLD_1, EngineBuildingBlocks::Input::Keys::World1)
	AddEntry(GLFW_KEY_WORLD_2, EngineBuildingBlocks::Input::Keys::World2)
	AddEntry(GLFW_KEY_0, EngineBuildingBlocks::Input::Keys::Key0)
	AddEntry(GLFW_KEY_1, EngineBuildingBlocks::Input::Keys::Key1)
	AddEntry(GLFW_KEY_2, EngineBuildingBlocks::Input::Keys::Key2)
	AddEntry(GLFW_KEY_3, EngineBuildingBlocks::Input::Keys::Key3)
	AddEntry(GLFW_KEY_4, EngineBuildingBlocks::Input::Keys::Key4)
	AddEntry(GLFW_KEY_5, EngineBuildingBlocks::Input::Keys::Key5)
	AddEntry(GLFW_KEY_6, EngineBuildingBlocks::Input::Keys::Key6)
	AddEntry(GLFW_KEY_7, EngineBuildingBlocks::Input::Keys::Key7)
	AddEntry(GLFW_KEY_8, EngineBuildingBlocks::Input::Keys::Key8)
	AddEntry(GLFW_KEY_9, EngineBuildingBlocks::Input::Keys::Key9)
	AddEntry(GLFW_KEY_A, EngineBuildingBlocks::Input::Keys::A)
	AddEntry(GLFW_KEY_B, EngineBuildingBlocks::Input::Keys::B)
	AddEntry(GLFW_KEY_C, EngineBuildingBlocks::Input::Keys::C)
	AddEntry(GLFW_KEY_D, EngineBuildingBlocks::Input::Keys::D)
	AddEntry(GLFW_KEY_E, EngineBuildingBlocks::Input::Keys::E)
	AddEntry(GLFW_KEY_F, EngineBuildingBlocks::Input::Keys::F)
	AddEntry(GLFW_KEY_G, EngineBuildingBlocks::Input::Keys::G)
	AddEntry(GLFW_KEY_H, EngineBuildingBlocks::Input::Keys::H)
	AddEntry(GLFW_KEY_I, EngineBuildingBlocks::Input::Keys::I)
	AddEntry(GLFW_KEY_J, EngineBuildingBlocks::Input::Keys::J)
	AddEntry(GLFW_KEY_K, EngineBuildingBlocks::Input::Keys::K)
	AddEntry(GLFW_KEY_L, EngineBuildingBlocks::Input::Keys::L)
	AddEntry(GLFW_KEY_M, EngineBuildingBlocks::Input::Keys::M)
	AddEntry(GLFW_KEY_N, EngineBuildingBlocks::Input::Keys::N)
	AddEntry(GLFW_KEY_O, EngineBuildingBlocks::Input::Keys::O)
	AddEntry(GLFW_KEY_P, EngineBuildingBlocks::Input::Keys::P)
	AddEntry(GLFW_KEY_Q, EngineBuildingBlocks::Input::Keys::Q)
	AddEntry(GLFW_KEY_R, EngineBuildingBlocks::Input::Keys::R)
	AddEntry(GLFW_KEY_S, EngineBuildingBlocks::Input::Keys::S)
	AddEntry(GLFW_KEY_T, EngineBuildingBlocks::Input::Keys::T)
	AddEntry(GLFW_KEY_U, EngineBuildingBlocks::Input::Keys::U)
	AddEntry(GLFW_KEY_V, EngineBuildingBlocks::Input::Keys::V)
	AddEntry(GLFW_KEY_W, EngineBuildingBlocks::Input::Keys::W)
	AddEntry(GLFW_KEY_X, EngineBuildingBlocks::Input::Keys::X)
	AddEntry(GLFW_KEY_Y, EngineBuildingBlocks::Input::Keys::Y)
	AddEntry(GLFW_KEY_Z, EngineBuildingBlocks::Input::Keys::Z)
	AddEntry(GLFW_KEY_ESCAPE, EngineBuildingBlocks::Input::Keys::Escape)
	AddEntry(GLFW_KEY_ENTER, EngineBuildingBlocks::Input::Keys::Enter)
	AddEntry(GLFW_KEY_TAB, EngineBuildingBlocks::Input::Keys::Tab)
	AddEntry(GLFW_KEY_BACKSPACE, EngineBuildingBlocks::Input::Keys::Backspace)
	AddEntry(GLFW_KEY_INSERT, EngineBuildingBlocks::Input::Keys::Insert)
	AddEntry(GLFW_KEY_DELETE, EngineBuildingBlocks::Input::Keys::Delete)
	AddEntry(GLFW_KEY_RIGHT, EngineBuildingBlocks::Input::Keys::Right)
	AddEntry(GLFW_KEY_LEFT, EngineBuildingBlocks::Input::Keys::Left)
	AddEntry(GLFW_KEY_DOWN, EngineBuildingBlocks::Input::Keys::Down)
	AddEntry(GLFW_KEY_UP, EngineBuildingBlocks::Input::Keys::Up)
	AddEntry(GLFW_KEY_PAGE_UP, EngineBuildingBlocks::Input::Keys::PageUp)
	AddEntry(GLFW_KEY_PAGE_DOWN, EngineBuildingBlocks::Input::Keys::PageDown)
	AddEntry(GLFW_KEY_HOME, EngineBuildingBlocks::Input::Keys::Home)
	AddEntry(GLFW_KEY_END, EngineBuildingBlocks::Input::Keys::End)
	AddEntry(GLFW_KEY_CAPS_LOCK, EngineBuildingBlocks::Input::Keys::CapsLock)
	AddEntry(GLFW_KEY_SCROLL_LOCK, EngineBuildingBlocks::Input::Keys::ScrollLock)
	AddEntry(GLFW_KEY_NUM_LOCK, EngineBuildingBlocks::Input::Keys::NumLock)
	AddEntry(GLFW_KEY_PRINT_SCREEN, EngineBuildingBlocks::Input::Keys::PrintScreen)
	AddEntry(GLFW_KEY_PAUSE, EngineBuildingBlocks::Input::Keys::Pause)
	AddEntry(GLFW_KEY_F1, EngineBuildingBlocks::Input::Keys::F1)
	AddEntry(GLFW_KEY_F2, EngineBuildingBlocks::Input::Keys::F2)
	AddEntry(GLFW_KEY_F3, EngineBuildingBlocks::Input::Keys::F3)
	AddEntry(GLFW_KEY_F4, EngineBuildingBlocks::Input::Keys::F4)
	AddEntry(GLFW_KEY_F5, EngineBuildingBlocks::Input::Keys::F5)
	AddEntry(GLFW_KEY_F6, EngineBuildingBlocks::Input::Keys::F6)
	AddEntry(GLFW_KEY_F7, EngineBuildingBlocks::Input::Keys::F7)
	AddEntry(GLFW_KEY_F8, EngineBuildingBlocks::Input::Keys::F8)
	AddEntry(GLFW_KEY_F9, EngineBuildingBlocks::Input::Keys::F9)
	AddEntry(GLFW_KEY_F10, EngineBuildingBlocks::Input::Keys::F10)
	AddEntry(GLFW_KEY_F11, EngineBuildingBlocks::Input::Keys::F11)
	AddEntry(GLFW_KEY_F12, EngineBuildingBlocks::Input::Keys::F12)
	AddEntry(GLFW_KEY_KP_0, EngineBuildingBlocks::Input::Keys::KeyPad0)
	AddEntry(GLFW_KEY_KP_1, EngineBuildingBlocks::Input::Keys::KeyPad1)
	AddEntry(GLFW_KEY_KP_2, EngineBuildingBlocks::Input::Keys::KeyPad2)
	AddEntry(GLFW_KEY_KP_3, EngineBuildingBlocks::Input::Keys::KeyPad3)
	AddEntry(GLFW_KEY_KP_4, EngineBuildingBlocks::Input::Keys::KeyPad4)
	AddEntry(GLFW_KEY_KP_5, EngineBuildingBlocks::Input::Keys::KeyPad5)
	AddEntry(GLFW_KEY_KP_6, EngineBuildingBlocks::Input::Keys::KeyPad6)
	AddEntry(GLFW_KEY_KP_7, EngineBuildingBlocks::Input::Keys::KeyPad7)
	AddEntry(GLFW_KEY_KP_8, EngineBuildingBlocks::Input::Keys::KeyPad8)
	AddEntry(GLFW_KEY_KP_9, EngineBuildingBlocks::Input::Keys::KeyPad9)
	AddEntry(GLFW_KEY_KP_DECIMAL, EngineBuildingBlocks::Input::Keys::KeyPadDecimal)
	AddEntry(GLFW_KEY_KP_DIVIDE, EngineBuildingBlocks::Input::Keys::KeyPadDivide)
	AddEntry(GLFW_KEY_KP_MULTIPLY, EngineBuildingBlocks::Input::Keys::KeyPadMultiply)
	AddEntry(GLFW_KEY_KP_SUBTRACT, EngineBuildingBlocks::Input::Keys::KeyPadSubtract)
	AddEntry(GLFW_KEY_KP_ADD, EngineBuildingBlocks::Input::Keys::KeyPadAdd)
	AddEntry(GLFW_KEY_KP_ENTER, EngineBuildingBlocks::Input::Keys::KeyPadEnter)
	AddEntry(GLFW_KEY_KP_EQUAL, EngineBuildingBlocks::Input::Keys::KeyPadEqual)
	AddEntry(GLFW_KEY_LEFT_SHIFT, EngineBuildingBlocks::Input::Keys::LeftShift)
	AddEntry(GLFW_KEY_LEFT_CONTROL, EngineBuildingBlocks::Input::Keys::LeftControl)
	AddEntry(GLFW_KEY_LEFT_ALT, EngineBuildingBlocks::Input::Keys::LeftAlt)
	AddEntry(GLFW_KEY_LEFT_SUPER, EngineBuildingBlocks::Input::Keys::LeftSuper)
	AddEntry(GLFW_KEY_RIGHT_SHIFT, EngineBuildingBlocks::Input::Keys::RightShift)
	AddEntry(GLFW_KEY_RIGHT_CONTROL, EngineBuildingBlocks::Input::Keys::RightControl)
	AddEntry(GLFW_KEY_RIGHT_ALT, EngineBuildingBlocks::Input::Keys::RightAlt)
	AddEntry(GLFW_KEY_RIGHT_SUPER, EngineBuildingBlocks::Input::Keys::RightSuper)
	AddEntry(GLFW_KEY_MENU, EngineBuildingBlocks::Input::Keys::Menu)
}