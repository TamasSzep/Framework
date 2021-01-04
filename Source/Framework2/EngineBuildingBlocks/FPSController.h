// EngineBuildingBlocks/FPSController.h

#ifndef _ENGINEBUILDINGBLOCKS_FPSCONTROLLER_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_FPSCONTROLLER_H_INCLUDED_

#include <EngineBuildingBlocks/SystemTime.h>

namespace EngineBuildingBlocks
{
	class SystemTime;

	class FPSController
	{
		// FPS controlling:
		double m_TimingFrequency;
		unsigned long long m_TotalFrameCount;
		bool m_AllowSleeping;
		SystemTime m_Time;

		// FPS measuring:
		double m_RefreshInterval;
		double m_LastRefreshTime;
		double m_LastCountFrames;
		unsigned m_CountFrames;

	public:

		FPSController();

		void Initialize();

		void SetTimingFrequency(double frequency);
		void SetAllowSleeping(bool value);

		double GetAccurateFPS() const;
		unsigned GetFPS() const;

		unsigned long long GetTotalFrameCount() const;
		double GetAverageFPS(const SystemTime& systemTime, int precision = 0) const;

		void Update(bool& isProcessingRequired, bool& isRefreshed);
	};
}

#endif