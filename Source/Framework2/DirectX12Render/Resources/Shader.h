// DirectX12Render/Resources/Shader.h

#ifndef _DIRECTX12RENDER_SHADER_H_INCLUDED_
#define _DIRECTX12RENDER_SHADER_H_INCLUDED_

#include <Core/DataStructures/Pool.hpp>
#include <WindowsApplication/DirectX/Shader.h>
#include <DirectX12Render/DirectX12Includes.h>

#include <map>

namespace EngineBuildingBlocks
{
	class PathHandler;
}

namespace DirectX12Render
{
	class Shader
	{
		DirectXRender::ShaderDescription m_Description;

		ComPtr<ID3DBlob> m_Blob;
		CD3DX12_SHADER_BYTECODE m_ByteCode;

	public:

		Shader();
		Shader(const DirectXRender::ShaderDescription& description);

		void Initialize(ID3D12Device* device, const EngineBuildingBlocks::PathHandler& pathHandler);

		const D3D12_SHADER_BYTECODE& GetByteCode() const;
	};

	class ShaderManager
	{
		EngineBuildingBlocks::PathHandler* m_PathHandler;

		Core::ResourcePoolU<Shader> m_Shaders;
		std::map<DirectXRender::ShaderDescription, Shader*> m_ShaderMap;

	public:

		ShaderManager(EngineBuildingBlocks::PathHandler* pathHandler);

		const Shader* GetShaderSimple(ID3D12Device* device,
			const DirectXRender::ShaderSimpleDescription& description, bool forceRecompilation = false);
		const Shader* GetShader(ID3D12Device* device,
			const DirectXRender::ShaderDescription& description, bool forceRecompilation = false);
	};
}

#endif