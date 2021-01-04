// DirectX12Render/Resources/PipelineStateObject.h

#ifndef _DIRECTX12RENDER_PIPELINESTATEOBJECT_H_INCLUDED_
#define _DIRECTX12RENDER_PIPELINESTATEOBJECT_H_INCLUDED_

#include <Core/DataStructures/Pool.hpp>
#include <DirectX12Render/DirectX12Includes.h>

#include <DirectX12Render/Resources/VertexBuffer.h>

#include <vector>
#include <map>

namespace DirectX12Render
{
	class RootSignature;
	class Shader;

	struct RasterizerDescription
	{
		D3D12_RASTERIZER_DESC Description;

		// Sets the D3D pipeline default values, but with the front face counter
		// clockwise indices. Note that this sets backface culling.
		RasterizerDescription();

		// Sets the D3D pipeline default values, but with the front face counter
		// clockwise indices. Note that this sets backface culling.
		void SetToDefault();

		// Sets the D3D pipeline default values. Note that this sets backface culling.
		void SetToD3DDefault();

		void DisableCulling();

		void SetWireframeFillMode();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const RasterizerDescription& other) const;
		bool operator!=(const RasterizerDescription& other) const;
		bool operator<(const RasterizerDescription& other) const;
	};

	struct DepthStencilDescription
	{
		D3D12_DEPTH_STENCIL_DESC Description;

		// Sets the D3D pipeline default values. Note that this enabled depth testing.
		DepthStencilDescription();

		// Sets the D3D pipeline default values. Note that this enabled depth testing.
		void SetToD3DDefault();

		void DisableDepthWriting();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const DepthStencilDescription& other) const;
		bool operator!=(const DepthStencilDescription& other) const;
		bool operator<(const DepthStencilDescription& other) const;
	};

	struct BlendDescription
	{
		D3D12_BLEND_DESC Description;

		// Sets the D3D pipeline default values. Note that this disables blending.
		BlendDescription();

		// Sets the D3D pipeline default values. Note that this disables blending.
		void SetToD3DDefault();

		void EnableAlphaToCoverage();

		void SetToNonpremultipliedAlphaBlending();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const BlendDescription& other) const;
		bool operator!=(const BlendDescription& other) const;
		bool operator<(const BlendDescription& other) const;

	private:

		void SetToBlending();
	};

	struct StreamOutputDescription
	{
		Core::SimpleTypeVectorU<D3D12_SO_DECLARATION_ENTRY> StreamOutputDeclarationEntries;
		Core::SimpleTypeVectorU<unsigned> BufferStrides;
		unsigned RasterizedStream;

		StreamOutputDescription();

		void Initialize(const D3D12_STREAM_OUTPUT_DESC& description);

		D3D12_STREAM_OUTPUT_DESC ToD3DDescription() const;

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const StreamOutputDescription& other) const;
		bool operator!=(const StreamOutputDescription& other) const;
		bool operator<(const StreamOutputDescription& other) const;
	};

	struct GraphicsWrappedPipelineData
	{
		const Shader* PVertexShader;
		const Shader* PHullShader;
		const Shader* PDomainShader;
		const Shader* PGeometryShader;
		const Shader* PPixelShader;
		const VertexInputLayout* PVertexInputLayout;

		GraphicsWrappedPipelineData();
	};

	struct PipelineStateObjectDescription
	{
		RootSignature* PRootSignature;

	public: // Direct members.

		UINT NodeMask;
		D3D12_PIPELINE_STATE_FLAGS Flags;
		D3D12_CACHED_PIPELINE_STATE CachedPSO;	// We assume that the pointed cached blob
												// is persistently stored.

		PipelineStateObjectDescription();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const PipelineStateObjectDescription& other) const;
		bool operator!=(const PipelineStateObjectDescription& other) const;
		bool operator<(const PipelineStateObjectDescription& other) const;
	};

	struct GraphicsPipelineStateObjectDescription
		: public GraphicsWrappedPipelineData
		, public PipelineStateObjectDescription
	{
		RasterizerDescription RasterizerState;
		DepthStencilDescription DepthStencilState;
		BlendDescription BlendState;
		StreamOutputDescription StreamOutput;

	public: // Direct members.

		UINT SampleMask;
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
		UINT NumRenderTargets;
		DXGI_FORMAT RTVFormats[8];
		DXGI_FORMAT DSVFormat;
		DXGI_SAMPLE_DESC SampleDesc;

	public:

		GraphicsPipelineStateObjectDescription();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const GraphicsPipelineStateObjectDescription& other) const;
		bool operator!=(const GraphicsPipelineStateObjectDescription& other) const;
		bool operator<(const GraphicsPipelineStateObjectDescription& other) const;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC ToD3DDescription() const;

		static GraphicsPipelineStateObjectDescription FromD3DDescription(
			RootSignature* pRootSignature,
			const GraphicsWrappedPipelineData& wrappedData,
			const D3D12_GRAPHICS_PIPELINE_STATE_DESC& description);
	};

	class GraphicsPipelineStateObject
	{
		ComPtr<ID3D12PipelineState> m_Resource;

		GraphicsPipelineStateObjectDescription m_Description;

	public:

		const GraphicsPipelineStateObjectDescription& GetDescription() const;
		RootSignature* GetRootSignature() const;

		// A non-const pointer is needed to create a command buffer
		// and to set the pipeline state object to the command list.
		// Note that this can cause problems when using the same pipeline state object
		// in parallel at the same time. If there's some caching in the object
		// going on, this could even lead to crashes.
		ID3D12PipelineState* GetPipelineStateObject();
		
		void Initialize(ID3D12Device* device,
			const GraphicsPipelineStateObjectDescription& description);
	};

	struct ComputePipelineStateObjectDescription
		: public PipelineStateObjectDescription
	{
		const Shader* PComputeShader;

		ComputePipelineStateObjectDescription();

		// Note that if these operators are used often, we should compute a hash
		// on the members to speed up the comparison.
		bool operator==(const ComputePipelineStateObjectDescription& other) const;
		bool operator!=(const ComputePipelineStateObjectDescription& other) const;
		bool operator<(const ComputePipelineStateObjectDescription& other) const;

		D3D12_COMPUTE_PIPELINE_STATE_DESC ToD3DDescription() const;

		static ComputePipelineStateObjectDescription FromD3DDescription(
			RootSignature* pRootSignature,
			const Shader* pComputeShader,
			const D3D12_COMPUTE_PIPELINE_STATE_DESC& description);
	};

	class ComputePipelineStateObject
	{
		ComPtr<ID3D12PipelineState> m_Resource;

		ComputePipelineStateObjectDescription m_Description;

	public:

		const ComputePipelineStateObjectDescription& GetDescription() const;
		RootSignature* GetRootSignature() const;

		// A non-const pointer is needed to create a command buffer
		// and to set the pipeline state object to the command list.
		// Note that this can cause problems when using the same pipeline state object
		// in parallel at the same time. If there's some caching in the object
		// going on, this could even lead to crashes.
		ID3D12PipelineState* GetPipelineStateObject();

		void Initialize(ID3D12Device* device,
			const ComputePipelineStateObjectDescription& description);
	};

	class PipelineStateObjectManager
	{
		Core::ResourcePoolU<GraphicsPipelineStateObject> m_GraphicsPSOs;
		Core::ResourcePoolU<ComputePipelineStateObject> m_ComputePSOs;
		std::map<GraphicsPipelineStateObjectDescription, GraphicsPipelineStateObject*> m_GraphicsPSOMap;
		std::map<ComputePipelineStateObjectDescription, ComputePipelineStateObject*> m_ComputePSOMap;

	public:

		GraphicsPipelineStateObject* GetGraphicsPipelineStateObject(ID3D12Device* device,
			const GraphicsPipelineStateObjectDescription& description);
		ComputePipelineStateObject* GetComputePipelineStateObject(ID3D12Device* device,
			const ComputePipelineStateObjectDescription& description);
		GraphicsPipelineStateObject* GetGraphicsPipelineStateObject(ID3D12Device* device,
			RootSignature* pRootSignature,
			const GraphicsWrappedPipelineData& wrappedData,
			const D3D12_GRAPHICS_PIPELINE_STATE_DESC& description);
		ComputePipelineStateObject* GetComputePipelineStateObject(ID3D12Device* device,
			RootSignature* pRootSignature,
			const Shader* pComputeShader,
			const D3D12_COMPUTE_PIPELINE_STATE_DESC& description);
	};
}

#endif