// EngineBuildingBlocks/FPSController.cpp

#include <EngineBuildingBlocks/FPSController.h>

#include <EngineBuildingBlocks/SystemTime.h>

#include <thread>
#include <cmath>

using namespace EngineBuildingBlocks;

FPSController::FPSController()
	: m_TimingFrequency(0.0)
	, m_TotalFrameCount(0)
	, m_AllowSleeping(true)
	, m_RefreshInterval(1.0)
	, m_LastRefreshTime(0.0)
	, m_LastCountFrames(0.0)
	, m_CountFrames(0)
{
}

void FPSController::Initialize()
{
	m_Time.Initialize();
}

void FPSController::Update(bool& isProcessingRequired, bool& isRefreshed)
{
	m_Time.Update();
	auto totalTime = m_Time.GetTotalTime();
	
	// FPS measuring.
	{
		auto elapsedTime = totalTime - m_LastRefreshTime;
		if (elapsedTime >= m_RefreshInterval)
		{
			m_LastCountFrames = m_CountFrames / elapsedTime;
			m_CountFrames = 0;
			m_LastRefreshTime = totalTime;
			isRefreshed = true;
		}
	}

	// FPS controlling.
	if (m_TimingFrequency > 0.0)
	{
		double restTime = static_cast<double>(m_TotalFrameCount) / m_TimingFrequency - totalTime;

		isProcessingRequired = (restTime <= 0.0);

		if (!isProcessingRequired && m_AllowSleeping)
		{
			const double clockError = 5e-3;
			int restTimeNanos = static_cast<int>((restTime - clockError) * 1e9);
			if (restTimeNanos > 0)
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(restTimeNanos));
			}
		}
	}
	else
	{
		isProcessingRequired = true;
	}

	if (isProcessingRequired)
	{
		m_TotalFrameCount++;
		m_CountFrames++;
	}
}

void FPSController::SetTimingFrequency(double frequency)
{
	this->m_TimingFrequency = frequency;
}

void FPSController::SetAllowSleeping(bool value)
{
	this->m_AllowSleeping = value;
}

double FPSController::GetAccurateFPS() const
{
	return m_LastCountFrames;
}

unsigned FPSController::GetFPS() const
{
	return static_cast<unsigned>(round(m_LastCountFrames));
}

unsigned long long FPSController::GetTotalFrameCount() const
{
	return m_TotalFrameCount;
}

double FPSController::GetAverageFPS(const SystemTime& systemTime, int precision) const
{
	double multiplier = std::pow(10.0, static_cast<double>(precision));
	double value = static_cast<double>(m_TotalFrameCount) / systemTime.GetTotalTime();
	return (round(value * multiplier) / multiplier);
}