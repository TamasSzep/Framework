// DirectX12Render/Utilities/BackgroundCMRenderer.h

#include <DirectX12Render/Utilities/BackgroundCMRenderer.h>

using namespace DirectXRender;
using namespace DirectX12Render;

enum class BackgroundRS_RootParameterIndices
{
	RenderPassCBV = 0,
	TextureSRVDescriptorTable,

	COUNT
};

enum class BackgroundRS_ConstanRegisters
{
	RenderPassCBV = 0,
	TextureSRV = 0
};

inline RootSignature* GetBackgroundRootSignature(const UtilityInitializationContext& context)
{
	CD3DX12_DESCRIPTOR_RANGE ranges[1];
	CD3DX12_ROOT_PARAMETER rootParameters[(int)BackgroundRS_RootParameterIndices::COUNT];

	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, (int)BackgroundRS_ConstanRegisters::TextureSRV);

	rootParameters[(int)BackgroundRS_RootParameterIndices::RenderPassCBV].InitAsConstantBufferView(
		(int)BackgroundRS_ConstanRegisters::RenderPassCBV, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[(int)BackgroundRS_RootParameterIndices::TextureSRVDescriptorTable].
		InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

	// Note that we cannot set directly the shader resource view in the root signature because:
	// "SRV or UAV root descriptors can only be Raw or Structured buffers."

	auto sampler = StaticSamplerHelper::GetDefaultTrilinearSamplerDescription(0);
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootSignatureDescription rootSignatureDesc;
	rootSignatureDesc.Initialize(_countof(rootParameters), rootParameters, 1, &sampler,
		RootSignatureDescription::c_IA_VertexPixelShader_Flags);

	return context.DX12M->RootSignatureManager.GetRootSignature(context.Device, rootSignatureDesc);
}

inline GraphicsPipelineStateObject* GetBackgroundPSO(const UtilityInitializationContext& context,
	const VertexInputLayout& vertexInputLayoutDescription)
{
	// Getting the root signature.
	auto pRootSignature = GetBackgroundRootSignature(context);

	auto vertexShader = context.DX12M->ShaderManager.GetShaderSimple(context.Device,
		{ "BackgroundCubemap.hlsl", "VSMain", ShaderType::Vertex });
	auto pixelShader = context.DX12M->ShaderManager.GetShaderSimple(context.Device,
		{ "BackgroundCubemap.hlsl", "PSMain", ShaderType::Pixel });

	GraphicsPipelineStateObjectDescription description;
	description.SampleDesc.Count = context.MultisampleCount;
	description.PRootSignature = pRootSignature;
	description.PVertexShader = vertexShader;
	description.PPixelShader = pixelShader;
	description.PVertexInputLayout = &vertexInputLayoutDescription;
	description.RasterizerState.DisableCulling();
	description.DepthStencilState.DisableDepthWriting();

	return context.DX12M->PipelineStateObjectManager.GetGraphicsPipelineStateObject(context.Device, description);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

BackgroundCMRenderer::BackgroundCMRenderer()
	: m_BackgroundPSO(nullptr)
{
}

void BackgroundCMRenderer::Initialize(const UtilityInitializationContext& context)
{
	// Creating quad primitive.
	m_BackgroundPrimitive = context.DX12M->PrimitiveManager.GetIndexedPrimitive(
			EngineBuildingBlocks::Graphics::QuadDescription(), &context.DX12M->TransferBufferManager,
			context.CommandList, context.Device);

	// Creating PSO.
	m_BackgroundPSO = GetBackgroundPSO(context, m_BackgroundPrimitive.PVertexBuffer->GetInputLayout());
}

unsigned BackgroundCMRenderer::AddTexture(const UtilityInitializationContext& context,
	StreamedDescriptorHeap& CBV_SRV_UAV_DescriptorHeap, const char* name)
{
	auto result = context.DX12M->ConstantTextureManager.GetTexture2DFromFile(context.Device,
		context.CommandList, &CBV_SRV_UAV_DescriptorHeap, name);
	unsigned index = m_BackgroundCubemaps.Add(result.Texture);
	if (result.IsTextureCreated) result.Texture->TransitionToPixelShaderResource(context.CommandList);
	return index;
}

void BackgroundCMRenderer::RemoveTexture(unsigned index)
{
	m_BackgroundCubemaps.Remove(index);
}

void BackgroundCMRenderer::ClearTextures()
{
	m_BackgroundCubemaps.Clear();
}

void BackgroundCMRenderer::Render(const UtilityRenderContext& context, ConstantBuffer* renderPassConstantBuffer,
	unsigned textureIndex, unsigned constantBufferIndex)
{
	auto commandList = context.CommandList;

	commandList->SetPipelineState(m_BackgroundPSO->GetPipelineStateObject());
	commandList->SetGraphicsRootSignature(m_BackgroundPSO->GetRootSignature()->GetRootSignature());

	commandList->SetGraphicsRootDescriptorTable(
		(int)BackgroundRS_RootParameterIndices::TextureSRVDescriptorTable,
		m_BackgroundCubemaps[textureIndex]->GetShaderResourceView().GPUDescriptorHandle);

	// This is frame/render pass level, but must be done after the root signature has been bound.
	commandList->SetGraphicsRootConstantBufferView((int)BackgroundRS_RootParameterIndices::RenderPassCBV,
		renderPassConstantBuffer->GetConstantBufferGPUVirtualAddress(constantBufferIndex));

	auto& primitive = m_BackgroundPrimitive;

	SetIndexedPrimitiveForInputAssembler(primitive, commandList);

	// Drawing primitive.
	commandList->DrawIndexedInstanced(primitive.CountIndices, 1, primitive.BaseIndex, primitive.BaseVertex, 0);
}