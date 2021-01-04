// DirectX11Render/Utilities/BackgroundCMRenderer.h

#include <DirectX11Render/Utilities/BackgroundCMRenderer.h>

using namespace DirectXRender;
using namespace DirectX11Render;

inline unsigned GetBackgroundPS(const UtilityInitializationContext& context,
	const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayoutDescription)
{
	auto vs = context.DX11M->ShaderManager.GetShaderSimple(context.Device,
	{ "BackgroundCubemap.hlsl", "VSMain", ShaderType::Vertex });
	auto ps = context.DX11M->ShaderManager.GetShaderSimple(context.Device,
	{ "BackgroundCubemap.hlsl", "PSMain", ShaderType::Pixel });

	SamplerStateDescription textureSampler;
	textureSampler.SetToWrap();
	textureSampler.SetToTrilinear();

	PipelineStateDescription description;
	description.Shaders = { vs, ps };
	description.InputLayout = VertexInputLayout::Create(inputLayoutDescription);
	description.RasterizerState.DisableCulling();
	description.DepthStencilState.DisableDepthWriting();
	description.SamplerStates = { { textureSampler, ShaderFlagType::Pixel } };

	return context.DX11M->PipelineStateManager.GetPipelineStateIndex(context.Device, description);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

BackgroundCMRenderer::BackgroundCMRenderer()
	: m_PipelineStateIndex(Core::c_InvalidIndexU)
{
}

void BackgroundCMRenderer::Initialize(const UtilityInitializationContext& context)
{
	// Creating quad primitive.
	m_Primitive = context.DX11M->PrimitiveManager.GetIndexedPrimitive(
			EngineBuildingBlocks::Graphics::QuadDescription(), context.Device);

	// Creating pipeline state.
	m_PipelineStateIndex = GetBackgroundPS(context, m_Primitive.PVertexBuffer->GetInputLayout());
}

unsigned BackgroundCMRenderer::AddTexture(const UtilityInitializationContext& context, const char* name)
{
	return m_BackgroundCubemaps.Add(
		context.DX11M->ConstantTextureManager.GetTexture2DFromFile(context.Device, name).Texture);
}

void BackgroundCMRenderer::RemoveTexture(unsigned index)
{
	m_BackgroundCubemaps.Remove(index);
}

void BackgroundCMRenderer::ClearTextures()
{
	m_BackgroundCubemaps.Clear();
}

void BackgroundCMRenderer::Render(const UtilityRenderContext& context, ConstantBuffer& renderPassConstantBuffer,
	unsigned textureIndex, unsigned constantBufferIndex)
{
	auto d3dContext = context.Context;

	context.DX11M->PipelineStateManager.GetPipelineState(m_PipelineStateIndex).SetForContext(d3dContext);

	renderPassConstantBuffer.Update(d3dContext, constantBufferIndex);
	ID3D11Buffer* cbs[] = { renderPassConstantBuffer.GetBuffer() };
	d3dContext->VSSetConstantBuffers(0, 1, cbs);

	ID3D11ShaderResourceView* srvs[] = { m_BackgroundCubemaps[textureIndex]->GetSRV() };
	d3dContext->PSSetShaderResources(0, 1, srvs);

	DrawPrimitive(m_Primitive, d3dContext);
}