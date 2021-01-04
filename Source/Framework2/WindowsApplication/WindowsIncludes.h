// WindowsApplication/WindowsIncludes.h

#ifndef _WINDOWSAPPLICATION_DIRECTX12INCLUDES_H_INCLUDED_
#define _WINDOWSAPPLICATION_DIRECTX12INCLUDES_H_INCLUDED_

#include <Core/String.hpp>
#include <Core/Windows.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <EngineBuildingBlocks/Graphics/Graphics.h>

#include <wrl.h>
#include <comdef.h>
#include <Windowsx.h>

using Microsoft::WRL::ComPtr;

class WindowsApplicationInitializer
{
public:

	WindowsApplicationInitializer()
	{
		// Initializing COM.
		auto hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
		if (FAILED(hr))
		{
			EngineBuildingBlocks::RaiseException("CoInitializeEx(...) has failed.");
		}
	}
};

template<typename T>
inline void SafeRelease(T*& resource)
{
	if (resource != nullptr)
	{
		resource->Release();
		resource = nullptr;
	}
}

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		std::stringstream ss;
		ss << "An unhandled Windows error has been occured: " << Core::ToString(_com_error(hr).ErrorMessage());
		EngineBuildingBlocks::RaiseException(ss);
	}
}

#endif