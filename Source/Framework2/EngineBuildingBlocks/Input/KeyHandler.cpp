// EngineBuildingBlocks/Input/KeyHandler.cpp

#include <EngineBuildingBlocks/Input/KeyHandler.h>

#include <Core/Constants.h>
#include <EngineBuildingBlocks/Input/Keys.h>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Input;

unsigned KeyData::GetCountDowns() const
{
	assert(StartState == KeyState::Pressed || StartState == KeyState::Released);
	return ((unsigned)KeyEventTimes.size() + (StartState == KeyState::Released ? 1 : 0)) >> 1;
}

unsigned KeyData::GetCountUps() const
{
	assert(StartState == KeyState::Pressed || StartState == KeyState::Released);
	return ((unsigned)KeyEventTimes.size() + (StartState == KeyState::Released ? 0 : 1)) >> 1;
}

KeyState KeyData::GetEndState() const
{
	assert(StartState == KeyState::Pressed || StartState == KeyState::Released);
	if (StartState == KeyState::Released)
	{
		return (((unsigned)KeyEventTimes.size() & 1) == 0 ? KeyState::Released : KeyState::Pressed);
	}
	else
	{
		return (((unsigned)KeyEventTimes.size() & 1) == 0 ? KeyState::Pressed : KeyState::Released);
	}
}

double KeyData::GetTotalDownTime(double startTime, double endTime) const
{
	bool isDown = (StartState == KeyState::Pressed);
	double downTime = 0.0;
	for (size_t i = 0; i < KeyEventTimes.size(); i++)
	{
		auto time = KeyEventTimes[i];
		if (isDown)
		{
			downTime += time - startTime;
		}
		else
		{
			startTime = time;
		}
		isDown = !isDown;
	}
	if (isDown)
	{
		downTime += endTime - startTime;
	}
	return downTime;
}

void KeyData::Reset()
{
	StartState = GetEndState();
	KeyEventTimes.clear();
}

KeyEvent::KeyEvent()
	: Event(Core::c_InvalidIndexU)
	, TotalDownTime(0.0)
	, CurrentState(KeyState::Released)
{
}

KeyEvent::KeyEvent(unsigned eventClassId, const KeyData& keyData, double startTime, double endTime)
	: Event(eventClassId)
	, TotalDownTime(keyData.GetTotalDownTime(startTime, endTime))
	, CurrentState(keyData.GetEndState())
{
}

KeyHandler::KeyEventHandler::KeyEventHandler(KeyHandler* owner)
	: m_Owner(owner)
{
}

KeyHandler::KeyEventHandler::~KeyEventHandler()
{
}

KeyHandler::StateKeyEventHandler::StateKeyEventHandler(KeyHandler* owner)
	: KeyEventHandler(owner)
{
}

KeyHandler::StateKeyEventHandler::~StateKeyEventHandler()
{
}

void KeyHandler::StateKeyEventHandler::Handle(unsigned eci, const KeyData& keyData, double startTime, double endTime)
{
	if (keyData.GetCountDowns() > 0) // The exact number of keydowns doesn't matter here.
	{
		m_Owner->PostKeyEvent({ eci, keyData, startTime, endTime });
	}
}

KeyHandler::TimedKeyEventHandler::TimedKeyEventHandler(KeyHandler* owner)
	: KeyEventHandler(owner)
{
}

KeyHandler::TimedKeyEventHandler::~TimedKeyEventHandler()
{
}

void KeyHandler::TimedKeyEventHandler::Handle(unsigned eci, const KeyData& keyData, double startTime, double endTime)
{
	if (keyData.GetTotalDownTime(startTime, endTime) > 0.0)
	{
		m_Owner->PostKeyEvent({ eci, keyData, startTime, endTime });
	}
}

KeyHandler::KeyHandler(EngineBuildingBlocks::EventManager* eventManager)
	: m_EventManager(eventManager)
	, m_KeyStateProvider(nullptr)
	, m_StartTime(0.0)
{
}

void KeyHandler::SetKeyStateProvider(IKeyStateProvider* keyStateProvider)
{
	this->m_KeyStateProvider = keyStateProvider;
}

void KeyHandler::Initialize()
{
	auto countKeys = static_cast<unsigned>(Keys::COUNT);
	m_KeyData.resize(countKeys);
	for (unsigned i = 0; i < countKeys; i++)
	{
		auto& keyData = m_KeyData[i];
		keyData.StartState = (m_KeyStateProvider != nullptr
			? m_KeyStateProvider->GetKeyState((Keys)i)
			: KeyState::Released); // Assuming that every key is released.
	}

	m_SystemTime.Initialize();
}

void KeyHandler::OnKeyAction(Keys key, KeyState keyState)
{
	assert(keyState == KeyState::Pressed || keyState == KeyState::Released);

	// OnKeyAction(...) is expected to be run in parallel to Update().
	std::lock_guard<std::mutex> lock(m_Mutex);

	m_SystemTime.Update();
	
	if (key != Keys::Unknown)
	{
		auto& keyData = m_KeyData[static_cast<unsigned>(key)];
		if (keyData.GetEndState() != keyState)
		{
			keyData.KeyEventTimes.push_back(m_SystemTime.GetTotalTime());
		}
	}
}

void KeyHandler::Update()
{
	auto countKeys = static_cast<unsigned>(Keys::COUNT);

	// Update() is expected to be run in parallel to OnKeyAction(...).
	std::lock_guard<std::mutex> lock(m_Mutex);

	m_SystemTime.Update();
	auto endTime = m_SystemTime.GetTotalTime();

	// Posting events.
	m_CurrentKeyEvents.ReleaseAll();
	for (auto& eciData : m_ECIMap)
	{
		eciData.second.Handler->Handle(eciData.first, m_KeyData[static_cast<unsigned>(eciData.second.BoundKey)],
			m_StartTime, endTime);
	}

	for (unsigned i = 0; i < countKeys; i++)
	{
		m_KeyData[i].Reset();
	}

	m_StartTime = endTime;
}

void KeyHandler::BindEventToKey(unsigned eventClassId, Keys key)
{
	// Valid in uninitialized state.

	auto it = m_ECIMap.find(eventClassId);
	if (it != m_ECIMap.end())
	{
		it->second.BoundKey = key;
	}
}

Keys KeyHandler::GetEventKey(unsigned eventClassId)
{
	// Valid in uninitialized state.

	auto kIt = m_ECIMap.find(eventClassId);
	if (kIt == m_ECIMap.end())
	{
		return Keys::Unknown;
	}
	return kIt->second.BoundKey;
}

unsigned KeyHandler::RegisterTimedKeyEventListener(const std::string& eventName, IEventListener* eventListener)
{
	// Valid in uninitialized state.

	std::string keyEventName = "KeyEvent_Timed_" + eventName;

	unsigned eventClassId = m_EventManager->GetEventClassId(keyEventName.c_str());
	if (eventClassId == Core::c_InvalidIndexU)
	{
		eventClassId = m_EventManager->RegisterEventClass(0, keyEventName.c_str());
	}
	m_EventManager->RegisterEventListener(eventClassId, eventListener);

	m_ECIMap[eventClassId] = { Keys::Unknown, std::make_unique<TimedKeyEventHandler>(this) };

	return eventClassId;
}

Core::IndexVectorU KeyHandler::RegisterTimedKeyEventListener(const std::vector<std::string>& eventNames, IEventListener* eventListener)
{
	// Valid in uninitialized state.

	Core::IndexVectorU eventClassIds;
	for (size_t i = 0; i < eventNames.size(); i++)
	{
		eventClassIds.PushBack(RegisterTimedKeyEventListener(eventNames[i], eventListener));
	}
	return eventClassIds;
}

unsigned KeyHandler::RegisterStateKeyEventListener(const std::string& eventName, IEventListener* eventListener)
{
	// Valid in uninitialized state.

	std::string keyEventName = "KeyEvent_State_" + eventName;

	unsigned eventClassId = m_EventManager->GetEventClassId(keyEventName.c_str());
	if (eventClassId == Core::c_InvalidIndexU)
	{
		eventClassId = m_EventManager->RegisterEventClass(0, keyEventName.c_str());
	}
	m_EventManager->RegisterEventListener(eventClassId, eventListener);

	m_ECIMap[eventClassId] = { Keys::Unknown, std::make_unique<StateKeyEventHandler>(this) };

	return eventClassId;
}

Core::IndexVectorU KeyHandler::RegisterStateKeyEventListener(const std::vector<std::string>& eventNames, IEventListener* eventListener)
{
	// Valid in uninitialized state.

	Core::IndexVectorU eventClassIds;
	for (size_t i = 0; i < eventNames.size(); i++)
	{
		eventClassIds.PushBack(RegisterStateKeyEventListener(eventNames[i], eventListener));
	}
	return eventClassIds;
}

void KeyHandler::PostKeyEvent(const KeyEvent& keyEvent)
{
	auto pKeyEvent = m_CurrentKeyEvents.Request(keyEvent);
	m_EventManager->PostEvent(pKeyEvent);
}