// DirectX12Render/Resources/RootSignature.h

#ifndef _DIRECTX12RENDER_ROOTSIGNATURE_H_INCLUDED_
#define _DIRECTX12RENDER_ROOTSIGNATURE_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <Core/DataStructures/Pool.hpp>
#include <DirectX12Render/Resources/StaticSampler.h>

#include <vector>
#include <map>

namespace DirectX12Render
{
	struct RootSignatureDescription
	{
		Core::SimpleTypeVectorU<D3D12_ROOT_PARAMETER> RootParameters;
		Core::SimpleTypeVectorU<D3D12_DESCRIPTOR_RANGE> DescriptorRanges;
		Core::SimpleTypeVectorU<D3D12_STATIC_SAMPLER_DESC> StaticSamplers;
		D3D12_ROOT_SIGNATURE_FLAGS Flags;

		RootSignatureDescription();
		void Initialize(const D3D12_ROOT_SIGNATURE_DESC& description);
		void Initialize(unsigned countRootParameters, const D3D12_ROOT_PARAMETER* rootParameters,
			unsigned countStaticSamplers = 0, const D3D12_STATIC_SAMPLER_DESC* staticSamplers = nullptr,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

		D3D12_ROOT_SIGNATURE_DESC ToD3DDescription() const;

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const RootSignatureDescription& other) const;
		bool operator!=(const RootSignatureDescription& other) const;
		bool operator<(const RootSignatureDescription& other) const;

		// Constants.
		static const D3D12_ROOT_SIGNATURE_FLAGS c_IA_VertexPixelShader_Flags = (D3D12_ROOT_SIGNATURE_FLAGS)
			((int)D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			(int)D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			(int)D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			(int)D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
	};

	class RootSignature
	{
		ComPtr<ID3D12RootSignature> m_Resource;

	public:

		// A non-const pointer is needed to create a pipeline state object
		// and to set the root signature to the command list.
		// Note that this can cause problems when using the same root signature object
		// in parallel at the same time. If there's some caching in the object
		// going on, this could even lead to crashes.
		ID3D12RootSignature* GetRootSignature();

		void Initialize(ID3D12Device* device,
			const RootSignatureDescription& description);
	};

	class RootSignatureManager
	{
		Core::ResourcePoolU<RootSignature> m_RootSignatures;
		std::map<RootSignatureDescription, RootSignature*> m_RootSignatureMap;

	public:

		RootSignature* GetRootSignature(ID3D12Device* device,
			const RootSignatureDescription& description);
		RootSignature* GetRootSignature(ID3D12Device* device,
			const D3D12_ROOT_SIGNATURE_DESC& description);
	};
}

#endif