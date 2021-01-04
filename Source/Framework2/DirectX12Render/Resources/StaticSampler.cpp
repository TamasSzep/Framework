// DirectX12Render/Resources/StaticSampler.cpp

#include <DirectX12Render/Resources/StaticSampler.h>

#include <Core/Comparison.h>

using namespace DirectX12Render;

void StaticSamplerHelper::SetDefaultMembers(D3D12_STATIC_SAMPLER_DESC& description)
{
	description.MipLODBias = 0;
	description.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
	description.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	description.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	description.MinLOD = 0.0f;
	description.MaxLOD = D3D12_FLOAT32_MAX;
}

void StaticSamplerHelper::SetBindingData(D3D12_STATIC_SAMPLER_DESC& description,
	unsigned shaderRegister, unsigned registerSpace,
	D3D12_SHADER_VISIBILITY shaderVisibility)
{
	description.ShaderRegister = shaderRegister;
	description.RegisterSpace = registerSpace;
	description.ShaderVisibility = shaderVisibility;
}

D3D12_STATIC_SAMPLER_DESC StaticSamplerHelper::GetDefaultPointSamplerDescription(
	unsigned shaderRegister, unsigned registerSpace, D3D12_SHADER_VISIBILITY shaderVisibility)
{
	D3D12_STATIC_SAMPLER_DESC description;
	SetDefaultMembers(description);
	SetBindingData(description, shaderRegister, registerSpace, shaderVisibility);
	
	description.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	description.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	description.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	description.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	
	return description;
}

D3D12_STATIC_SAMPLER_DESC StaticSamplerHelper::GetDefaultBilinearSamplerDescription(
	unsigned shaderRegister, unsigned registerSpace,
	D3D12_SHADER_VISIBILITY shaderVisibility)
{
	D3D12_STATIC_SAMPLER_DESC description;
	SetDefaultMembers(description);
	SetBindingData(description, shaderRegister, registerSpace, shaderVisibility);

	description.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	description.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	description.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	description.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	return description;
}

D3D12_STATIC_SAMPLER_DESC StaticSamplerHelper::GetDefaultTrilinearSamplerDescription(
	unsigned shaderRegister, unsigned registerSpace,
	D3D12_SHADER_VISIBILITY shaderVisibility)
{
	D3D12_STATIC_SAMPLER_DESC description;
	SetDefaultMembers(description);
	SetBindingData(description, shaderRegister, registerSpace, shaderVisibility);

	description.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	description.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	description.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	description.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	return description;
}

D3D12_STATIC_SAMPLER_DESC StaticSamplerHelper::GetDefaultAnisotropicSamplerDescription(
	unsigned shaderRegister, unsigned registerSpace,
	D3D12_SHADER_VISIBILITY shaderVisibility)
{
	D3D12_STATIC_SAMPLER_DESC description;
	SetDefaultMembers(description);
	SetBindingData(description, shaderRegister, registerSpace, shaderVisibility);

	description.Filter = D3D12_FILTER_ANISOTROPIC;
	description.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	description.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	description.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	return description;
}