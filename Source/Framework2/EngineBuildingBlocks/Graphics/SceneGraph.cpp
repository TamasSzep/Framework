// EngineBuildingBlocks/Graphics/SceneGraph.cpp

#include <EngineBuildingBlocks/Graphics/SceneGraph.h>

#include <Core/System/Filesystem.h>
#include <Core/System/SimpleIO.h>
#include <EngineBuildingBlocks/ErrorHandling.h>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;

template <typename T, typename C>
inline T* GetContainerData(C& container, const char* name)
{
	for (size_t i = 0; i < container.size(); i++) if (container[i].second == name) return &container[i].first;
	return nullptr;
}

void SceneGraph::Get(Camera& camera, const char* name) const
{
	auto pCameraData = GetContainerData<const CameraData>(Cameras, name);
	if (pCameraData != nullptr) camera.SetData(*pCameraData);
}

void SceneGraph::Set(const Camera& camera, const char* name)
{
	auto pCameraData = GetContainerData<CameraData>(Cameras, name);
	if (pCameraData == nullptr) Cameras.push_back({ camera.GetData(), name });
	else *pCameraData = camera.GetData();
}

void SceneGraph::Synchronize(Camera& camera, const char* name)
{
	auto pCameraData = GetContainerData<const CameraData>(Cameras, name);
	if (pCameraData == nullptr) Cameras.push_back({ camera.GetData(), name });
	else camera.SetData(*pCameraData);
}

inline std::string GetSceneGraphPath(EngineBuildingBlocks::PathHandler* pathHandler, const std::string& name)
{
	return pathHandler->GetPathFromResourcesDirectory("Scenes/" + name + ".xml");
}

void SceneGraph::Load(EngineBuildingBlocks::PathHandler* pathHandler, const std::string& name)
{
	auto path = GetSceneGraphPath(pathHandler, name);
	if (Core::FileExists(path))
	{
		std::string text;
		Core::ReadAllText(path, text);
		Core::StartDeserializeSXML(text, *this, "Scene");
	}
}

void SceneGraph::Save(EngineBuildingBlocks::PathHandler* pathHandler, const std::string& name) const
{
	std::string str;
	Core::StartSerializeSXML(str, *this, "Scene");
	Core::WriteAllText(GetSceneGraphPath(pathHandler, name), str);
}

void SceneGraph::SerializeSXML(Core::SerializationSXMLData& data) const
{
	Core_SerializeSXML(data, Cameras);
}

void SceneGraph::DeserializeSXML(Core::DeserializationSXMLData& data)
{
	Core_DeserializeSXML(data, Cameras);
}