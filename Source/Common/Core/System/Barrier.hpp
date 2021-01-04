// Core/System/Barrier.hpp

#ifndef _CORE_BARRIER_HPP_
#define _CORE_BARRIER_HPP_

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <cassert>

namespace Core
{
	class Barrier
	{
		std::mutex m_Mutex;
		std::condition_variable m_ConditionalVariable;

		unsigned m_CountThreads;

		bool m_State;
		unsigned m_CurrentCount;
		bool m_IsAborted;

	public:

		Barrier(unsigned countThreads)
			: m_CountThreads(countThreads)
		{
			assert(countThreads > 0);
		
			Reset();
		}

		// Waits until all threads arrived to this point.
		// Returns whether the current thread was the last thread arriving.
		inline bool Synchronize()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			if (++m_CurrentCount == m_CountThreads)
			{
				m_CurrentCount = 0;
				m_State = !m_State;
				lock.unlock(); // Avoiding pessimization.
				m_ConditionalVariable.notify_all();
				return true;
			}
			else
			{
				const bool targetState = !m_State;
				m_ConditionalVariable.wait(lock, [=] { return m_State == targetState; });
			}
			return false;
		}

		// Waits until all threads arrived to this point or the given time duration is expired.
		// Returns whether the current thread was the last thread arriving.
		// If a timeout happens, the IsAborted flag is set, thus if the IsAborted flag is not set after the execution
		// of this function, the synchronization succeeded, but another thread might sees the IsAborted flag to be set
		// (e.g. if it times out, but sets the IsAborted flag only later).
		template<typename Duration_Rep, typename Duration_Period>
		inline bool Synchronize(const std::chrono::duration<Duration_Rep, Duration_Period>& maximumWaitingTime)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			if (++m_CurrentCount == m_CountThreads)
			{
				m_CurrentCount = 0;
				m_State = !m_State;
				lock.unlock(); // Avoiding pessimization.
				m_ConditionalVariable.notify_all();
				return true;
			}
			else
			{
				const bool targetState = !m_State;
				if (!m_ConditionalVariable.wait_for(lock, maximumWaitingTime, [=] { return m_State == targetState; }))
				{
					m_IsAborted = true;
				}
			}
			return false;
		}

		inline bool IsAborted()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			return m_IsAborted;
		}

		// This function is not allowed to call in race condition with the synchronize functions.
		void Reset()
		{
			m_State = false;
			m_CurrentCount = 0;
			m_IsAborted = false;
		}
	};
}

#endif