// DirectX12Render/Resources/StaticSampler.h

#ifndef _DIRECTX12RENDER_STATICSAMPLER_H_INCLUDED_
#define _DIRECTX12RENDER_STATICSAMPLER_H_INCLUDED_

#include <DirectX12Render/DirectX12Includes.h>

namespace DirectX12Render
{
	class StaticSamplerHelper
	{
		static void SetDefaultMembers(D3D12_STATIC_SAMPLER_DESC& description);
		static void SetBindingData(D3D12_STATIC_SAMPLER_DESC& description,
			unsigned shaderRegister, unsigned registerSpace,
			D3D12_SHADER_VISIBILITY shaderVisibility);

	public:

		static D3D12_STATIC_SAMPLER_DESC GetDefaultPointSamplerDescription(
			unsigned shaderRegister, unsigned registerSpace = 0,
			D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
		static D3D12_STATIC_SAMPLER_DESC GetDefaultBilinearSamplerDescription(
			unsigned shaderRegister, unsigned registerSpace = 0,
			D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
		static D3D12_STATIC_SAMPLER_DESC GetDefaultTrilinearSamplerDescription(
			unsigned shaderRegister, unsigned registerSpace = 0,
			D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
		static D3D12_STATIC_SAMPLER_DESC GetDefaultAnisotropicSamplerDescription(
			unsigned shaderRegister, unsigned registerSpace = 0,
			D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
	};
}

#endif