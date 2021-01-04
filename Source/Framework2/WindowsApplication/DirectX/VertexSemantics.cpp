// WindowsApplication/DirectX/VertexSemantics.cpp

#include <WindowsApplication/DirectX/VertexSemantics.h>

#include <EngineBuildingBlocks/Graphics/Primitives/Primitive.h>
#include <EngineBuildingBlocks/ErrorHandling.h>

const std::map<std::string, std::string> c_SematicMap =
{
	{ EngineBuildingBlocks::Graphics::c_PositionVertexElement.Name, "POSITION" },
	{ EngineBuildingBlocks::Graphics::c_TextureCoordinateVertexElement.Name, "TEXCOORD" },
	{ EngineBuildingBlocks::Graphics::c_NormalVertexElement.Name, "NORMAL" },
	{ EngineBuildingBlocks::Graphics::c_TangentVertexElement.Name, "TANGENT" },
	{ EngineBuildingBlocks::Graphics::c_BitangentVertexElement.Name, "BINORMAL" },
	{ EngineBuildingBlocks::Graphics::c_VertexColorVertexElement.Name, "COLOR" },
};

namespace DirectXRender
{
	const char* GetD3DSemanticName(const std::string& name)
	{
		auto it = c_SematicMap.find(name);
		if (it == c_SematicMap.end())
		{
			return "TEXCOORD";
		}
		else
		{
			return it->second.c_str();
		}
	}

	unsigned GetD3DVertexElementSemanticIndex(const std::string& d3dSemanticName,
		std::map<std::string, unsigned>& semanticIndices)
	{
		auto indexIt = semanticIndices.find(d3dSemanticName);
		if (indexIt == semanticIndices.end())
		{
			semanticIndices[d3dSemanticName] = 1;
			return 0;
		}

		return indexIt->second++;
	}

	const char* GetStoredD3DSemanticString(const char* d3dSemanticName)
	{
		for (auto& it : c_SematicMap)
		{
			if (it.second == d3dSemanticName)
			{
				return it.second.c_str();
			}
		}

		std::stringstream ss;
		ss << "Unknown vertex semantics : '" << d3dSemanticName << "'.";
		EngineBuildingBlocks::RaiseException(ss);

		return nullptr;
	}
}