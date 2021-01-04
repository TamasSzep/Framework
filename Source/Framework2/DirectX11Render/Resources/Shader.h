// DirectX11Render/Resources/Shader.h

#ifndef _DIRECTX11RENDER_SHADER_H_INCLUDED_
#define _DIRECTX11RENDER_SHADER_H_INCLUDED_

#include <Core/DataStructures/Pool.hpp>
#include <WindowsApplication/DirectX/Shader.h>
#include <DirectX11Render/DirectX11Includes.h>

#include <map>

namespace EngineBuildingBlocks
{
	class PathHandler;
}

namespace DirectX11Render
{
	class Shader
	{
		DirectXRender::ShaderDescription m_Description;

		ComPtr<ID3DBlob> m_Blob;
		void* m_ShaderPtr;

	public:

		Shader();
		Shader(const DirectXRender::ShaderDescription& description);
		~Shader();

		void Initialize(ID3D11Device* device, const EngineBuildingBlocks::PathHandler& pathHandler);

		const DirectXRender::ShaderDescription& GetDescription() const;

		ID3DBlob* GetBlob() const;
		const void* GetShaderPtr() const;
	};

	class ShaderManager
	{
		EngineBuildingBlocks::PathHandler* m_PathHandler;

		Core::ResourcePoolU<Shader> m_Shaders;
		std::map<DirectXRender::ShaderDescription, Shader*> m_ShaderMap;

	public:

		ShaderManager(EngineBuildingBlocks::PathHandler* pathHandler);

		const Shader* GetShaderSimple(ID3D11Device* device,
			const DirectXRender::ShaderSimpleDescription& description, bool forceRecompilation = false);
		const Shader* GetShader(ID3D11Device* device,
			const DirectXRender::ShaderDescription& description, bool forceRecompilation = false);
	};
}

#endif