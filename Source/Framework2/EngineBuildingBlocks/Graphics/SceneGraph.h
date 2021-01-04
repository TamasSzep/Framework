// EngineBuildingBlocks/Graphics/SceneGraph.h

#ifndef _ENGINEBUILDINGBLOCKS_SCENEGRAPH_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_SCENEGRAPH_H_INCLUDED_

#include <Core/SimpleXMLSerialization.hpp>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/Graphics/Camera/Camera.h>

#include <vector>
#include <utility>
#include <string>

namespace EngineBuildingBlocks
{
	namespace Graphics
	{
		struct SceneGraph
		{
			// Vector of camera - name pairs.
			std::vector<std::pair<CameraData, std::string>> Cameras;

			void Get(Camera& camera, const char* name) const;
			void Set(const Camera& camera, const char* name);
			void Synchronize(Camera& camera, const char* name);
		
			void Load(EngineBuildingBlocks::PathHandler* pathHandler, const std::string& name);
			void Save(EngineBuildingBlocks::PathHandler* pathHandler, const std::string& name) const;
		
			void SerializeSXML(Core::SerializationSXMLData& data) const;
			void DeserializeSXML(Core::DeserializationSXMLData& data);
		};
	}
}

#endif