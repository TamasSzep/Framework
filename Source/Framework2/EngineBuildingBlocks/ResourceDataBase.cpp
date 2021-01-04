// EngineBuildingBlocks/ResourceDatabase.cpp

#include <EngineBuildingBlocks/ResourceDatabase.h>

#include <Core/String.hpp>
#include <Core/Comparison.h>
#include <Core/System/Filesystem.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/ErrorHandling.h>

#include <functional>
#include <sstream>

using namespace EngineBuildingBlocks;

ResourceDescription::ResourceDescription()
	: IsBuiltResource(false)
{
}

bool ResourceDescription::operator==(const ResourceDescription& other) const
{
	NumericalEqualCompareBlock(IsBuiltResource);
	StructureEqualCompareBlock(ResourceFilePath);
	StructureEqualCompareBlock(DependencyPaths);
	StructureEqualCompareBlock(SerializedBuildOptions);
	return true;
}

bool ResourceDescription::operator!=(const ResourceDescription& other) const
{
	return !(*this == other);
}

bool ResourceDescription::operator<(const ResourceDescription& other) const
{
	NumericalLessCompareBlock(IsBuiltResource);
	StructureLessCompareBlock(ResourceFilePath);
	StructureLessCompareBlock(DependencyPaths);
	StructureLessCompareBlock(SerializedBuildOptions);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceDatabase::ResourceDatabase(PathHandler* pathHandler)
	: m_PathHandler(pathHandler)
{
}

std::string ResourceDatabase::GetBuiltResourceFilePath(const ResourceDescription& description)
{
	auto& filePath = description.ResourceFilePath;
	auto& serializedBuildOptions = description.SerializedBuildOptions;

	unsigned filePathLength = static_cast<unsigned>(filePath.length());
	unsigned bufferLength = serializedBuildOptions.GetSize();

	// Serializing data to buffer.
	std::string buffer;
	buffer.append(reinterpret_cast<const char*>(&filePathLength), sizeof(unsigned));
	buffer.append(reinterpret_cast<const char*>(filePath.c_str()), filePathLength);
	buffer.append(reinterpret_cast<const char*>(&bufferLength), sizeof(unsigned));
	buffer.append(reinterpret_cast<const char*>(serializedBuildOptions.GetArray()),
		bufferLength);

	auto hashValue = std::hash<std::string>()(buffer);

	// Checking hash collisions.
	// Note that this solution is not complete: it is possible that the same hash and name is assigned
	// to two different resources, however we save/load them in different program instances.
	auto hIt = m_Hashes.find(hashValue);
	if (hIt == m_Hashes.end())
	{
		m_Hashes[hashValue] = description;
	}
	else if (hIt->second != description)
	{
		EngineBuildingBlocks::RaiseException("A hash collision has occured.");
	}

	// We also use the file name to make built resources to resource connection readable for humans.
	std::stringstream ss;
	ss << Core::Replace(Core::GetFileName(filePath), ".", "_") << "_" << hashValue << ".bin";

	return m_PathHandler->GetPathFromBuiltResourcesDirectory(ss.str());
}

void ResourceDatabase::GetBuiltResourceDescription(
	const ResourceDescription& resourceDescription,
	BuiltResourceDescription& builtResourceDescription)
{
	auto& filePath = resourceDescription.ResourceFilePath;
	if (resourceDescription.IsBuiltResource)
	{
		builtResourceDescription.BuiltResourceFilePath = filePath;
		builtResourceDescription.IsUpToDate = true;
	}
	else
	{
		auto builtFilePath = GetBuiltResourceFilePath(resourceDescription);
		bool isUptoDate = Core::FileExists(builtFilePath);
		if (isUptoDate)
		{
			auto resourceLastWriteTime = Core::GetLastWriteTime(filePath);
			auto builtResourceCacheLastWriteTime = Core::GetLastWriteTime(builtFilePath);

			isUptoDate = (resourceLastWriteTime < builtResourceCacheLastWriteTime);

			if (isUptoDate)
			{
				auto& dependencyPaths = resourceDescription.DependencyPaths;
				for (size_t i = 0; i < dependencyPaths.size(); i++)
				{
					auto dependencyLastWriteTime = Core::GetLastWriteTime(dependencyPaths[i]);

					isUptoDate = (dependencyLastWriteTime < builtResourceCacheLastWriteTime);

					if (!isUptoDate)
					{
						break;
					}
				}
			}
		}
		builtResourceDescription.BuiltResourceFilePath = builtFilePath;
		builtResourceDescription.IsUpToDate = isUptoDate;
	}
}