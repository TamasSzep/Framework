// DirectX12Render/Utilities/MSAAResolver.cpp

#include <DirectX12Render/Utilities/MSAAResolver.h>

#include <WindowsApplication/DirectX/Format.h>
#include <DirectX12Render/Resources/StaticSampler.h>
#include <DirectX12Render/Resources/RootSignature.h>
#include <DirectX12Render/Resources/Shader.h>
#include <DirectX12Render/Resources/PipelineStateObject.h>
#include <DirectX12Render/Resources/Texture2DBase.h>

using namespace DirectXRender;
using namespace DirectX12Render;

enum class Ranges
{
	Textures_SRV,
	Textures_UAV,

	COUNT
};

enum class RootParameters
{
	CBV = 0,
	Texture_SRV_DescriptorTable,
	Texture_UAV_DescriptorTable,

	COUNT
};

enum class Registers
{
	CBV = 0,
	Texture_Source_SRV = 0,
	Texture_Target_UAV = 0
};

struct CB_Type
{
	unsigned CountSamples;
	float InvCountSamples;
};

const unsigned c_MaxTextures = 256;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

MSAAResolver::MSAAResolver()
	: m_RootSignature(nullptr)
	, m_PipelineStateObject(nullptr)
	, m_MultisampleCount(0)
{
}

void MSAAResolver::Initialize(const UtilityInitializationContext& context)
{
	this->m_MultisampleCount = context.MultisampleCount;

	for (unsigned i = 0; i < context.FrameCount; i++)
	{
		m_ConstantBuffers.emplace_back(ConstantBufferMappingType::AlwaysMapped,
			static_cast<unsigned>(sizeof(CB_Type)), 1);
		m_ConstantBuffers.back().Initialize(context.Device);
	}
	
	m_SRV_UAV_DescriptorHeap.Initialize(context.Device, DescriptorHeapType::CBV_SRV_UAV, c_MaxTextures);

	CreateRootSignature(context);
	CreatePipelineStateObject(context);
}

void MSAAResolver::CreateRootSignature(const UtilityInitializationContext& context)
{
	CD3DX12_DESCRIPTOR_RANGE ranges[(int)Ranges::COUNT];
	CD3DX12_ROOT_PARAMETER rootParameters[(int)RootParameters::COUNT];

	ranges[(int)Ranges::Textures_SRV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, (int)Registers::Texture_Source_SRV);
	ranges[(int)Ranges::Textures_UAV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, (int)Registers::Texture_Target_UAV);

	rootParameters[(int)RootParameters::CBV].InitAsConstantBufferView((int)Registers::CBV);
	rootParameters[(int)RootParameters::Texture_SRV_DescriptorTable].InitAsDescriptorTable(1, &ranges[(int)Ranges::Textures_SRV], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[(int)RootParameters::Texture_UAV_DescriptorTable].InitAsDescriptorTable(1, &ranges[(int)Ranges::Textures_UAV], D3D12_SHADER_VISIBILITY_ALL);

	DirectX12Render::RootSignatureDescription rootSignatureDesc;
	rootSignatureDesc.Initialize(_countof(rootParameters), rootParameters, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_NONE);

	m_RootSignature = context.DX12M->RootSignatureManager.GetRootSignature(context.Device, rootSignatureDesc);
}

void MSAAResolver::CreatePipelineStateObject(const UtilityInitializationContext& context)
{
	auto pComputeShader = context.DX12M->ShaderManager.GetShaderSimple(context.Device,
	{ "ResolveDepth.hlsl", "Main", ShaderType::Compute });

	DirectX12Render::ComputePipelineStateObjectDescription description;
	description.PRootSignature = m_RootSignature;
	description.PComputeShader = pComputeShader;

	m_PipelineStateObject = context.DX12M->PipelineStateObjectManager.GetComputePipelineStateObject(context.Device, description);
}

void MSAAResolver::RegisterTextures(ID3D12Device* device,
	Texture2DBase* sourceTexture, Texture2DBase* targetTexture)
{
	if (!IsResolvableWithAPICall(sourceTexture, targetTexture))
	{
		sourceTexture->CreateShaderResourceView(device, &m_SRV_UAV_DescriptorHeap);
		targetTexture->CreateUnorderedAccessView(device, &m_SRV_UAV_DescriptorHeap);
	}
}

void MSAAResolver::Update(const UtilityUpdateContext& context)
{
	auto& cb = m_ConstantBuffers[context.FrameIndex];

	cb.StartWrite();

	auto& cbData = cb.Access<CB_Type>(0);
	cbData.CountSamples = m_MultisampleCount;
	cbData.InvCountSamples = 1.0f / m_MultisampleCount;

	cb.IndicateWrittenRangeWithViewIndex(0, 1);
	cb.EndWrite();
}

void MSAAResolver::Resolve(ID3D12GraphicsCommandList* commandList,
	Texture2DBase* sourceTexture, Texture2DBase* targetTexture, unsigned frameIndex, ResolveType resolveType)
{
	switch (resolveType)
	{
	case ResolveType::APICall : ResolveWithAPICall(commandList, sourceTexture, targetTexture); break;
	case ResolveType::ResolveDepth: ResolveDepth(commandList, sourceTexture, targetTexture, frameIndex); break;
	}
}

bool MSAAResolver::IsResolvableWithAPICall(Texture2DBase* sourceTexture, Texture2DBase* targetTexture)
{
	return Texture2DDescription::IsResolvableWithAPICall(sourceTexture->GetDescription(),
		targetTexture->GetDescription());
}

void MSAAResolver::ResolveWithAPICall(ID3D12GraphicsCommandList* commandList,
	Texture2DBase* sourceTexture, Texture2DBase* targetTexture)
{
	assert(IsResolvableWithAPICall(sourceTexture, targetTexture));

	auto& sourceDesc = sourceTexture->GetDescription();
	auto& targetDesc = targetTexture->GetDescription();
	auto resolveFormat = ::GetResolveFormat(sourceDesc.Format, targetDesc.Format);
	bool isMultisampled = (sourceDesc.SampleCount > 1);

	auto oldState1 = sourceTexture->Transition(commandList, (isMultisampled
		? D3D12_RESOURCE_STATE_RESOLVE_SOURCE
		: D3D12_RESOURCE_STATE_COPY_SOURCE));
	auto oldState2 = targetTexture->Transition(commandList, (isMultisampled
		? D3D12_RESOURCE_STATE_RESOLVE_DEST
		: D3D12_RESOURCE_STATE_COPY_DEST));
	
	auto sourceRes = sourceTexture->GetResource();
	auto targetRes = targetTexture->GetResource();

	if (isMultisampled)
		commandList->ResolveSubresource(targetRes, 0, sourceRes, 0, resolveFormat);
	else
		commandList->CopyResource(targetRes, sourceRes);

	sourceTexture->Transition(commandList, oldState1);
	targetTexture->Transition(commandList, oldState2);
}

void MSAAResolver::ResolveDepth(ID3D12GraphicsCommandList* commandList,
	Texture2DBase* sourceTexture, Texture2DBase* targetTexture, unsigned frameIndex)
{
	auto& textureDescription = targetTexture->GetDescription();

	auto oldState1 = sourceTexture->TransitionToNonPixelShaderResource(commandList);
	auto oldState2 = targetTexture->TransitionToUAV(commandList);

	ID3D12DescriptorHeap* ppHeaps[] = { m_SRV_UAV_DescriptorHeap.GetHeap() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->SetPipelineState(m_PipelineStateObject->GetPipelineStateObject());
	commandList->SetComputeRootSignature(m_RootSignature->GetRootSignature());

	commandList->SetComputeRootConstantBufferView((int)RootParameters::CBV, m_ConstantBuffers[frameIndex].GetConstantBufferGPUVirtualAddress(0));
	commandList->SetComputeRootDescriptorTable((int)RootParameters::Texture_SRV_DescriptorTable, sourceTexture->GetShaderResourceView().GPUDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)RootParameters::Texture_UAV_DescriptorTable, targetTexture->GetUnorderedAccessView().GPUDescriptorHandle);

	DispatchByThreadCount(commandList, textureDescription.Width, textureDescription.Height, 1, 8, 4, 1);

	sourceTexture->Transition(commandList, oldState1);
	targetTexture->Transition(commandList, oldState2);
}