// DirectX11Render/Resources/PipelineState.h

#ifndef _DIRECTX11RENDER_PIPELINESTATE_INCLUDED_
#define _DIRECTX11RENDER_PIPELINESTATE_INCLUDED_

#include <DirectX11Render/Resources/Shader.h>
#include <DirectX11Render/Resources/VertexBuffer.h>

#include <utility>
#include <map>

namespace DirectX11Render
{
	struct RasterizerStateDescription
	{
		D3D11_RASTERIZER_DESC Description;

		// Sets the D3D pipeline default values, but with the front face counter
		// clockwise indices. Note that this sets backface culling.
		RasterizerStateDescription();

		// Sets the D3D pipeline default values, but with the front face counter
		// clockwise indices. Note that this sets backface culling.
		void SetToDefault();

		// Sets the D3D pipeline default values. Note that this sets backface culling.
		void SetToD3DDefault();

		void DisableCulling();

		void SetWireframeFillMode();

		void EnableMultisampling();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const RasterizerStateDescription& other) const;
		bool operator!=(const RasterizerStateDescription& other) const;
		bool operator<(const RasterizerStateDescription& other) const;
	};

	struct DepthStencilStateDescription
	{
		D3D11_DEPTH_STENCIL_DESC Description;

		// Sets the D3D pipeline default values. Note that this enabled depth testing.
		DepthStencilStateDescription();

		// Sets the D3D pipeline default values. Note that this enabled depth testing.
		void SetToD3DDefault();

		void DisableDepthWriting();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const DepthStencilStateDescription& other) const;
		bool operator!=(const DepthStencilStateDescription& other) const;
		bool operator<(const DepthStencilStateDescription& other) const;
	};

	struct BlendStateDescription
	{
		D3D11_BLEND_DESC Description;

		// Sets the D3D pipeline default values. Note that this disables blending.
		BlendStateDescription();

		// Sets the D3D pipeline default values. Note that this disables blending.
		void SetToD3DDefault();

		void EnableAlphaToCoverage();

		void SetToNonpremultipliedAlphaBlending();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const BlendStateDescription& other) const;
		bool operator!=(const BlendStateDescription& other) const;
		bool operator<(const BlendStateDescription& other) const;

	private:

		void SetToBlending();
	};

	struct SamplerStateDescription
	{
		D3D11_SAMPLER_DESC Description;

		SamplerStateDescription();

		void SetToD3DDefault();
		void SetToNearest();
		void SetToBilinear();
		void SetToTrilinear();
		void SetToAnisotropic(unsigned countSamples = D3D11_MAX_MAXANISOTROPY);

		void SetToWrap();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const SamplerStateDescription& other) const;
		bool operator!=(const SamplerStateDescription& other) const;
		bool operator<(const SamplerStateDescription& other) const;
	};

	struct PipelineStateDescription
	{
		Core::SimpleTypeVectorU<const Shader*> Shaders;
		VertexInputLayout InputLayout;
		RasterizerStateDescription RasterizerState;
		DepthStencilStateDescription DepthStencilState;
		BlendStateDescription BlendState;

		std::vector<std::pair<SamplerStateDescription, DirectXRender::ShaderFlagType>> SamplerStates;

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const PipelineStateDescription& other) const;
		bool operator!=(const PipelineStateDescription& other) const;
		bool operator<(const PipelineStateDescription& other) const;
	};

	struct PipelineStateSettingContext
	{
		unsigned StencilReference;
		float BlendFactor[4];
		unsigned BlendSampleMask;

		PipelineStateSettingContext();
	};

	struct PipelineState
	{
		ID3D11VertexShader* VS;
		ID3D11HullShader* HS;
		ID3D11DomainShader* DS;
		ID3D11GeometryShader* GS;
		ID3D11PixelShader* PS;
		ID3D11ComputeShader* CS;

		ID3D11InputLayout* InputLayout;

		ID3D11RasterizerState* RasterizerState;
		ID3D11DepthStencilState* DepthStencilState;
		ID3D11BlendState* BlendState;

		unsigned SamplerStateStart[7];
		Core::SimpleTypeVectorU<ID3D11SamplerState*> SamplerStates;

		PipelineState();

		void SetForContext(ID3D11DeviceContext* context,
			const PipelineStateSettingContext& pssContext = PipelineStateSettingContext()) const;
	};

	class PipelineStateManager
	{
	public:

		using InputLayoutResource = ComPtr<ID3D11InputLayout>;
		using RasterizerStateResource = ComPtr<ID3D11RasterizerState>;
		using DepthStencilStateResource = ComPtr<ID3D11DepthStencilState>;
		using BlendStateResource = ComPtr<ID3D11BlendState>;
		using SamplerStateResource = ComPtr<ID3D11SamplerState>;

	private:

		Core::ResourcePoolU<InputLayoutResource> m_InputLayouts;
		Core::ResourcePoolU<RasterizerStateResource> m_RasterizerStates;
		Core::ResourcePoolU<DepthStencilStateResource> m_DepthStencilStates;
		Core::ResourcePoolU<BlendStateResource> m_BlendStates;
		Core::ResourcePoolU<SamplerStateResource> m_SamplerStates;
		std::map<VertexInputLayout, InputLayoutResource*> m_InputLayoutMap;
		std::map<RasterizerStateDescription, RasterizerStateResource*> m_RasterizerStateMap;
		std::map<DepthStencilStateDescription, DepthStencilStateResource*> m_DepthStencilStateMap;
		std::map<BlendStateDescription, BlendStateResource*> m_BlendStateMap;
		std::map<SamplerStateDescription, SamplerStateResource*> m_SamplerStateMap;

		Core::ResourceUnorderedVectorU<PipelineState> m_PipelineStates;
		std::map<PipelineStateDescription, unsigned> m_PipelineStateMap;

		void CreatePipelineState(ID3D11Device* device, const PipelineStateDescription& description,
			PipelineState& pipelineState);

	public:

		const PipelineState& GetPipelineState(unsigned index) const;

		unsigned GetPipelineStateIndex(ID3D11Device* device, const PipelineStateDescription& description);
		const PipelineState& GetPipelineState(ID3D11Device* device, const PipelineStateDescription& description);
	};
}

#endif