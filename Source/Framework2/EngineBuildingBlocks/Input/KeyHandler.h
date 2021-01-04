// EngineBuildingBlocks/Input/KeyHandler.h

#ifndef _ENGINEBUILDINGBLOCKS_KEYHANDLER_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_KEYHANDLER_H_INCLUDED_

#include <EngineBuildingBlocks/Input/Keys.h>

#include <Core/DataStructures/Pool.hpp>
#include <EngineBuildingBlocks/EventHandling.h>
#include <EngineBuildingBlocks/SystemTime.h>

#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

namespace EngineBuildingBlocks
{
	class EventManager;

	namespace Input
	{
		enum class KeyState : char
		{
			Pressed,
			Released
		};

		struct KeyData
		{
			KeyState StartState;
			std::vector<double> KeyEventTimes;

			unsigned GetCountDowns() const;
			unsigned GetCountUps() const;
			KeyState GetEndState() const;
			double GetTotalDownTime(double startTime, double endTime) const;

			void Reset();
		};

		class IKeyStateProvider
		{
		public:
			virtual ~IKeyStateProvider() {}
			virtual KeyState GetKeyState(Keys key) const = 0;
		};

		struct KeyEvent : public Event
		{
			KeyEvent();
			KeyEvent(unsigned eventClassId, const KeyData& keyData,
				double startTime, double endTime);
			~KeyEvent() override {}

			double TotalDownTime;
			KeyState CurrentState;
		};

		inline const KeyEvent& ToKeyEvent(const Event* _event)
		{
			assert(dynamic_cast<const KeyEvent*>(_event) != nullptr);
			return *static_cast<const KeyEvent*>(_event);
		}

		class KeyHandler
		{
			EventManager* m_EventManager;

			SystemTime m_SystemTime;
			double m_StartTime;

			// Key state.
			IKeyStateProvider* m_KeyStateProvider;
			std::vector<KeyData> m_KeyData;

			Core::ResourcePoolU<KeyEvent> m_CurrentKeyEvents;

			std::mutex m_Mutex;

			// Key event handling.

			class KeyEventHandler
			{
			protected:

				KeyHandler* m_Owner;

			public:

				KeyEventHandler(KeyHandler* owner);
				virtual ~KeyEventHandler();

				virtual void Handle(unsigned eci, 
					const KeyData& keyData,
					double startTime, double endTime) = 0;
			};

			class StateKeyEventHandler : public KeyEventHandler
			{
			public:

				StateKeyEventHandler(KeyHandler* owner);
				~StateKeyEventHandler() override;
				void Handle(unsigned eci,
					const KeyData& keyData,
					double startTime, double endTime);
			};

			class TimedKeyEventHandler : public KeyEventHandler
			{

			public:

				TimedKeyEventHandler(KeyHandler* owner);
				~TimedKeyEventHandler() override;
				void Handle(unsigned eci,
					const KeyData& keyData,
					double startTime, double endTime);
			};

			struct ECIData
			{
				Keys BoundKey;
				std::unique_ptr<KeyEventHandler> Handler;
			};
			std::map<unsigned, ECIData> m_ECIMap;

		public:

			KeyHandler(EventManager* eventManager);

			void SetKeyStateProvider(IKeyStateProvider* keyStateProvider);

			void OnKeyAction(Keys key, KeyState keyState);
			void Update();

			void BindEventToKey(unsigned eventId, Keys key);
			Keys GetEventKey(unsigned eventId);

			unsigned RegisterTimedKeyEventListener(const std::string& eventName, IEventListener* eventListener);
			Core::IndexVectorU RegisterTimedKeyEventListener(const std::vector<std::string>& eventNames, IEventListener* eventListener);

			unsigned RegisterStateKeyEventListener(const std::string& eventName, IEventListener* eventListener);
			Core::IndexVectorU RegisterStateKeyEventListener(const std::vector<std::string>& eventNames, IEventListener* eventListener);

			// There is no unregistration needed, just unregister from the EventManager.

			void Initialize();

			void PostKeyEvent(const KeyEvent& keyEvent);
		};
	}
}

#endif