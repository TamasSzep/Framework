// DirectX11Render/Resources/PipelineState.h

#include <DirectX11Render/Resources/PipelineState.h>

using namespace DirectXRender;
using namespace DirectX11Render;

RasterizerStateDescription::RasterizerStateDescription()
{
	SetToDefault();
}

void RasterizerStateDescription::SetToDefault()
{
	SetToD3DDefault();
	Description.FrontCounterClockwise = TRUE;
}

void RasterizerStateDescription::SetToD3DDefault()
{
	Description.FillMode = D3D11_FILL_SOLID;
	Description.CullMode = D3D11_CULL_BACK;
	Description.FrontCounterClockwise = FALSE;
	Description.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
	Description.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	Description.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	Description.DepthClipEnable = TRUE;
	Description.ScissorEnable = FALSE;
	Description.MultisampleEnable = FALSE;
	Description.AntialiasedLineEnable = FALSE;
}

void RasterizerStateDescription::DisableCulling()
{
	Description.CullMode = D3D11_CULL_NONE;
}

void RasterizerStateDescription::SetWireframeFillMode()
{
	Description.FillMode = D3D11_FILL_WIREFRAME;
}

void RasterizerStateDescription::EnableMultisampling()
{
	Description.MultisampleEnable = TRUE;
	Description.AntialiasedLineEnable = TRUE;
}

bool RasterizerStateDescription::operator==(const RasterizerStateDescription& other) const
{
	NumericalEqualCompareBlock(Description.FillMode);
	NumericalEqualCompareBlock(Description.CullMode);
	NumericalEqualCompareBlock(Description.FrontCounterClockwise);
	NumericalEqualCompareBlock(Description.DepthBias);
	NumericalEqualCompareBlock(Description.DepthBiasClamp);
	NumericalEqualCompareBlock(Description.SlopeScaledDepthBias);
	NumericalEqualCompareBlock(Description.DepthClipEnable);
	NumericalEqualCompareBlock(Description.ScissorEnable);
	NumericalEqualCompareBlock(Description.MultisampleEnable);
	NumericalEqualCompareBlock(Description.AntialiasedLineEnable);
	return true;
}

bool RasterizerStateDescription::operator!=(const RasterizerStateDescription& other) const
{
	return !(*this == other);
}

bool RasterizerStateDescription::operator<(const RasterizerStateDescription& other) const
{
	NumericalLessCompareBlock(Description.FillMode);
	NumericalLessCompareBlock(Description.CullMode);
	NumericalLessCompareBlock(Description.FrontCounterClockwise);
	NumericalLessCompareBlock(Description.DepthBias);
	NumericalLessCompareBlock(Description.DepthBiasClamp);
	NumericalLessCompareBlock(Description.SlopeScaledDepthBias);
	NumericalLessCompareBlock(Description.DepthClipEnable);
	NumericalLessCompareBlock(Description.ScissorEnable);
	NumericalLessCompareBlock(Description.MultisampleEnable);
	NumericalLessCompareBlock(Description.AntialiasedLineEnable);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

DepthStencilStateDescription::DepthStencilStateDescription()
{
	SetToD3DDefault();
}

void DepthStencilStateDescription::SetToD3DDefault()
{
	Description.DepthEnable = TRUE;
	Description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	Description.DepthFunc = D3D11_COMPARISON_LESS;
	Description.StencilEnable = FALSE;
	Description.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	Description.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	const D3D11_DEPTH_STENCILOP_DESC defaultStencilOp =
	{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
	Description.FrontFace = defaultStencilOp;
	Description.BackFace = defaultStencilOp;
}

void DepthStencilStateDescription::DisableDepthWriting()
{
	Description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
}

bool DepthStencilStateDescription::operator==(const DepthStencilStateDescription& other) const
{
	NumericalEqualCompareBlock(Description.DepthEnable);
	NumericalEqualCompareBlock(Description.DepthWriteMask);
	NumericalEqualCompareBlock(Description.DepthFunc);
	NumericalEqualCompareBlock(Description.StencilEnable);
	NumericalEqualCompareBlock(Description.StencilReadMask);
	NumericalEqualCompareBlock(Description.StencilWriteMask);
	NumericalEqualCompareBlock(Description.FrontFace.StencilFailOp);
	NumericalEqualCompareBlock(Description.FrontFace.StencilDepthFailOp);
	NumericalEqualCompareBlock(Description.FrontFace.StencilPassOp);
	NumericalEqualCompareBlock(Description.FrontFace.StencilFunc);
	NumericalEqualCompareBlock(Description.BackFace.StencilFailOp);
	NumericalEqualCompareBlock(Description.BackFace.StencilDepthFailOp);
	NumericalEqualCompareBlock(Description.BackFace.StencilPassOp);
	NumericalEqualCompareBlock(Description.BackFace.StencilFunc);
	return true;
}

bool DepthStencilStateDescription::operator!=(const DepthStencilStateDescription& other) const
{
	return !(*this == other);
}

bool DepthStencilStateDescription::operator<(const DepthStencilStateDescription& other) const
{
	NumericalLessCompareBlock(Description.DepthEnable);
	NumericalLessCompareBlock(Description.DepthWriteMask);
	NumericalLessCompareBlock(Description.DepthFunc);
	NumericalLessCompareBlock(Description.StencilEnable);
	NumericalLessCompareBlock(Description.StencilReadMask);
	NumericalLessCompareBlock(Description.StencilWriteMask);
	NumericalLessCompareBlock(Description.FrontFace.StencilFailOp);
	NumericalLessCompareBlock(Description.FrontFace.StencilDepthFailOp);
	NumericalLessCompareBlock(Description.FrontFace.StencilPassOp);
	NumericalLessCompareBlock(Description.FrontFace.StencilFunc);
	NumericalLessCompareBlock(Description.BackFace.StencilFailOp);
	NumericalLessCompareBlock(Description.BackFace.StencilDepthFailOp);
	NumericalLessCompareBlock(Description.BackFace.StencilPassOp);
	NumericalLessCompareBlock(Description.BackFace.StencilFunc);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

BlendStateDescription::BlendStateDescription()
{
	SetToD3DDefault();
}

void BlendStateDescription::SetToD3DDefault()
{
	Description.AlphaToCoverageEnable = FALSE;
	Description.IndependentBlendEnable = FALSE;
	const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		FALSE,
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL
	};
	for (unsigned i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		Description.RenderTarget[i] = defaultRenderTargetBlendDesc;
}

void BlendStateDescription::SetToBlending()
{
	Description.AlphaToCoverageEnable = FALSE;
	Description.IndependentBlendEnable = FALSE;
	Description.RenderTarget[0].BlendEnable = TRUE;
	Description.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
}

void BlendStateDescription::EnableAlphaToCoverage()
{
	Description.AlphaToCoverageEnable = true;
}

void BlendStateDescription::SetToNonpremultipliedAlphaBlending()
{
	SetToBlending();
	Description.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	Description.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	Description.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	Description.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	Description.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	Description.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
}

bool BlendStateDescription::operator==(const BlendStateDescription& other) const
{
	NumericalEqualCompareBlock(Description.AlphaToCoverageEnable);
	NumericalEqualCompareBlock(Description.IndependentBlendEnable);
	for (unsigned i = 0; i < 8; i++)
	{
		NumericalEqualCompareBlock(Description.RenderTarget[i].BlendEnable);
		NumericalEqualCompareBlock(Description.RenderTarget[i].SrcBlend);
		NumericalEqualCompareBlock(Description.RenderTarget[i].DestBlend);
		NumericalEqualCompareBlock(Description.RenderTarget[i].BlendOp);
		NumericalEqualCompareBlock(Description.RenderTarget[i].SrcBlendAlpha);
		NumericalEqualCompareBlock(Description.RenderTarget[i].DestBlendAlpha);
		NumericalEqualCompareBlock(Description.RenderTarget[i].BlendOpAlpha);
		NumericalEqualCompareBlock(Description.RenderTarget[i].RenderTargetWriteMask);
	}
	return true;
}

bool BlendStateDescription::operator!=(const BlendStateDescription& other) const
{
	return !(*this == other);
}

bool BlendStateDescription::operator<(const BlendStateDescription& other) const
{
	NumericalLessCompareBlock(Description.AlphaToCoverageEnable);
	NumericalLessCompareBlock(Description.IndependentBlendEnable);
	for (unsigned i = 0; i < 8; i++)
	{
		NumericalLessCompareBlock(Description.RenderTarget[i].BlendEnable);
		NumericalLessCompareBlock(Description.RenderTarget[i].SrcBlend);
		NumericalLessCompareBlock(Description.RenderTarget[i].DestBlend);
		NumericalLessCompareBlock(Description.RenderTarget[i].BlendOp);
		NumericalLessCompareBlock(Description.RenderTarget[i].SrcBlendAlpha);
		NumericalLessCompareBlock(Description.RenderTarget[i].DestBlendAlpha);
		NumericalLessCompareBlock(Description.RenderTarget[i].BlendOpAlpha);
		NumericalLessCompareBlock(Description.RenderTarget[i].RenderTargetWriteMask);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

SamplerStateDescription::SamplerStateDescription()
{
	SetToD3DDefault();
}

void SamplerStateDescription::SetToD3DDefault()
{
	Description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	Description.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	Description.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	Description.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	Description.MipLODBias = D3D11_DEFAULT_MIP_LOD_BIAS;
	Description.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	Description.ComparisonFunc = D3D11_COMPARISON_NEVER;
	for (unsigned i = 0; i < 4; i++) Description.BorderColor[i] = 0.0f;
	Description.MinLOD = -D3D11_FLOAT32_MAX;
	Description.MaxLOD = D3D11_FLOAT32_MAX;
}

void SamplerStateDescription::SetToNearest()
{
	Description.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
}

void SamplerStateDescription::SetToBilinear()
{
	Description.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
}

void SamplerStateDescription::SetToTrilinear()
{
	Description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
}

void SamplerStateDescription::SetToAnisotropic(unsigned countSamples)
{
	Description.Filter = D3D11_FILTER_ANISOTROPIC;
	Description.MaxAnisotropy = countSamples;
}

void SamplerStateDescription::SetToWrap()
{
	Description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	Description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	Description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
}

bool SamplerStateDescription::operator==(const SamplerStateDescription& other) const
{
	NumericalEqualCompareBlock(Description.Filter);
	NumericalEqualCompareBlock(Description.AddressU);
	NumericalEqualCompareBlock(Description.AddressV);
	NumericalEqualCompareBlock(Description.AddressW);
	NumericalEqualCompareBlock(Description.MipLODBias);
	NumericalEqualCompareBlock(Description.MaxAnisotropy);
	NumericalEqualCompareBlock(Description.ComparisonFunc);
	for (unsigned i = 0; i < 4; i++) NumericalEqualCompareBlock(Description.BorderColor[i]);
	NumericalEqualCompareBlock(Description.MinLOD);
	NumericalEqualCompareBlock(Description.MaxLOD);
	return true;
}

bool SamplerStateDescription::operator!=(const SamplerStateDescription& other) const
{
	return !(*this == other);
}

bool SamplerStateDescription::operator<(const SamplerStateDescription& other) const
{
	NumericalLessCompareBlock(Description.Filter);
	NumericalLessCompareBlock(Description.AddressU);
	NumericalLessCompareBlock(Description.AddressV);
	NumericalLessCompareBlock(Description.AddressW);
	NumericalLessCompareBlock(Description.MipLODBias);
	NumericalLessCompareBlock(Description.MaxAnisotropy);
	NumericalLessCompareBlock(Description.ComparisonFunc);
	for (unsigned i = 0; i < 4; i++) NumericalLessCompareBlock(Description.BorderColor[i]);
	NumericalLessCompareBlock(Description.MinLOD);
	NumericalLessCompareBlock(Description.MaxLOD);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool PipelineStateDescription::operator==(const PipelineStateDescription& other) const
{
	StructureEqualCompareBlock(Shaders);
	StructureEqualCompareBlock(InputLayout);
	StructureEqualCompareBlock(RasterizerState);
	StructureEqualCompareBlock(DepthStencilState);
	StructureEqualCompareBlock(BlendState);
	StructureEqualCompareBlock(SamplerStates);
	return true;
}

bool PipelineStateDescription::operator!=(const PipelineStateDescription& other) const
{
	return !(*this == other);
}

bool PipelineStateDescription::operator<(const PipelineStateDescription& other) const
{
	StructureLessCompareBlock(Shaders);
	StructureLessCompareBlock(InputLayout);
	StructureLessCompareBlock(RasterizerState);
	StructureLessCompareBlock(DepthStencilState);
	StructureLessCompareBlock(BlendState);
	StructureLessCompareBlock(SamplerStates);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineStateSettingContext::PipelineStateSettingContext()
	: StencilReference(D3D11_DEFAULT_STENCIL_REFERENCE)
	, BlendSampleMask(D3D11_DEFAULT_SAMPLE_MASK)
{
	BlendFactor[0] = D3D11_DEFAULT_BLEND_FACTOR_RED;
	BlendFactor[1] = D3D11_DEFAULT_BLEND_FACTOR_GREEN;
	BlendFactor[2] = D3D11_DEFAULT_BLEND_FACTOR_BLUE;
	BlendFactor[3] = D3D11_DEFAULT_BLEND_FACTOR_ALPHA;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

PipelineState::PipelineState()
	: VS(nullptr), HS(nullptr), DS(nullptr), GS(nullptr), PS(nullptr), CS(nullptr)
	, InputLayout(nullptr), RasterizerState(nullptr), DepthStencilState(nullptr), BlendState(nullptr)
{
}

template <typename D3DShaderType, typename D3DShaderSettingFunctionType, typename D3DSamplerSettingFunctionType>
void SetShaderForContext(ID3D11DeviceContext* context, D3DShaderType* shader,
	const PipelineState& pipelineState, unsigned shaderIndex,
	D3DShaderSettingFunctionType&& shaderSettingfunction, D3DSamplerSettingFunctionType&& samplerSettingFunction)
{
	(context->*shaderSettingfunction)(shader, nullptr, 0);
	if (shader != nullptr)
	{
		auto startIndex = pipelineState.SamplerStateStart[shaderIndex];
		auto countSamplers = pipelineState.SamplerStateStart[shaderIndex + 1] - startIndex;
		if (countSamplers > 0)
		{
			(context->*samplerSettingFunction)(0, countSamplers, &pipelineState.SamplerStates[startIndex]);
		}
	}
}

void PipelineState::SetForContext(ID3D11DeviceContext* context, const PipelineStateSettingContext& pssContext) const
{
	using dct = ID3D11DeviceContext;
	SetShaderForContext(context, VS, *this, 0, &dct::VSSetShader, &dct::VSSetSamplers);
	SetShaderForContext(context, HS, *this, 1, &dct::HSSetShader, &dct::HSSetSamplers);
	SetShaderForContext(context, DS, *this, 2, &dct::DSSetShader, &dct::DSSetSamplers);
	SetShaderForContext(context, GS, *this, 3, &dct::GSSetShader, &dct::GSSetSamplers);
	SetShaderForContext(context, PS, *this, 4, &dct::PSSetShader, &dct::PSSetSamplers);
	SetShaderForContext(context, CS, *this, 5, &dct::CSSetShader, &dct::CSSetSamplers);

	context->IASetInputLayout(InputLayout);
	context->RSSetState(RasterizerState);
	context->OMSetDepthStencilState(DepthStencilState, pssContext.StencilReference);
	context->OMSetBlendState(BlendState, pssContext.BlendFactor, pssContext.BlendSampleMask);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

const PipelineState& PipelineStateManager::GetPipelineState(unsigned index) const
{
	return m_PipelineStates[index];
}

unsigned PipelineStateManager::GetPipelineStateIndex(ID3D11Device* device, const PipelineStateDescription& description)
{
	auto it = m_PipelineStateMap.find(description);
	if (it == m_PipelineStateMap.end())
	{
		unsigned index = m_PipelineStates.Add();
		CreatePipelineState(device, description, m_PipelineStates[index]);
		it = m_PipelineStateMap.insert({ description, index }).first;
	}
	return it->second;
}

const PipelineState& PipelineStateManager::GetPipelineState(ID3D11Device* device, const PipelineStateDescription& description)
{
	return GetPipelineState(GetPipelineStateIndex(device, description));
}

inline void AddShader(const Shader* shader, PipelineState& pipelineState)
{
	auto type = shader->GetDescription().Type;
	auto shaderPtr = shader->GetShaderPtr();

	switch (type)
	{
	case ShaderType::Vertex: pipelineState.VS = (ID3D11VertexShader*)shaderPtr; break;
	case ShaderType::Hull: pipelineState.HS = (ID3D11HullShader*)shaderPtr; break;
	case ShaderType::Domain: pipelineState.DS = (ID3D11DomainShader*)shaderPtr; break;
	case ShaderType::Geometry: pipelineState.GS = (ID3D11GeometryShader*)shaderPtr; break;
	case ShaderType::Pixel: pipelineState.PS = (ID3D11PixelShader*)shaderPtr; break;
	case ShaderType::Compute: pipelineState.CS = (ID3D11ComputeShader*)shaderPtr; break;
	}
}

inline const Shader* GetShader(const Core::SimpleTypeVectorU<const Shader*>& shaders, ShaderType shaderType)
{
	unsigned countShaders = shaders.GetSize();
	for (unsigned i = 0; i < countShaders; i++)
	{
		if (shaders[i]->GetDescription().Type == shaderType) return shaders[i];
	}
	return nullptr;
}

template <typename NResourceType, typename DescriptionType, typename InitializationFunctionType, typename... Args>
inline NResourceType* GetResource(ID3D11Device* device, const DescriptionType& description,
	Core::ResourcePoolU<ComPtr<NResourceType>>& resourcePool,
	std::map<DescriptionType, ComPtr<NResourceType>*>& resourceMap,
	InitializationFunctionType&& initFunction, Args&&... initArgs)
{
	auto it = resourceMap.find(description);
	if (it == resourceMap.end())
	{
		auto pResource = resourcePool.Request();
		initFunction(*pResource, device, description, std::forward<Args>(initArgs)...);
		it = resourceMap.insert({ description, pResource }).first;
	}
	return it->second->Get();
}

inline void InitializeInputLayout(PipelineStateManager::InputLayoutResource& resource, ID3D11Device* device,
	const DirectX11Render::VertexInputLayout& inputLayout, const Shader* pVertexShader)
{
	auto shaderBlob = pVertexShader->GetBlob();
	auto hr = device->CreateInputLayout(inputLayout.Elements.GetArray(), inputLayout.Elements.GetSize(),
		shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &resource);
	if (FAILED(hr)) EngineBuildingBlocks::RaiseException("Failed to create vertex input layout.");
}

inline void InitializeRasterizerState(PipelineStateManager::RasterizerStateResource& resource, ID3D11Device* device,
	const RasterizerStateDescription& description)
{
	auto hr = device->CreateRasterizerState(&description.Description, &resource);
	if (FAILED(hr)) EngineBuildingBlocks::RaiseException("Failed to create rasterizer state.");
}

inline void InitializeDepthStencilState(PipelineStateManager::DepthStencilStateResource& resource, ID3D11Device* device,
	const DepthStencilStateDescription& description)
{
	auto hr = device->CreateDepthStencilState(&description.Description, &resource);
	if (FAILED(hr)) EngineBuildingBlocks::RaiseException("Failed to create depth stencil state.");
}

inline void InitializeBlendState(PipelineStateManager::BlendStateResource& resource, ID3D11Device* device,
	const BlendStateDescription& description)
{
	auto hr = device->CreateBlendState(&description.Description, &resource);
	if (FAILED(hr)) EngineBuildingBlocks::RaiseException("Failed to create blend state.");
}

inline void InitializeSamplerState(PipelineStateManager::SamplerStateResource& resource, ID3D11Device* device,
	const SamplerStateDescription& description)
{
	auto hr = device->CreateSamplerState(&description.Description, &resource);
	if (FAILED(hr)) EngineBuildingBlocks::RaiseException("Failed to create sampler state.");
}

void PipelineStateManager::CreatePipelineState(ID3D11Device* device, const PipelineStateDescription& description,
	PipelineState& pipelineState)
{
	for (unsigned i = 0; i < description.Shaders.GetSize(); i++)
		AddShader(description.Shaders[i], pipelineState);

	if (pipelineState.VS == nullptr)
	{
		pipelineState.InputLayout = nullptr;
	}
	else
	{
		pipelineState.InputLayout = GetResource(device, description.InputLayout, m_InputLayouts, m_InputLayoutMap,
			&InitializeInputLayout, GetShader(description.Shaders, ShaderType::Vertex));
	}

	pipelineState.RasterizerState = GetResource(device, description.RasterizerState, m_RasterizerStates,
		m_RasterizerStateMap, &InitializeRasterizerState);
	pipelineState.DepthStencilState = GetResource(device, description.DepthStencilState, m_DepthStencilStates,
		m_DepthStencilStateMap, &InitializeDepthStencilState);
	pipelineState.BlendState = GetResource(device, description.BlendState, m_BlendStates,
		m_BlendStateMap, &InitializeBlendState);

	unsigned countSamplers = static_cast<unsigned>(description.SamplerStates.size());
	Core::SimpleTypeVectorU<ID3D11SamplerState*> samplers;
	samplers.Resize(countSamplers);
	for (unsigned i = 0; i < countSamplers; i++)
	{
		samplers[i] = GetResource(device, description.SamplerStates[i].first, m_SamplerStates, m_SamplerStateMap,
			&InitializeSamplerState);
	}

	unsigned samplerStartIndex = 0;
	for (unsigned i = 0; i < 6; i++)
	{
		auto shaderType = (ShaderType)i;
		pipelineState.SamplerStateStart[i] = samplerStartIndex;
		if (GetShader(description.Shaders, shaderType) != nullptr)
		{
			auto shaderTypeFlag = AsFlag(shaderType);
			for (unsigned j = 0; j < countSamplers; j++)
			{
				pipelineState.SamplerStates.PushBack(
					HasFlag(description.SamplerStates[j].second, shaderTypeFlag) ? samplers[j] : nullptr);
			}
			samplerStartIndex += countSamplers;
		}
	}
	pipelineState.SamplerStateStart[6] = samplerStartIndex;
}