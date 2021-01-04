// EngineBuildingBlocks/Graphics/Primitives/AssimpExtensions/SXMLSerialization.h

#ifndef _ENGINEBUILDINGBLOCKS_SXMLSERIALIZATION_H_
#define _ENGINEBUILDINGBLOCKS_SXMLSERIALIZATION_H_

struct aiScene;

namespace EngineBuildingBlocks
{
	namespace Graphics
	{
		aiScene* ImportAssimpSceneSXML(const char* path);
		void ExportAssimpSceneSXML(const aiScene* scene, const char* path);
	}
}

#endif