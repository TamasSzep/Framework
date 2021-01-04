// EngineBuildingBlocks/Input/MouseHandler.cpp

#include <EngineBuildingBlocks/Input/MouseHandler.h>

#include <Core/Constants.h>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Input;

MouseEvent::MouseEvent(unsigned eventClassId)
	: Event(eventClassId)
{
}

MouseButtonEvent::MouseButtonEvent(unsigned eventClassId, bool pressed, const glm::vec2& position)
	: MouseEvent(eventClassId)
	, Pressed(pressed)
	, Position(position)
{
}

MouseMoveEvent::MouseMoveEvent(unsigned eventClassId, const glm::vec2& position)
	: MouseEvent(eventClassId)
	, Position(position)
{
}

MouseDragEvent::MouseDragEvent(unsigned eventClassId, const glm::vec2& dragDiff)
	: MouseEvent(eventClassId)
	, DragDifference(dragDiff)
{
}

MouseScrollEvent::MouseScrollEvent(unsigned eventClassId, int scrolls) 
	: MouseEvent(eventClassId)
	, Scrolls(scrolls)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace EngineBuildingBlocks
{
	namespace Input
	{
		namespace Detail
		{
			enum class MouseEventType : unsigned char
			{
				Press, Release, Move, Scroll
			};

			struct MouseEventData
			{
				MouseEventType Type;
				MouseButton Button;
				glm::vec2 Position;
				int Scrolls;
			};

			class MouseData
			{
			public:
				using MouseButtonStates = std::array<MouseButtonState, (int)MouseButton::COUNT>;
			private:
				MouseButtonStates m_StartStates;
				glm::vec2 m_StartPosition;
				std::vector<MouseEventData> m_Events;
			public:
				MouseData();
				void PushEvent(const MouseEventData& _event);
				const std::vector<MouseEventData>& GetEvents() const;

				void SetStartState(MouseButton button, MouseButtonState state);
				const MouseButtonStates& GetStartStates() const;
				MouseButtonState GetStartState(MouseButton button) const;
				MouseButtonState GetEndState(MouseButton button) const;

				void SetStartPosition(const glm::vec2& position);
				const glm::vec2& GetStartPosition() const;
				glm::vec2 GetEndPosition() const;

				glm::vec2 GetDragDifference(MouseButton button) const;
				int GetScrollDifference() const;

				void Reset();
			};
		}
	}
}

Detail::MouseData::MouseData()
{
	m_StartStates.fill(MouseButtonState::Released);
}

void Detail::MouseData::PushEvent(const MouseEventData& _event)
{
	m_Events.push_back(_event);
}

const std::vector<Detail::MouseEventData>& Detail::MouseData::GetEvents() const
{
	return m_Events;
}

void Detail::MouseData::SetStartState(MouseButton button, MouseButtonState state)
{
	m_StartStates[(int)button] = state;
}

const Detail::MouseData::MouseButtonStates& Detail::MouseData::GetStartStates() const
{
	return m_StartStates;
}

MouseButtonState Detail::MouseData::GetStartState(MouseButton button) const
{
	return m_StartStates[(int)button];
}

MouseButtonState Detail::MouseData::GetEndState(MouseButton button) const
{
	for (int i = ((int)m_Events.size()) - 1; i >= 0; i--)
	{
		auto& _event = m_Events[i];
		if (_event.Button == button) // For non-button events the button is unknown.
		{
			if (_event.Type == MouseEventType::Press) return MouseButtonState::Pressed;
			else if (_event.Type == MouseEventType::Release) return MouseButtonState::Released;
		}
	}
	return GetStartState(button);
}

void Detail::MouseData::SetStartPosition(const glm::vec2& position)
{
	m_StartPosition = position;
}

const glm::vec2& Detail::MouseData::GetStartPosition() const
{
	return m_StartPosition;
}

glm::vec2 Detail::MouseData::GetEndPosition() const
{
	// All events contain the currently valid end position.
	return (m_Events.size() > 0 ? m_Events.back().Position : m_StartPosition);
}

glm::vec2 Detail::MouseData::GetDragDifference(MouseButton button) const
{
	glm::vec2 dragDiff(0.0f);
	bool inDrag = (m_StartStates[(int)button] == MouseButtonState::Pressed);
	auto dragPosition = m_StartPosition;
	for (size_t i = 0; i < m_Events.size(); i++)
	{
		auto& _event = m_Events[i];
		if (_event.Type == MouseEventType::Move)
		{
			if (inDrag)
			{
				auto pos = _event.Position;
				dragDiff += pos - dragPosition;
				dragPosition = pos;
			}
		}
		else if (_event.Button == button)
		{
			if (inDrag)
			{
				assert(_event.Type == MouseEventType::Release);
				inDrag = false;
			}
			else
			{
				assert(_event.Type == MouseEventType::Press);
				inDrag = true;
				dragPosition = _event.Position;
			}
		}
	}
	return dragDiff;
}

int Detail::MouseData::GetScrollDifference() const
{
	int scroll = 0;
	for (size_t i = 0; i < m_Events.size(); i++)
	{
		auto& _event = m_Events[i];
		if (_event.Type == MouseEventType::Scroll)
		{
			assert(_event.Button == MouseButton::Middle);
			scroll += _event.Scrolls;
		}
	}
	return scroll;
}

void Detail::MouseData::Reset()
{
	for (unsigned i = 0; i < (unsigned)MouseButton::COUNT; i++)
	{
		m_StartStates[i] = GetEndState((MouseButton)i);
	}
	m_StartPosition = GetEndPosition();
	m_Events.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace EngineBuildingBlocks
{
	namespace Input
	{
		namespace Detail
		{
			class MouseEventHandler
			{
			protected:
				MouseHandler* m_Owner;

			public:
				MouseEventHandler(MouseHandler* owner);
				virtual ~MouseEventHandler();

				virtual void Handle(unsigned eci, MouseButton button, const MouseData& mouseData) = 0;
			};
		}
	}
}

Detail::MouseEventHandler::MouseEventHandler(MouseHandler* owner)
	: m_Owner(owner)
{
}

Input::Detail::MouseEventHandler::~MouseEventHandler()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace EngineBuildingBlocks
{
	namespace Input
	{
		namespace Detail
		{
			class MouseButtonEventHandler : public MouseEventHandler
			{
				MouseButtonState m_State;
				bool m_FireOnPress;
				bool m_FireOnRelease;

			public:
				MouseButtonEventHandler(MouseHandler* owner, bool fireOnPress, bool fireOnRelease);
				~MouseButtonEventHandler() override;

				void Handle(unsigned eci, MouseButton button, const MouseData& mouseData) override;
			};
		}
	}
}

Detail::MouseButtonEventHandler::MouseButtonEventHandler(MouseHandler* owner,
	bool fireOnPress, bool fireOnRelease)
	: MouseEventHandler(owner)
	, m_State(MouseButtonState::Released)
	, m_FireOnPress(fireOnPress)
	, m_FireOnRelease(fireOnRelease)
{
}

Detail::MouseButtonEventHandler::~MouseButtonEventHandler()
{
}

void Detail::MouseButtonEventHandler::Handle(unsigned eci, MouseButton button,
	const MouseData& mouseData)
{
	auto currentState = mouseData.GetStartState(button);
	for (auto& _event : mouseData.GetEvents())
	{
		if (_event.Button == button
			&& (_event.Type == MouseEventType::Press || _event.Type == MouseEventType::Release))
		{
			bool pressed = (_event.Type == MouseEventType::Press);
			auto newState = pressed ? MouseButtonState::Pressed : MouseButtonState::Released;
			if (newState != currentState) // Not firing PRESSED multiple times.
			{
				if (pressed && m_FireOnPress || !pressed && m_FireOnRelease)
				{
					m_Owner->PostMouseEvent(MouseButtonEvent(eci, pressed, _event.Position));
				}
				currentState = newState;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace EngineBuildingBlocks
{
	namespace Input
	{
		namespace Detail
		{
			class MouseMoveEventHandler : public MouseEventHandler
			{
			public:
				explicit MouseMoveEventHandler(MouseHandler* owner);
				~MouseMoveEventHandler() override;

				void Handle(unsigned eci, MouseButton button, const MouseData& mouseData) override;
			};
		}
	}
}

Detail::MouseMoveEventHandler::MouseMoveEventHandler(MouseHandler* owner)
	: MouseEventHandler(owner)
{
}

Detail::MouseMoveEventHandler::~MouseMoveEventHandler()
{
}

void Detail::MouseMoveEventHandler::Handle(unsigned eci, MouseButton button,
	const MouseData& mouseData)
{
	auto endPosition = mouseData.GetEndPosition();
	if (mouseData.GetStartPosition() != endPosition)
	{
		m_Owner->PostMouseEvent(MouseMoveEvent(eci, endPosition));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace EngineBuildingBlocks
{
	namespace Input
	{
		namespace Detail
		{
			class MouseDragEventHandler : public MouseEventHandler
			{
			public:

				MouseDragEventHandler(MouseHandler* owner);
				~MouseDragEventHandler() override;

				void Handle(unsigned eci, MouseButton button, const MouseData& mouseData) override;
			};
		}
	}
}

Detail::MouseDragEventHandler::MouseDragEventHandler(MouseHandler* owner)
	: MouseEventHandler(owner)
{
}

Detail::MouseDragEventHandler::~MouseDragEventHandler()
{
}

void Detail::MouseDragEventHandler::Handle(unsigned eci, MouseButton button,
	const MouseData& mouseData)
{
	auto dragDiff = mouseData.GetDragDifference(button);
	if (dragDiff != glm::vec2(0.0f))
	{
		m_Owner->PostMouseEvent(MouseDragEvent(eci, dragDiff));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace EngineBuildingBlocks
{
	namespace Input
	{
		namespace Detail
		{
			class MouseScrollEventHandler : public MouseEventHandler
			{
			public:
				MouseScrollEventHandler(MouseHandler* owner);
				~MouseScrollEventHandler() override;

				void Handle(unsigned eci, MouseButton button, const MouseData& mouseData) override;
			};
		}
	}
}

Detail::MouseScrollEventHandler::MouseScrollEventHandler(MouseHandler* owner)
	: MouseEventHandler(owner)
{
}

Detail::MouseScrollEventHandler::~MouseScrollEventHandler()
{
}

void Detail::MouseScrollEventHandler::Handle(unsigned eci, MouseButton button,
	const MouseData& mouseData)
{
	auto scroll = mouseData.GetScrollDifference();
	if (scroll != 0)
	{
		m_Owner->PostMouseEvent(MouseScrollEvent(eci, scroll));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace EngineBuildingBlocks
{
	namespace Input
	{
		namespace Detail
		{
			class AntiPrellFilter
			{
				MouseData& m_MouseData;
				const MouseButton m_Button;
				const unsigned m_Timeout;

				enum class State { Released, Pressed, ReleasedFrozen } m_State;

				std::chrono::steady_clock::time_point m_ReleasedFreezeTime;

				void PushEvent(MouseButtonState state);

			public:

				explicit AntiPrellFilter(MouseData& mouseData, MouseButton button, unsigned timeout);
				~AntiPrellFilter();

				void OnMouseButtonAction(MouseButtonState state);
				void Update(std::chrono::steady_clock::time_point currentTime);
			};
		}
	}
}

Detail::AntiPrellFilter::AntiPrellFilter(MouseData& mouseData, MouseButton button,
	unsigned timeout)
	: m_MouseData(mouseData)
	, m_Button(button)
	, m_Timeout(timeout)
{
	auto buttonState = m_MouseData.GetEndState(button);
	m_State = (buttonState == MouseButtonState::Pressed) ? State::Pressed : State::Released;
}

Detail::AntiPrellFilter::~AntiPrellFilter()
{
}

void Detail::AntiPrellFilter::OnMouseButtonAction(MouseButtonState state)
{
	if (m_Timeout == 0)
	{
		if (state != m_MouseData.GetEndState(m_Button))
		{
			PushEvent(state);
		}
	}
	else
	{
		if (m_State == State::Released)
		{
			if (state == MouseButtonState::Pressed)
			{
				m_State = State::Pressed;
				PushEvent(state);
			}
		}
		else if (m_State == State::Pressed)
		{
			if (state == MouseButtonState::Released)
			{
				m_State = State::ReleasedFrozen;
				PushEvent(state);
				m_ReleasedFreezeTime = std::chrono::steady_clock::now();
			}
		}
	}
}

void Detail::AntiPrellFilter::Update(std::chrono::steady_clock::time_point currentTime)
{
	if (m_State == State::ReleasedFrozen && std::chrono::duration_cast<std::chrono::milliseconds>(
		currentTime - m_ReleasedFreezeTime).count() >= m_Timeout)
	{
		m_State = State::Released;
	}
}

void Detail::AntiPrellFilter::PushEvent(MouseButtonState state)
{
	m_MouseData.PushEvent({ state == MouseButtonState::Pressed
		? MouseEventType::Press : MouseEventType::Release, m_Button, m_MouseData.GetEndPosition(), 0 });
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MouseHandler::MouseHandler(EventManager* eventManager)
	: m_EventManager(eventManager)
	, m_MouseStateProvider(nullptr)
	, m_MouseData(std::make_unique<Detail::MouseData>())
{
	m_Synced_MouseStates.fill(MouseButtonState::Released);
}

MouseHandler::~MouseHandler()
{
}

void MouseHandler::SetMouseStateProvider(IMouseStateProvider* mouseStateProvider)
{
	this->m_MouseStateProvider = mouseStateProvider;
}

void MouseHandler::SetAntiPrellFilterTimeout(unsigned timeout)
{
	m_AntiPrellFilterTimeout = timeout;
	if (!m_AntiPrellFilters.empty())
	{
		InitializeAntiPrellFilters();
	}
}

void MouseHandler::InitializeAntiPrellFilters()
{
	constexpr auto c_CountButtons = (unsigned)MouseButton::COUNT;
	m_AntiPrellFilters.resize(c_CountButtons);
	for (unsigned i = 0; i < c_CountButtons; i++)
	{
		auto button = (MouseButton)i;
		m_AntiPrellFilters[i] = std::make_unique<Detail::AntiPrellFilter>(*m_MouseData, button,
			m_AntiPrellFilterTimeout);
	}
}

void MouseHandler::Initialize()
{
	if (m_MouseStateProvider != nullptr)
	{
		m_MouseData->SetStartPosition(m_MouseStateProvider->GetCursorPosition());
	}
	for (unsigned i = 0; i < (unsigned)MouseButton::COUNT; i++)
	{
		auto button = (MouseButton)i;
		m_MouseData->SetStartState(button, m_MouseStateProvider != nullptr
			? m_MouseStateProvider->GetMouseButtonState(button)
			: MouseButtonState::Released);
	}

	InitializeAntiPrellFilters();
}

void MouseHandler::BindEventToButton(unsigned eventClassId, MouseButton button)
{
	// Valid in uninitialized state.

	auto it = m_ECIMap.find(eventClassId);
	if (it != m_ECIMap.end())
	{
		it->second.BoundButton = button;
	}
}

MouseButton MouseHandler::GetEventButton(unsigned eventClassId) const
{
	// Valid in uninitialized state.

	auto kIt = m_ECIMap.find(eventClassId);
	if (kIt == m_ECIMap.end())
	{
		return MouseButton::Unknown;
	}
	return kIt->second.BoundButton;
}

MouseButtonState MouseHandler::GetMouseButtonState(MouseButton button) const
{
	return m_Synced_MouseStates[(int)button];
}

const glm::vec2& MouseHandler::GetMouseCursorPosition() const
{
	return m_Synced_MousePosition;
}

void MouseHandler::OnCursorPositionChanged(const glm::vec2& cursorPosition)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	m_MouseData->PushEvent({ Detail::MouseEventType::Move, MouseButton::Unknown, cursorPosition, 0 });
}

void MouseHandler::OnMouseButtonAction(MouseButton button, MouseButtonState state)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	assert(!m_AntiPrellFilters.empty());
	m_AntiPrellFilters[(int)button]->OnMouseButtonAction(state);
}

void MouseHandler::OnMouseWheelAction(int scrolls)
{
	if (scrolls != 0)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_MouseData->PushEvent({ Detail::MouseEventType::Scroll, MouseButton::Middle,
			m_MouseData->GetEndPosition(), scrolls });
	}
}

void MouseHandler::Update()
{
	auto currentTime = std::chrono::steady_clock::now();

	std::lock_guard<std::mutex> lock(m_Mutex);

	assert(!m_AntiPrellFilters.empty());

	if (m_MouseStateProvider != nullptr)
	{
		// Committing last move event.
		auto cursorPosition = m_MouseStateProvider->GetCursorPosition();
		if (cursorPosition != m_MouseData->GetEndPosition())
		{
			m_MouseData->PushEvent({ Detail::MouseEventType::Move, MouseButton::Unknown, cursorPosition, 0 });
		}
	}

	for (unsigned i = 0; i < (unsigned)MouseButton::COUNT; i++)
	{
		m_AntiPrellFilters[i]->Update(currentTime);
	}

	m_CurrentMouseButtonEvents.ReleaseAll();
	m_CurrentMouseDragEvents.ReleaseAll();
	m_CurrentMouseScrollEvents.ReleaseAll();

	for (auto& it : m_ECIMap)
	{
		it.second.Handler->Handle(it.first, it.second.BoundButton, *m_MouseData);
	}
	m_MouseData->Reset();

	m_Synced_MouseStates = m_MouseData->GetStartStates();
	m_Synced_MousePosition = m_MouseData->GetStartPosition();
}

unsigned MouseHandler::RegisterMouseEventListener(const std::string& fullEventName, IEventListener* mouseEventListener)
{
	unsigned eventClassId = m_EventManager->GetEventClassId(fullEventName.c_str());
	if (eventClassId == Core::c_InvalidIndexU)
	{
		eventClassId = m_EventManager->RegisterEventClass(0, fullEventName.c_str());
	}
	m_EventManager->RegisterEventListener(eventClassId, mouseEventListener);
	return eventClassId;
}

unsigned MouseHandler::RegisterMouseButtonEventListener(const std::string& eventName,
	IEventListener* mouseEventListener, bool fireOnPress, bool fireOnRelease)
{
	// Valid in uninitialized state.

	unsigned eventClassId = RegisterMouseEventListener("MouseEvent_Button_" + eventName, mouseEventListener);

	m_ECIMap[eventClassId] = { MouseButton::Unknown, std::make_unique<Detail::MouseButtonEventHandler>(this,
		fireOnPress, fireOnRelease) };

	return eventClassId;
}

unsigned MouseHandler::RegisterMouseMoveEventListener(const std::string& eventName,
	IEventListener* mouseEventListener)
{
	// Valid in uninitialized state.

	unsigned eventClassId = RegisterMouseEventListener("MouseEvent_Move_" + eventName, mouseEventListener);

	m_ECIMap[eventClassId] = { MouseButton::Unknown, std::make_unique<Detail::MouseMoveEventHandler>(this) };

	return eventClassId;
}

unsigned MouseHandler::RegisterMouseDragEventListener(const std::string& eventName, IEventListener* mouseEventListener)
{
	// Valid in uninitialized state.

	unsigned eventClassId = RegisterMouseEventListener("MouseEvent_Drag_" + eventName, mouseEventListener);

	m_ECIMap[eventClassId] = { MouseButton::Unknown, std::make_unique<Detail::MouseDragEventHandler>(this) };

	return eventClassId;
}

Core::IndexVectorU MouseHandler::RegisterMouseDragEventListener(
	const Core::SimpleTypeVectorU<std::string>& eventNames, IEventListener* mouseEventListener)
{
	// Valid in uninitialized state.

	Core::IndexVectorU eventClassIds;
	for (unsigned i = 0; i < eventNames.GetSize(); i++)
	{
		eventClassIds.PushBack(RegisterMouseDragEventListener(eventNames[i], mouseEventListener));
	}
	return eventClassIds;
}

unsigned MouseHandler::RegisterMouseScrollEventListener(const std::string& eventName,
	IEventListener* mouseEventListener)
{
	// Valid in uninitialized state.

	unsigned eventClassId = RegisterMouseEventListener("MouseEvent_Scroll_" + eventName, mouseEventListener);

	m_ECIMap[eventClassId] = { MouseButton::Middle, std::make_unique<Detail::MouseScrollEventHandler>(this) };

	return eventClassId;
}

void MouseHandler::PostMouseEvent(const MouseButtonEvent& mouseButtonEvent)
{
	auto pMouseButtonEvent = m_CurrentMouseButtonEvents.Request(mouseButtonEvent);
	m_EventManager->PostEvent(pMouseButtonEvent);
}

void MouseHandler::PostMouseEvent(const MouseMoveEvent& mouseMoveEvent)
{
	auto pMouseMoveEvent = m_CurrentMouseMoveEvents.Request(mouseMoveEvent);
	m_EventManager->PostEvent(pMouseMoveEvent);
}

void MouseHandler::PostMouseEvent(const MouseDragEvent& mouseDragEvent)
{
	auto pMouseDragEvent = m_CurrentMouseDragEvents.Request(mouseDragEvent);
	m_EventManager->PostEvent(pMouseDragEvent);
}

void MouseHandler::PostMouseEvent(const MouseScrollEvent& mouseScrollEvent)
{
	auto pMouseScrollEvent = m_CurrentMouseScrollEvents.Request(mouseScrollEvent);
	m_EventManager->PostEvent(pMouseScrollEvent);
}
