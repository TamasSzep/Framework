// EngineBuildingBlocks/PathHandler.h

#ifndef _ENGINEBUILDINGBLOCKS_PATHHANDLER_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_PATHHANDLER_H_INCLUDED_

#include <string>

namespace EngineBuildingBlocks
{
	class PathHandler
	{
		std::string m_ExecutablePath;
		std::string m_ExecutableDirectory;
		std::string m_SolutionDirectory;

		std::string CompletePath(const std::string& path) const;

	public:

		PathHandler();

		std::string GetPathFromExecutableDirectory(const std::string& relativePath) const;
		std::string GetPathFromSolution(const std::string& relativePath) const;
		std::string GetPathFromResourcesDirectory(const std::string& relativePath) const;
		std::string GetPathFromBuiltResourcesDirectory(const std::string& relativePath) const;
	};
}

#endif