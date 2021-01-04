// DirectX12Render/Utilities/MSAAResolver.h

#ifndef _DIRECTX12RENDER_MSAARESOLVER_H_INCLUDED_
#define _DIRECTX12RENDER_MSAARESOLVER_H_INCLUDED_

#include <EngineBuildingBlocks/Math/GLM.h>
#include <DirectX12Render/Utilities/Utility.h>
#include <DirectX12Render/Resources/ConstantBuffer.h>
#include <DirectX12Render/Resources/DescriptorHeap.h>

namespace EngineBuildingBlocks
{
	class PathHandler;
}

namespace DirectX12Render
{
	class PrimitiveManager;
	class RootSignature;
	class RootSignatureManager;
	class ShaderManager;
	class ComputePipelineStateObject;
	class PipelineStateObjectManager;
	class Texture2DBase;

	enum class ResolveType
	{
		APICall, ResolveDepth
	};

	class MSAAResolver
	{
		RootSignature* m_RootSignature;
		ComputePipelineStateObject* m_PipelineStateObject;

		std::vector<DirectX12Render::ConstantBuffer> m_ConstantBuffers;
		DirectX12Render::StreamedDescriptorHeap m_SRV_UAV_DescriptorHeap;

		unsigned m_MultisampleCount;

		void CreateRootSignature(const UtilityInitializationContext& context);
		void CreatePipelineStateObject(const UtilityInitializationContext& context);

		bool IsResolvableWithAPICall(Texture2DBase* sourceTexture,
			Texture2DBase* targetTexture);
		void ResolveWithAPICall(ID3D12GraphicsCommandList* commandList,
			Texture2DBase* sourceTexture, Texture2DBase* targetTexture);
		void ResolveDepth(ID3D12GraphicsCommandList* commandList,
			Texture2DBase* sourceTexture, Texture2DBase* targetTexture,
			unsigned frameIndex);

	public:

		MSAAResolver();

		void Initialize(const UtilityInitializationContext& context);
		void RegisterTextures(ID3D12Device* device,
			Texture2DBase* sourceTexture, Texture2DBase* targetTexture);

		void Update(const UtilityUpdateContext& context);

		void Resolve(ID3D12GraphicsCommandList* commandList,
			Texture2DBase* sourceTexture, Texture2DBase* targetTexture,
			unsigned frameIndex,
			ResolveType resolveType);
	};
}

#endif