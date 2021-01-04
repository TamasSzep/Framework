// EngineBuildingBlocks/Application/Application.hpp

#pragma once

#include <Core/System/ThreadPool.h>
#include <EngineBuildingBlocks/Application/PostUpdateContext.h>
#include <EngineBuildingBlocks/SystemTime.h>
#include <EngineBuildingBlocks/FPSController.h>
#include <EngineBuildingBlocks/EventHandling.h>
#include <EngineBuildingBlocks/Input/KeyHandler.h>
#include <EngineBuildingBlocks/Input/MouseHandler.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/Graphics/Primitives/ModelLoader.h>
#include <EngineBuildingBlocks/ResourceDatabase.h>

#include <string>
#include <iostream>

namespace EngineBuildingBlocks
{
	struct RenderContext
	{
	};

	inline void PrintException(const char* msg)
	{
		printf("An application error has been occured: %s\n", msg);
		std::cin.get();
	}

	inline void PrintException()
	{
		PrintException("unknown error");
	}

	inline void PrintException(const std::exception& ex)
	{
		PrintException(ex.what());
	}

	template <typename DerivedType>
	class Application
	{
	private:

		inline DerivedType* CRTPCall()
		{
			return static_cast<DerivedType*>(this);
		}

	protected:

		EventManager m_EventManager;

		Input::KeyHandler m_KeyHandler;
		Input::MouseHandler m_MouseHandler;

		PathHandler m_PathHandler;

		// The resource database uses the path handler.
		ResourceDatabase m_ResourceDatabase;

		// The model loader uses the path handler and the resource database.
		Graphics::ModelLoader m_ModelLoader;

		SystemTime m_SystemTime;

		unsigned m_SyncInterval;

		Core::ThreadPool m_ApplicationThreadPool;
		Core::Semaphore m_RenderSemaphore;
		std::atomic<bool> m_IsRunning, m_IsExitingRequested;

		enum class ApplicationThreadTypes
		{
			Render = 0, COUNT
		};

	private:

		void PreInitialize()
		{
			LoadConfiguration();
		}

		void ParseCommandLineParameters(int argc, char *argv[])
		{
			CRTPCall()->DerivedParseCommandLineArgs(argc, argv);
		}

		void LoadConfiguration()
		{
			// MUADDIB_TODO: implement configuration loading.
			// ...
		}

		void InitializeInput()
		{
			m_KeyHandler.Initialize();
			m_MouseHandler.Initialize();
		}

		void UpdateSystemTime()
		{
			m_SystemTime.Update();
		}

		void HandleEvents()
		{
			// First handling input events: they post events to event manager.
			m_KeyHandler.Update();
			m_MouseHandler.Update();

			m_EventManager.HandleEvents();
		}

		void PreUpdate()
		{
			CRTPCall()->DerivedPreUpdate();
		}

		void PostUpdate()
		{
			PostUpdateContext context;
			context.IsFPSRefreshed = m_IsFPSRefreshed;

			CRTPCall()->DerivedPostUpdate(context);

			m_IsFPSRefreshed = false;
		}

		void Render()
		{
			RenderContext context;

			CRTPCall()->DerivedRender(context);
		}

		void Present()
		{
			CRTPCall()->DerivedPresent();
		}

	public:

		Application(int argc, char *argv[])
			: m_KeyHandler(&m_EventManager)
			, m_MouseHandler(&m_EventManager)
			, m_ResourceDatabase(&m_PathHandler)
			, m_ModelLoader(&m_PathHandler, &m_ResourceDatabase)
			, m_SyncInterval(0)
			, m_ApplicationThreadPool((unsigned)ApplicationThreadTypes::COUNT)
			, m_RenderSemaphore(1, 0)
			, m_IsRunning(false)
			, m_IsExitingRequested(false)
		{
			ParseCommandLineParameters(argc, argv);
		}

		int Run()
		{
			auto& renderThread = m_ApplicationThreadPool.GetThread((unsigned)ApplicationThreadTypes::Render);

			// Running 2 loops in parallel:
			//
			// 1.) The main event loop on the main thread.
			// 2.) The render loop on a worker thread.
			//
			// The two event loops are synchronized by a semaphore: in the main event loop a paint event
			// request an iteration in the render loop (it increases the semaphore).

			try
			{
				// Preinitializing: parsing command line, loading configurations.
				PreInitialize();

				CRTPCall()->InitializeMain();

				InitializeInput();

				renderThread.Execute([this]
				{
					CRTPCall()->InitializeRendering();
					m_SystemTime.Initialize();
					InitializeTiming();
				});
				renderThread.Join();

				m_IsRunning = true;

				// Starting render loop on thread.
				renderThread.Execute(&Application::RenderLoop, this);

				// Entering main event loop.
				int result = CRTPCall()->EnterMainEventLoop();

				m_IsRunning = false;
				m_RenderSemaphore.Increase();
				m_ApplicationThreadPool.Join();

				// Destroying resources.
				renderThread.Execute([this]
				{
					CRTPCall()->DestroyRendering();
				});
				renderThread.Join();

				CRTPCall()->DestroyMain();

				return result;
			}
			catch (std::exception& ex)
			{
				PrintException(ex);
				return 1;
			}
			catch (...)
			{
				PrintException();
				return 2;
			}
		}

		void OnPaintEvent()
		{
			m_RenderSemaphore.Increase();
		}

		void RenderLoop()
		{
			while (m_IsRunning)
			{
				m_RenderSemaphore.Decrease();
				if (!m_IsRunning) break;

				UpdateTiming();

				if (m_IsProcessingInCurrentLoop)
				{
					UpdateSystemTime();
					PreUpdate();
					Render();
					HandleEvents();
					PostUpdate();
					Present();
				}
			}
		}

		void SetVerticalSyncEnabled(bool isEnabled)
		{
			m_SyncInterval = (isEnabled ? 1 : 0);
		}

		const PathHandler* GetPathHandler() const
		{
			return &m_PathHandler;
		}

		EventManager* GetEventManager()
		{
			return &m_EventManager;
		}

		Input::KeyHandler* GetKeyHandler()
		{
			return &m_KeyHandler;
		}

		Input::MouseHandler* GetMouseHandler()
		{
			return &m_MouseHandler;
		}

		bool IsExitingRequested() const
		{
			return m_IsExitingRequested;
		}

		void RequestExiting()
		{
			m_IsExitingRequested = true;
		}

	protected: // Timing.

		FPSController m_FPSController;
		bool m_IsProcessingInCurrentLoop;
		bool m_IsFPSRefreshed;	

		void InitializeTiming()
		{
			m_FPSController.Initialize();
			m_IsProcessingInCurrentLoop = true;
			m_IsFPSRefreshed = true;
		}

		void UpdateTiming()
		{
			m_FPSController.Update(m_IsProcessingInCurrentLoop, m_IsFPSRefreshed);
		}

	public:

		void SetMainLoopFrequency(double frequency, bool allowSleeping = true)
		{
			m_FPSController.SetTimingFrequency(frequency);
			m_FPSController.SetAllowSleeping(allowSleeping);
		}
	};
}
