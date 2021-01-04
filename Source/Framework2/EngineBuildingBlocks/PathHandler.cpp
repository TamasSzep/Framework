// EngineBuildingBlocks/PathHandler.cpp

#include <EngineBuildingBlocks/PathHandler.h>

#include <Core/Platform.h>
#include <Core/System/Filesystem.h>
#include <EngineBuildingBlocks/ErrorHandling.h>

#ifdef IS_WINDOWS
#include <Core/String.hpp>
#include "Windows.h"
#else
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace EngineBuildingBlocks;

/////////////////////////////////////////////////////////////////////////////////////////////////

PathHandler::PathHandler()
{
	// Setting executable path.
#ifdef IS_WINDOWS

	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(nullptr, buffer, MAX_PATH);
	if (FAILED(HRESULT_FROM_WIN32(GetLastError())))
	{
		EngineBuildingBlocks::RaiseException("Error getting executable path.");
	}
	m_ExecutablePath = Core::ToString(buffer);

#else

	// Works in Linux.
	const unsigned maxPathSize = 1024;
	char id[256];
	char path[maxPathSize];
	sprintf(id, "/proc/%d/exe", getpid());
	ssize_t numChars = readlink(id, path, maxPathSize - 1);
	if (numChars == -1)
	{
		EngineBuildingBlocks::RaiseException("Error getting executable path.");
	}
	path[numChars] = '\0';
	m_ExecutablePath = path;

#endif

	// Setting executable path.
	m_ExecutableDirectory = Core::GetParentPath(m_ExecutablePath);

	// Solution folder contains a folder called 'Sources', the project folder of the file.
	for (auto cPath = Core::GetParentPath(m_ExecutableDirectory); !cPath.empty();
		cPath = Core::GetParentPath(cPath))
	{
		if (Core::FileExists(cPath + "/" + "Source"))
		{
			m_SolutionDirectory = cPath;
			return;
		}
	}

	std::stringstream ss;
	ss << "Unable to find the solution path. Executable path: " << m_ExecutablePath;
	EngineBuildingBlocks::RaiseException(ss);
}

// Note that this only works for EXISTING paths.
std::string PathHandler::CompletePath(const std::string& path) const
{
	if (Core::IsDirectory(path))
	{
		char lastChar = path[path.length() - 1];
		if (lastChar != '/' && lastChar != '\\')
		{
			return path + "/";
		}
	}
	return path;
}

std::string PathHandler::GetPathFromSolution(const std::string& relativePath) const
{
	return CompletePath(m_SolutionDirectory + "/" + relativePath);
}

std::string PathHandler::GetPathFromExecutableDirectory(const std::string& relativePath) const
{
	return CompletePath(m_ExecutableDirectory + "/" + relativePath);
}

std::string PathHandler::GetPathFromResourcesDirectory(const std::string& relativePath) const
{
	return GetPathFromSolution("Resources/" + relativePath);
}

std::string PathHandler::GetPathFromBuiltResourcesDirectory(const std::string& relativePath) const
{
	return GetPathFromSolution("Build/Resources/" + relativePath);
}