// EngineBuildingBlocks/EventHandling.h

#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>

#include <Core/DataStructures/SimpleTypeVector.hpp>

namespace EngineBuildingBlocks
{
	struct Event
	{
		Event() : ClassId(0xffffffff) {}
		Event(unsigned classId);
		virtual ~Event() {}

		unsigned ClassId;
	};

	class IEventListener
	{
	public:
		virtual ~IEventListener() {}
		virtual bool HandleEvent(const Event* _event) = 0;
	};

	struct EventClassData
	{
		unsigned ClassId;
		unsigned StartPriority;
		unsigned EndPriority;
		unsigned DefaultPriority;
		std::string Name;
		std::vector<IEventListener*> Listeners;
	};

	class EventManager
	{
		std::vector<EventClassData> m_EventClassData;

		unsigned m_IsHandlingEvents;
		unsigned m_CountEvents;
		unsigned m_StartPriority;
		std::vector<std::queue<const Event*>> m_Events;

		std::mutex m_Mutex;

	private: // Default event naming.

		unsigned m_DefaultNameIndex;

	private:

		void FireEvent(const Event* _event);

	public:

		EventManager();

		unsigned RegisterEventClass(unsigned priority, const char* name = nullptr);
		unsigned RegisterEventClass(unsigned startPriority, unsigned endPriority, unsigned defaultPriority,
			const char* name = nullptr);

		bool HasEventClass(const char* eventClassName) const;
		unsigned GetEventClassId(const char* eventClassName) const;

		void RegisterEventListener(unsigned eventClassId, IEventListener* eventListener);
		void RegisterEventListenerThreadSafe(unsigned eventClassId, IEventListener* eventListener);

		// These functions only remove the listener, but not the event class itself, even if no more listener is left.
		void UnregisterEventListener(IEventListener* eventListener);
		void UnregisterEventListenerThreadSafe(IEventListener* eventListener);
		void UnregisterEventListenersForEvent(unsigned eventClassId);

		const EventClassData& GetEventClassData(unsigned eventClassId) const;

		void PostEvent(const Event* _event);
		void PostEvent(const Event* _event, unsigned priority);

		void PostEventThreadSafe(const Event* _event);
		void PostEventThreadSafe(const Event* _event, unsigned priority);

		void HandleEvents();
	};
}
