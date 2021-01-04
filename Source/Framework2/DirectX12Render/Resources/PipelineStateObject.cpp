// DirectX12Render/Resources/PipelineStateObject.cpp

#include <DirectX12Render/Resources/PipelineStateObject.h>

#include <Core/Comparison.h>
#include <WindowsApplication/DirectX/VertexSemantics.h>
#include <DirectX12Render/Resources/RootSignature.h>
#include <DirectX12Render/Resources/Shader.h>

using namespace DirectXRender;
using namespace DirectX12Render;

RasterizerDescription::RasterizerDescription()
{
	SetToDefault();
}

void RasterizerDescription::SetToDefault()
{
	SetToD3DDefault();
	Description.FrontCounterClockwise = TRUE;
}

void RasterizerDescription::SetToD3DDefault()
{
	Description = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
}

void RasterizerDescription::DisableCulling()
{
	Description.CullMode = D3D12_CULL_MODE_NONE;
}

void RasterizerDescription::SetWireframeFillMode()
{
	Description.FillMode = D3D12_FILL_MODE_WIREFRAME;
}

bool RasterizerDescription::operator==(const RasterizerDescription& other) const
{
	NumericalEqualCompareBlock(Description.FillMode);
	NumericalEqualCompareBlock(Description.CullMode);
	NumericalEqualCompareBlock(Description.FrontCounterClockwise);
	NumericalEqualCompareBlock(Description.DepthBias);
	NumericalEqualCompareBlock(Description.DepthBiasClamp);
	NumericalEqualCompareBlock(Description.SlopeScaledDepthBias);
	NumericalEqualCompareBlock(Description.DepthClipEnable);
	NumericalEqualCompareBlock(Description.MultisampleEnable);
	NumericalEqualCompareBlock(Description.AntialiasedLineEnable);
	NumericalEqualCompareBlock(Description.ForcedSampleCount);
	NumericalEqualCompareBlock(Description.ConservativeRaster);
	return true;
}

bool RasterizerDescription::operator!=(const RasterizerDescription& other) const
{
	return !(*this == other);
}

bool RasterizerDescription::operator<(const RasterizerDescription& other) const
{
	NumericalLessCompareBlock(Description.FillMode);
	NumericalLessCompareBlock(Description.CullMode);
	NumericalLessCompareBlock(Description.FrontCounterClockwise);
	NumericalLessCompareBlock(Description.DepthBias);
	NumericalLessCompareBlock(Description.DepthBiasClamp);
	NumericalLessCompareBlock(Description.SlopeScaledDepthBias);
	NumericalLessCompareBlock(Description.DepthClipEnable);
	NumericalLessCompareBlock(Description.MultisampleEnable);
	NumericalLessCompareBlock(Description.AntialiasedLineEnable);
	NumericalLessCompareBlock(Description.ForcedSampleCount);
	NumericalLessCompareBlock(Description.ConservativeRaster);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

DepthStencilDescription::DepthStencilDescription()
{
	SetToD3DDefault();
}

void DepthStencilDescription::SetToD3DDefault()
{
	Description = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
}

void DepthStencilDescription::DisableDepthWriting()
{
	Description.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
}

bool DepthStencilDescription::operator==(const DepthStencilDescription& other) const
{
	NumericalEqualCompareBlock(Description.DepthEnable);
	NumericalEqualCompareBlock(Description.DepthWriteMask);
	NumericalEqualCompareBlock(Description.DepthFunc);
	NumericalEqualCompareBlock(Description.StencilEnable);
	NumericalEqualCompareBlock(Description.StencilReadMask);
	NumericalEqualCompareBlock(Description.StencilWriteMask);
	NumericalEqualCompareBlock(Description.FrontFace.StencilFunc);
	NumericalEqualCompareBlock(Description.FrontFace.StencilDepthFailOp);
	NumericalEqualCompareBlock(Description.FrontFace.StencilFailOp);
	NumericalEqualCompareBlock(Description.FrontFace.StencilPassOp);
	NumericalEqualCompareBlock(Description.BackFace.StencilFunc);
	NumericalEqualCompareBlock(Description.BackFace.StencilDepthFailOp);
	NumericalEqualCompareBlock(Description.BackFace.StencilFailOp);
	NumericalEqualCompareBlock(Description.BackFace.StencilPassOp);
	return true;
}

bool DepthStencilDescription::operator!=(const DepthStencilDescription& other) const
{
	return !(*this == other);
}

bool DepthStencilDescription::operator<(const DepthStencilDescription& other) const
{
	NumericalLessCompareBlock(Description.DepthEnable);
	NumericalLessCompareBlock(Description.DepthWriteMask);
	NumericalLessCompareBlock(Description.DepthFunc);
	NumericalLessCompareBlock(Description.StencilEnable);
	NumericalLessCompareBlock(Description.StencilReadMask);
	NumericalLessCompareBlock(Description.StencilWriteMask);
	NumericalLessCompareBlock(Description.FrontFace.StencilFunc);
	NumericalLessCompareBlock(Description.FrontFace.StencilDepthFailOp);
	NumericalLessCompareBlock(Description.FrontFace.StencilFailOp);
	NumericalLessCompareBlock(Description.FrontFace.StencilPassOp);
	NumericalLessCompareBlock(Description.BackFace.StencilFunc);
	NumericalLessCompareBlock(Description.BackFace.StencilDepthFailOp);
	NumericalLessCompareBlock(Description.BackFace.StencilFailOp);
	NumericalLessCompareBlock(Description.BackFace.StencilPassOp);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

BlendDescription::BlendDescription()
{
	SetToD3DDefault();
}

void BlendDescription::SetToD3DDefault()
{
	Description = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
}

void BlendDescription::SetToBlending()
{
	Description.AlphaToCoverageEnable = false;
	Description.IndependentBlendEnable = false;
	Description.RenderTarget[0].BlendEnable = true;
	Description.RenderTarget[0].LogicOpEnable = false;
	Description.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	Description.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}

void BlendDescription::EnableAlphaToCoverage()
{
	Description.AlphaToCoverageEnable = true;
}

void BlendDescription::SetToNonpremultipliedAlphaBlending()
{
	SetToBlending();
	Description.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	Description.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	Description.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	Description.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	Description.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	Description.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
}

bool BlendDescription::operator==(const BlendDescription& other) const
{
	NumericalEqualCompareBlock(Description.AlphaToCoverageEnable);
	NumericalEqualCompareBlock(Description.IndependentBlendEnable);
	for (unsigned i = 0; i < 8; i++)
	{
		NumericalEqualCompareBlock(Description.RenderTarget[i].BlendEnable);
		NumericalEqualCompareBlock(Description.RenderTarget[i].LogicOpEnable);
		NumericalEqualCompareBlock(Description.RenderTarget[i].SrcBlend);
		NumericalEqualCompareBlock(Description.RenderTarget[i].DestBlend);
		NumericalEqualCompareBlock(Description.RenderTarget[i].BlendOp);
		NumericalEqualCompareBlock(Description.RenderTarget[i].SrcBlendAlpha);
		NumericalEqualCompareBlock(Description.RenderTarget[i].DestBlendAlpha);
		NumericalEqualCompareBlock(Description.RenderTarget[i].BlendOpAlpha);
		NumericalEqualCompareBlock(Description.RenderTarget[i].LogicOp);
		NumericalEqualCompareBlock(Description.RenderTarget[i].RenderTargetWriteMask);
	}
	return true;
}

bool BlendDescription::operator!=(const BlendDescription& other) const
{
	return !(*this == other);
}

bool BlendDescription::operator<(const BlendDescription& other) const
{
	NumericalLessCompareBlock(Description.AlphaToCoverageEnable);
	NumericalLessCompareBlock(Description.IndependentBlendEnable);
	for (unsigned i = 0; i < 8; i++)
	{
		NumericalLessCompareBlock(Description.RenderTarget[i].BlendEnable);
		NumericalLessCompareBlock(Description.RenderTarget[i].LogicOpEnable);
		NumericalLessCompareBlock(Description.RenderTarget[i].SrcBlend);
		NumericalLessCompareBlock(Description.RenderTarget[i].DestBlend);
		NumericalLessCompareBlock(Description.RenderTarget[i].BlendOp);
		NumericalLessCompareBlock(Description.RenderTarget[i].SrcBlendAlpha);
		NumericalLessCompareBlock(Description.RenderTarget[i].DestBlendAlpha);
		NumericalLessCompareBlock(Description.RenderTarget[i].BlendOpAlpha);
		NumericalLessCompareBlock(Description.RenderTarget[i].LogicOp);
		NumericalLessCompareBlock(Description.RenderTarget[i].RenderTargetWriteMask);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

StreamOutputDescription::StreamOutputDescription()
	: RasterizedStream(0)
{
}

void StreamOutputDescription::Initialize(const D3D12_STREAM_OUTPUT_DESC& description)
{
	for (unsigned i = 0; i < description.NumEntries; i++)
	{
		StreamOutputDeclarationEntries.PushBack(description.pSODeclaration[i]);
		StreamOutputDeclarationEntries.GetLastElement().SemanticName
			= GetStoredD3DSemanticString(description.pSODeclaration[i].SemanticName);
	}
	if (description.NumStrides > 0)
	{
		BufferStrides.PushBack(description.pBufferStrides, description.NumStrides);
	}
	RasterizedStream = description.RasterizedStream;
}

D3D12_STREAM_OUTPUT_DESC StreamOutputDescription::ToD3DDescription() const
{
	D3D12_STREAM_OUTPUT_DESC description;
	description.NumEntries = StreamOutputDeclarationEntries.GetSize();
	description.pSODeclaration = StreamOutputDeclarationEntries.GetArray();
	description.NumStrides = BufferStrides.GetSize();
	description.pBufferStrides = BufferStrides.GetArray();
	description.RasterizedStream = RasterizedStream;
	return description;
}

bool StreamOutputDescription::operator==(const StreamOutputDescription& other) const
{
	NumericalEqualCompareBlock(StreamOutputDeclarationEntries.GetSize());
	for (unsigned i = 0; i < StreamOutputDeclarationEntries.GetSize(); i++)
	{
		NumericalEqualCompareBlock(StreamOutputDeclarationEntries[i].Stream);
		CStringEqualCompareBlock(StreamOutputDeclarationEntries[i].SemanticName);
		NumericalEqualCompareBlock(StreamOutputDeclarationEntries[i].SemanticIndex);
		NumericalEqualCompareBlock(StreamOutputDeclarationEntries[i].StartComponent);
		NumericalEqualCompareBlock(StreamOutputDeclarationEntries[i].ComponentCount);
		NumericalEqualCompareBlock(StreamOutputDeclarationEntries[i].OutputSlot);
	}
	StructureEqualCompareBlock(BufferStrides);
	NumericalEqualCompareBlock(RasterizedStream);
	return true;
}

bool StreamOutputDescription::operator!=(const StreamOutputDescription& other) const
{
	return !(*this == other);
}

bool StreamOutputDescription::operator<(const StreamOutputDescription& other) const
{
	NumericalLessCompareBlock(StreamOutputDeclarationEntries.GetSize());
	for (unsigned i = 0; i < StreamOutputDeclarationEntries.GetSize(); i++)
	{
		NumericalLessCompareBlock(StreamOutputDeclarationEntries[i].Stream);
		CStringLessCompareBlock(StreamOutputDeclarationEntries[i].SemanticName);
		NumericalLessCompareBlock(StreamOutputDeclarationEntries[i].SemanticIndex);
		NumericalLessCompareBlock(StreamOutputDeclarationEntries[i].StartComponent);
		NumericalLessCompareBlock(StreamOutputDeclarationEntries[i].ComponentCount);
		NumericalLessCompareBlock(StreamOutputDeclarationEntries[i].OutputSlot);
	}
	StructureLessCompareBlock(BufferStrides);
	NumericalLessCompareBlock(RasterizedStream);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsWrappedPipelineData::GraphicsWrappedPipelineData()
	: PVertexShader(nullptr)
	, PHullShader(nullptr)
	, PDomainShader(nullptr)
	, PGeometryShader(nullptr)
	, PPixelShader(nullptr)
	, PVertexInputLayout(nullptr)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineStateObjectDescription::PipelineStateObjectDescription()
	: PRootSignature(nullptr)
{
	NodeMask = 0;
	Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	memset(&CachedPSO, 0, sizeof(D3D12_CACHED_PIPELINE_STATE));
}

bool PipelineStateObjectDescription::operator==(const PipelineStateObjectDescription& other) const
{
	NumericalEqualCompareBlock(PRootSignature);

	NumericalEqualCompareBlock(NodeMask);
	NumericalEqualCompareBlock(Flags);

	// Direct members with structures: we should refactor this.
	{
		// Cached pipeline state object.
		{
			ArrayEqualCompareBlock(CachedPSO.pCachedBlob,
				CachedPSO.CachedBlobSizeInBytes);
		}
	}
	return true;
}

bool PipelineStateObjectDescription::operator!=(const PipelineStateObjectDescription& other) const
{
	return !(*this == other);
}

bool PipelineStateObjectDescription::operator<(const PipelineStateObjectDescription& other) const
{
	NumericalLessCompareBlock(PRootSignature);

	NumericalLessCompareBlock(NodeMask);
	NumericalLessCompareBlock(Flags);

	// Cached pipeline state object.
	{
		ArrayLessCompareBlock(CachedPSO.pCachedBlob,
			CachedPSO.CachedBlobSizeInBytes);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsPipelineStateObjectDescription::GraphicsPipelineStateObjectDescription()
{
	SampleMask = UINT_MAX;
	IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	NumRenderTargets = 1;
	RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	DSVFormat = DXGI_FORMAT_D32_FLOAT;
	SampleDesc.Count = 1;
	SampleDesc.Quality = 0;
}

bool GraphicsPipelineStateObjectDescription::operator==(const GraphicsPipelineStateObjectDescription& other) const
{
	BaseEqualCompareBlock(PipelineStateObjectDescription, other);

	// Wrapped members.
	NumericalEqualCompareBlock(PVertexShader);
	NumericalEqualCompareBlock(PHullShader);
	NumericalEqualCompareBlock(PDomainShader);
	NumericalEqualCompareBlock(PGeometryShader);
	NumericalEqualCompareBlock(PPixelShader);
	PointerEqualCompareBlock(PVertexInputLayout);
	StructureEqualCompareBlock(RasterizerState);
	StructureEqualCompareBlock(DepthStencilState);
	StructureEqualCompareBlock(BlendState);
	StructureEqualCompareBlock(StreamOutput);

	// Simple direct members.
	NumericalEqualCompareBlock(SampleMask);
	NumericalEqualCompareBlock(IBStripCutValue);
	NumericalEqualCompareBlock(PrimitiveTopologyType);
	NumericalEqualCompareBlock(NumRenderTargets);
	for (unsigned i = 0; i < NumRenderTargets; i++)
	{
		NumericalEqualCompareBlock(RTVFormats[i]);
	}
	NumericalEqualCompareBlock(DSVFormat);

	// Direct members with structures: we should refactor this.
	{
		// Sample.
		{
			NumericalEqualCompareBlock(SampleDesc.Count);
			NumericalEqualCompareBlock(SampleDesc.Quality);
		}
	}

	return true;
}

bool GraphicsPipelineStateObjectDescription::operator!=(const GraphicsPipelineStateObjectDescription& other) const
{
	return !(*this == other);
}

bool GraphicsPipelineStateObjectDescription::operator<(const GraphicsPipelineStateObjectDescription& other) const
{
	BaseLessCompareBlock(PipelineStateObjectDescription, other);

	// Wrapped members.
	NumericalLessCompareBlock(PVertexShader);
	NumericalLessCompareBlock(PHullShader);
	NumericalLessCompareBlock(PDomainShader);
	NumericalLessCompareBlock(PGeometryShader);
	NumericalLessCompareBlock(PPixelShader);
	PointerLessCompareBlock(PVertexInputLayout);
	StructureLessCompareBlock(RasterizerState);
	StructureLessCompareBlock(DepthStencilState);
	StructureLessCompareBlock(BlendState);
	StructureLessCompareBlock(StreamOutput);

	// Simple direct members.
	NumericalLessCompareBlock(SampleMask);
	NumericalLessCompareBlock(IBStripCutValue);
	NumericalLessCompareBlock(PrimitiveTopologyType);
	NumericalLessCompareBlock(NumRenderTargets);
	for (unsigned i = 0; i < NumRenderTargets; i++)
	{
		NumericalLessCompareBlock(RTVFormats[i]);
	}
	NumericalLessCompareBlock(DSVFormat);

	// Direct members with structures: we should refactor this.
	{
		// Sample.
		{
			NumericalLessCompareBlock(SampleDesc.Count);
			NumericalLessCompareBlock(SampleDesc.Quality);
		}
	}

	return false;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateObjectDescription::ToD3DDescription() const
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC description;
	memset(&description, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	// Wrapped members.
	description.pRootSignature = PRootSignature->GetRootSignature();
	description.VS = PVertexShader->GetByteCode();
	if (PHullShader != nullptr)
	{
		description.HS = PHullShader->GetByteCode();
	}
	if (PDomainShader != nullptr)
	{
		description.DS = PDomainShader->GetByteCode();
	}
	if (PGeometryShader != nullptr)
	{
		description.GS = PGeometryShader->GetByteCode();
	}
	if (PPixelShader != nullptr)
	{
		description.PS = PPixelShader->GetByteCode();
	}
	description.InputLayout = PVertexInputLayout->InputLayoutDescription;
	description.RasterizerState = RasterizerState.Description;
	description.DepthStencilState = DepthStencilState.Description;
	description.BlendState = BlendState.Description;
	description.StreamOutput = StreamOutput.ToD3DDescription();

	// Direct members.
	description.SampleMask = SampleMask;
	description.IBStripCutValue = IBStripCutValue;
	description.PrimitiveTopologyType = PrimitiveTopologyType;
	description.NumRenderTargets = NumRenderTargets;
	for (unsigned i = 0; i < NumRenderTargets; i++)
	{
		description.RTVFormats[i] = RTVFormats[i];
	}
	description.DSVFormat = DSVFormat;
	description.SampleDesc = SampleDesc;
	description.NodeMask = NodeMask;
	description.CachedPSO = CachedPSO;
	description.Flags = Flags;

	return description;
}

GraphicsPipelineStateObjectDescription GraphicsPipelineStateObjectDescription::FromD3DDescription(
	RootSignature* pRootSignature,
	const GraphicsWrappedPipelineData& wrappedData,
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC& description)
{
	GraphicsPipelineStateObjectDescription result;

	result.PRootSignature = pRootSignature;
	result.PVertexShader = wrappedData.PVertexShader;
	result.PHullShader = wrappedData.PHullShader;
	result.PDomainShader = wrappedData.PDomainShader;
	result.PGeometryShader = wrappedData.PGeometryShader;
	result.PPixelShader = wrappedData.PPixelShader;
	result.PVertexInputLayout = wrappedData.PVertexInputLayout;

	result.RasterizerState.Description = description.RasterizerState;
	result.DepthStencilState.Description = description.DepthStencilState;
	result.BlendState.Description = description.BlendState;
	result.StreamOutput.Initialize(description.StreamOutput);

	// Direct members.
	result.SampleMask = description.SampleMask;
	result.IBStripCutValue = description.IBStripCutValue;
	result.PrimitiveTopologyType = description.PrimitiveTopologyType;
	result.NumRenderTargets = description.NumRenderTargets;
	for (unsigned i = 0; i < description.NumRenderTargets; i++)
	{
		result.RTVFormats[i] = description.RTVFormats[i];
	}
	result.DSVFormat = description.DSVFormat;
	result.SampleDesc = description.SampleDesc;
	result.NodeMask = description.NodeMask;
	result.CachedPSO = description.CachedPSO;
	result.Flags = description.Flags;

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

const GraphicsPipelineStateObjectDescription& GraphicsPipelineStateObject::GetDescription() const
{
	return m_Description;
}

RootSignature* GraphicsPipelineStateObject::GetRootSignature() const
{
	return m_Description.PRootSignature;
}

ID3D12PipelineState* GraphicsPipelineStateObject::GetPipelineStateObject()
{
	return m_Resource.Get();
}

void GraphicsPipelineStateObject::Initialize(ID3D12Device* device,
	const GraphicsPipelineStateObjectDescription& description)
{
	auto d3dDescription = description.ToD3DDescription();

	auto hr = device->CreateGraphicsPipelineState(&d3dDescription, IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a graphics pipeline state object.");
	}

	m_Description = description;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ComputePipelineStateObjectDescription::ComputePipelineStateObjectDescription()
	: PComputeShader(nullptr)
{
}

bool ComputePipelineStateObjectDescription::operator==(const ComputePipelineStateObjectDescription& other) const
{
	BaseEqualCompareBlock(PipelineStateObjectDescription, other);
	NumericalEqualCompareBlock(PComputeShader);
	return true;
}

bool ComputePipelineStateObjectDescription::operator!=(const ComputePipelineStateObjectDescription& other) const
{
	return !(*this == other);
}

bool ComputePipelineStateObjectDescription::operator<(const ComputePipelineStateObjectDescription& other) const
{
	BaseLessCompareBlock(PipelineStateObjectDescription, other);
	NumericalLessCompareBlock(PComputeShader);
	return false;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC ComputePipelineStateObjectDescription::ToD3DDescription() const
{
	assert(PRootSignature != nullptr && PComputeShader != nullptr);

	D3D12_COMPUTE_PIPELINE_STATE_DESC description;
	description.pRootSignature = PRootSignature->GetRootSignature();
	description.CS = PComputeShader->GetByteCode();
	description.NodeMask = NodeMask;
	description.CachedPSO = CachedPSO;
	description.Flags = Flags;
	return description;
}

ComputePipelineStateObjectDescription ComputePipelineStateObjectDescription::FromD3DDescription(
	RootSignature* pRootSignature, const Shader* pComputeShader,
	const D3D12_COMPUTE_PIPELINE_STATE_DESC& description)
{
	ComputePipelineStateObjectDescription result;

	result.PRootSignature = pRootSignature;
	result.PComputeShader = pComputeShader;

	// Direct members.
	result.NodeMask = description.NodeMask;
	result.CachedPSO = description.CachedPSO;
	result.Flags = description.Flags;

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

const ComputePipelineStateObjectDescription& ComputePipelineStateObject::GetDescription() const
{
	return m_Description;
}

RootSignature* ComputePipelineStateObject::GetRootSignature() const
{
	return m_Description.PRootSignature;
}

ID3D12PipelineState* ComputePipelineStateObject::GetPipelineStateObject()
{
	return m_Resource.Get();
}

void ComputePipelineStateObject::Initialize(ID3D12Device* device,
	const ComputePipelineStateObjectDescription& description)
{
	auto d3dDescription = description.ToD3DDescription();

	auto hr = device->CreateComputePipelineState(&d3dDescription, IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a compute pipeline state object.");
	}

	m_Description = description;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsPipelineStateObject* PipelineStateObjectManager::GetGraphicsPipelineStateObject(ID3D12Device* device,
	const GraphicsPipelineStateObjectDescription& description)
{
	auto it = m_GraphicsPSOMap.find(description);
	if (it == m_GraphicsPSOMap.end())
	{
		auto pPipelineStateObject = m_GraphicsPSOs.Request();
		pPipelineStateObject->Initialize(device, description);
		m_GraphicsPSOMap[description] = pPipelineStateObject;
		return pPipelineStateObject;
	}
	return it->second;
}

ComputePipelineStateObject* PipelineStateObjectManager::GetComputePipelineStateObject(ID3D12Device* device,
	const ComputePipelineStateObjectDescription& description)
{
	auto it = m_ComputePSOMap.find(description);
	if (it == m_ComputePSOMap.end())
	{
		auto pPipelineStateObject = m_ComputePSOs.Request();
		pPipelineStateObject->Initialize(device, description);
		m_ComputePSOMap[description] = pPipelineStateObject;
		return pPipelineStateObject;
	}
	return it->second;
}

GraphicsPipelineStateObject* PipelineStateObjectManager::GetGraphicsPipelineStateObject(ID3D12Device* device,
	RootSignature* pRootSignature,
	const GraphicsWrappedPipelineData& wrappedData,
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC& description)
{
	return GetGraphicsPipelineStateObject(device,
		GraphicsPipelineStateObjectDescription::FromD3DDescription(pRootSignature, wrappedData, description));
}

ComputePipelineStateObject* PipelineStateObjectManager::GetComputePipelineStateObject(ID3D12Device* device,
	RootSignature* pRootSignature, const Shader* pComputeShader,
	const D3D12_COMPUTE_PIPELINE_STATE_DESC& description)
{
	return GetComputePipelineStateObject(device,
		ComputePipelineStateObjectDescription::FromD3DDescription(pRootSignature, pComputeShader, description));
}