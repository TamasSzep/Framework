// EngineBuildingBlocks/ResourceDatabase.h

#ifndef _ENGINEBUILDINGBLOCKS_RESOURCEDATABASE_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_RESOURCEDATABASE_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeVector.hpp>

#include <string>
#include <vector>
#include <map>

namespace EngineBuildingBlocks
{
	class PathHandler;

	struct ResourceDescription
	{
		// True if the resource as a built resource, e.g. an exported it from a third party framework
		// (e.g. game engine) in the built format.
		bool IsBuiltResource;

		std::string ResourceFilePath;
		std::vector<std::string> DependencyPaths;
		Core::ByteVectorU SerializedBuildOptions;

		ResourceDescription();

		bool operator==(const ResourceDescription& other) const;
		bool operator!=(const ResourceDescription& other) const;
		bool operator<(const ResourceDescription& other) const;
	};

	struct BuiltResourceDescription
	{
		std::string BuiltResourceFilePath;
		bool IsUpToDate;
	};

	class ResourceDatabase
	{
		PathHandler* m_PathHandler;

		std::map<size_t, ResourceDescription> m_Hashes;

		std::string GetBuiltResourceFilePath(const ResourceDescription& description);

	public:

		ResourceDatabase(PathHandler* pathHandler);

		void GetBuiltResourceDescription(
			const ResourceDescription& resourceDescription,
			BuiltResourceDescription& builtResourceDescription);
	};
}

#endif