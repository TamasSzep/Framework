// EngineBuildingBlocks/Input/MouseHandler.h

#pragma once

#include <EngineBuildingBlocks/EventHandling.h>
#include <EngineBuildingBlocks/Math/GLM.h>

#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <Core/DataStructures/Pool.hpp>

#include <array>
#include <cassert>
#include <map>
#include <mutex>
#include <vector>

namespace EngineBuildingBlocks
{
	namespace Input
	{
		enum class MouseButton : char
		{
			Unknown = -1,

			Left,
			Middle,
			Right,

			COUNT,
		};

		enum class MouseButtonState : unsigned char
		{
			Pressed,
			Released
		};

		class IMouseStateProvider
		{
		public:
			virtual ~IMouseStateProvider() {}
			virtual MouseButtonState GetMouseButtonState(MouseButton button) const = 0;
			virtual glm::vec2 GetCursorPosition() const = 0;
		};

		struct MouseEvent : public Event
		{
			MouseEvent() {}
			MouseEvent(unsigned eventClassId);
			~MouseEvent() override {}
		};

		struct MouseButtonEvent : public MouseEvent
		{
			bool Pressed;
			glm::vec2 Position;

			MouseButtonEvent() : Pressed(false) {}
			MouseButtonEvent(unsigned eventClassId, bool pressed,
				const glm::vec2& position);
			~MouseButtonEvent() override {}
		};

		struct MouseMoveEvent : public MouseEvent
		{
			glm::vec2 Position;

			MouseMoveEvent() {}
			MouseMoveEvent(unsigned eventClassId, const glm::vec2& position);
			~MouseMoveEvent() override {}
		};

		struct MouseDragEvent : public MouseEvent
		{
			glm::vec2 DragDifference;

			MouseDragEvent() {}
			MouseDragEvent(unsigned eventClassId, const glm::vec2& dragDiff);
			~MouseDragEvent() override {}
		};

		struct MouseScrollEvent : public MouseEvent
		{
			int Scrolls;

			MouseScrollEvent() : Scrolls(-1) {}
			MouseScrollEvent(unsigned eventClassId, int scrolls);
			~MouseScrollEvent() override {}
		};

		inline const MouseButtonEvent& ToMouseButtonEvent(const Event* _event)
		{
			assert(dynamic_cast<const MouseButtonEvent*>(_event) != nullptr);
			return *static_cast<const MouseButtonEvent*>(_event);
		}

		inline const MouseMoveEvent& ToMouseMoveEvent(const Event* _event)
		{
			assert(dynamic_cast<const MouseMoveEvent*>(_event) != nullptr);
			return *static_cast<const MouseMoveEvent*>(_event);
		}

		inline const MouseDragEvent& ToMouseDragEvent(const Event* _event)
		{
			assert(dynamic_cast<const MouseDragEvent*>(_event) != nullptr);
			return *static_cast<const MouseDragEvent*>(_event);
		}

		inline const MouseScrollEvent& ToMouseScrollEvent(const Event* _event)
		{
			assert(dynamic_cast<const MouseScrollEvent*>(_event) != nullptr);
			return *static_cast<const MouseScrollEvent*>(_event);
		}

		namespace Detail
		{
			class AntiPrellFilter;
			class MouseData;
			class MouseEventHandler;
		}

		class MouseHandler
		{
			EventManager* m_EventManager;

			// Mouse state.
			IMouseStateProvider* m_MouseStateProvider;
			std::unique_ptr<Detail::MouseData> m_MouseData;
			
			std::vector<std::unique_ptr<Detail::AntiPrellFilter>> m_AntiPrellFilters;

			mutable std::mutex m_Mutex;

			Core::ResourcePoolU<MouseButtonEvent> m_CurrentMouseButtonEvents;
			Core::ResourcePoolU<MouseMoveEvent> m_CurrentMouseMoveEvents;
			Core::ResourcePoolU<MouseDragEvent> m_CurrentMouseDragEvents;
			Core::ResourcePoolU<MouseScrollEvent> m_CurrentMouseScrollEvents;
			
			struct ECIData
			{
				MouseButton BoundButton;
				std::unique_ptr<Detail::MouseEventHandler> Handler;
			};
			std::map<unsigned, ECIData> m_ECIMap;

			unsigned m_AntiPrellFilterTimeout = 0;

			// These members are synchronized with the mouse input in the render thread.
			std::array<MouseButtonState, (int)MouseButton::COUNT> m_Synced_MouseStates;
			glm::vec2 m_Synced_MousePosition;

			void InitializeAntiPrellFilters();

			unsigned RegisterMouseEventListener(const std::string& fullEventName,
				IEventListener* mouseEventListener);

		public:

			MouseHandler(EventManager* eventManager);
			~MouseHandler();

			void SetMouseStateProvider(IMouseStateProvider* mouseStateProvider);
			void SetAntiPrellFilterTimeout(unsigned timeout);

			void BindEventToButton(unsigned eventClassId, MouseButton button);
			MouseButton GetEventButton(unsigned eventClassId) const;

			MouseButtonState GetMouseButtonState(MouseButton button) const;
			const glm::vec2& GetMouseCursorPosition() const;

			void OnCursorPositionChanged(const glm::vec2& cursorPosition);
			void OnMouseButtonAction(MouseButton button, MouseButtonState state);
			void OnMouseWheelAction(int scrolls);

			unsigned RegisterMouseButtonEventListener(const std::string& eventName,
				IEventListener* mouseEventListener, bool fireOnPress, bool fireOnRelease);
			unsigned RegisterMouseMoveEventListener(const std::string& eventName,
				IEventListener* mouseEventListener);
			unsigned RegisterMouseDragEventListener(const std::string& eventName, 
				IEventListener* mouseEventListener);
			Core::IndexVectorU RegisterMouseDragEventListener(
				const Core::SimpleTypeVectorU<std::string>& eventNames,
				IEventListener* mouseEventListener);
			unsigned RegisterMouseScrollEventListener(const std::string& eventName,
				IEventListener* mouseEventListener);
		
			// There is no unregistration needed, just unregister from the EventManager.

			void Initialize();
			void Update();

			void PostMouseEvent(const MouseButtonEvent& mouseButtonEvent);
			void PostMouseEvent(const MouseMoveEvent& mouseMoveEvent);
			void PostMouseEvent(const MouseDragEvent& mouseDragEvent);
			void PostMouseEvent(const MouseScrollEvent& mouseScrollEvent);
		};
	}
}
