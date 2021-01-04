// DirectX11Render/Resources/ResourceState.h

#ifndef _DIRECTX11RENDER_RESOURCESTATE_INCLUDED_
#define _DIRECTX11RENDER_RESOURCESTATE_INCLUDED_

#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <Core/DataStructures/ResourceUnorderedVector.hpp>
#include <DirectX11Render/Resources/PipelineState.h>

#include <map>

namespace DirectX11Render
{
	struct ResourceState
	{
		DirectXRender::ShaderFlagType Shaders;
		unsigned char CountCBs;
		unsigned char CountSRVs;
		unsigned char PointerStartIndex[6];
		Core::SimpleTypeVectorU<void*> Pointers;

		ResourceState();
		ResourceState(DirectXRender::ShaderFlagType shaders,
			unsigned char countCBs, unsigned char countSRVs);

		void AddConstantBuffer(ID3D11Buffer* buffer, unsigned char index,
			DirectXRender::ShaderFlagType shaders = DirectXRender::ShaderFlagType::All);
		void AddShaderResourceView(ID3D11ShaderResourceView* srv, unsigned char index,
			DirectXRender::ShaderFlagType shaders = DirectXRender::ShaderFlagType::All);

		void SetForContext(ID3D11DeviceContext* context);

		bool operator==(const ResourceState& other) const;
		bool operator!=(const ResourceState& other) const;
		bool operator<(const ResourceState& other) const;
	};

	class ResourceStateManager
	{
		Core::ResourceUnorderedVectorU<DirectX11Render::ResourceState> m_ResourceStates;
		std::map<DirectX11Render::ResourceState, unsigned> m_ResourceStateIndices;

	public:

		ResourceState& GetResourceState(unsigned index);
		unsigned AddResourceState(const ResourceState& resourceState);
	};
}

#endif