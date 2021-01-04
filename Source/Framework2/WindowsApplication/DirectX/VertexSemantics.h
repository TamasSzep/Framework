// WindowsApplication/DirectX/VertexSemantics.h

#ifndef _WINDOWSAPPLICATION_VERTEXSEMANTICS_H_INCLUDED_
#define _WINDOWSAPPLICATION_VERTEXSEMANTICS_H_INCLUDED_

#include <string>
#include <map>

namespace DirectXRender
{
	const char* GetD3DSemanticName(const std::string& name);
	unsigned GetD3DVertexElementSemanticIndex(const std::string& d3dSemanticName,
			std::map<std::string, unsigned>& semanticIndices);
	const char* GetStoredD3DSemanticString(const char* d3dSemanticName);
}

#endif