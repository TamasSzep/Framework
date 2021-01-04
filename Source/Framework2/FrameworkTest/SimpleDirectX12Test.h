// FrameworkTest/SimpleDirectX12Test.h

#ifndef _FRAMEWORKTEST_SIMPLEDIRECTX12TEST_H_INCLUDED_
#define _FRAMEWORKTEST_SIMPLEDIRECTX12TEST_H_INCLUDED_

#include <WindowsApplication/Application.hpp>

#include <Core/DataStructures/Properties.h>
#include <EngineBuildingBlocks/Graphics/Camera/FreeCamera.h>
#include <EngineBuildingBlocks/SceneNode.h>
#include <EngineBuildingBlocks/Graphics/ViewFrustumCuller.h>
#include <DirectX12Render/Managers.h>
#include <DirectX12Render/Utilities/Utilities.h>
#include <DirectX12Render/Resources/VertexBuffer.h>
#include <DirectX12Render/Resources/Texture2D.h>
#include <DirectX12Render/Resources/DepthStencilBuffer.h>
#include <DirectX12Render/Resources/RenderTargetBuffer.h>
#include <DirectX12Render/Resources/IndexBuffer.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace FrameworkTest
{
	struct DX12T_Material
	{
		DirectX12Render::RootSignature* PRootSignature;
		DirectX12Render::GraphicsPipelineStateObject* PPipelineStateObject;

		DirectX12Render::Texture2D* DiffuseTexture;

		bool IsUsingMaterialColors;
		glm::vec3 DiffuseColor;
		glm::vec3 Specularcolor;
		bool IsUsingVertexColor;

		bool operator==(const DX12T_Material& other) const;
		bool operator!=(const DX12T_Material& other) const;
		bool operator<(const DX12T_Material& other) const;
	};

	struct DX12T_RenderTask
	{
		unsigned SceneNodeIndex;
		unsigned ObjectCBIndex;
		unsigned MaterialIndex;
		EngineBuildingBlocks::Math::AABoundingBox BoundingBox;
		DirectX12Render::IndexedPrimitive Primitive;
	};

	struct DX12T_BackbufferResources
	{
		ComPtr<ID3D12Resource> RenderTarget;
	};

	struct DX12T_FrameResources
	{
		UINT64 FenceValue;
		ComPtr<ID3D12CommandAllocator> CommandAllocator;

		DirectX12Render::ConstantBuffer LightingConstantBuffer;
		DirectX12Render::ConstantBuffer RenderPassConstantBuffer;
		DirectX12Render::ConstantBuffer MaterialConstantBuffer;
		DirectX12Render::ConstantBuffer ObjectConstantBuffer;

		DirectX12Render::Texture2D ColorTexture;
		DirectX12Render::ReadbackBuffer* ColorReadbackBuffer;
		bool HasPrintScreenResult;

		DX12T_FrameResources();
	};

	class SimpleDirectX12Test : public WindowsApplication::Application<SimpleDirectX12Test>
	{
		DirectX12Render::Managers m_DX12M;
		DirectX12Render::Utilites m_DX12U;
		EngineBuildingBlocks::SceneNodeHandler m_SceneNodeHandler;

		Core::SimpleTypeSetU<DX12T_Material> m_Materials;
		Core::SimpleTypeUnorderedVectorU<DX12T_RenderTask> m_RenderTasks;
		EngineBuildingBlocks::Graphics::FreeCamera m_Camera;

		EngineBuildingBlocks::Graphics::Vertex_SOA_Data m_VertexData;

		std::vector<DX12T_FrameResources> m_FrameResources;
		std::vector<DX12T_BackbufferResources> m_BackbufferResources;

		// NOTE THAT SOME OF THESE OBJECT MIGHT SHOULD BE DUPLICATED IN ORDER TO AVOID WAITING
		// FOR SYNCHRONIZATION!

		DirectX12Render::StreamedDescriptorHeap m_RTV_DescriptorHeap;
		DirectX12Render::StreamedDescriptorHeap m_DSV_DescriptorHeap;
		DirectX12Render::StreamedDescriptorHeap m_CBV_SRV_UAV_DescriptorHeap;

		DirectX12Render::DepthStencilBuffer m_DepthStencilBuffer;
		DirectX12Render::RenderTargetBuffer m_RenderTargetBuffer;

	public: // Misc rendering.

		void RenderBackgroundCubemap();
		void RenderText();

	public: // View frustum culling.

		Core::ThreadPool m_ThreadPool;
		EngineBuildingBlocks::Graphics::ViewFrustumCuller m_ViewFrustumCuller;

		bool m_IsSceneNodeVectorsUpToDate;
		Core::IndexVectorU m_RenderableSceneNodeIndices;
		Core::IndexVectorU m_RenderTaskIndices;

		Core::IndexVectorU m_ViewFrustumCulledRenderTaskIndices;

		unsigned m_CountDrawCalls;

		void UpdateRenderableSceneNodeData();

	public: // For writing statistical data in fullscreen mode.

		DirectX12Render::UpdatableTextRenderTask m_Statistics_TRT;

	public:

		SimpleDirectX12Test(int argc, char *argv[]);
		~SimpleDirectX12Test();

		void InitializeMain();
		void InitializeRendering();
		void DestroyMain();
		void DestroyRendering();

		void DerivedPreUpdate();
		void DerivedPostUpdate(EngineBuildingBlocks::PostUpdateContext& context);
		void DerivedRender(EngineBuildingBlocks::RenderContext& context);

		void DerivedPresent();
		
		bool OnEvent(UINT message, WPARAM wParam, LPARAM lParam);

		void DerivedParseCommandLineArgs(int argc, char *argv[]);

	private: // Event listening.

		unsigned m_ExitECI;
		unsigned m_ScreenshotECI;

		void RegisterEvents();

	public:

		unsigned GetExitECI() const;

		bool HandleEvent(const EngineBuildingBlocks::Event* _event);

	private: // Updating.

		void UpdateCameras();
		void UpdateSceneNodes();

	private: // Configuration.

		Core::Properties m_Configuration;

		std::string m_Title;

		unsigned m_Width = 0;
		unsigned m_Height = 0;
		unsigned m_MultisampleCount = 0;

		unsigned m_BackbufferFrameCount = 0;
		unsigned m_MaximumFrameLatency = 0;
		unsigned m_RenderQueueSize = 0;
		bool m_IsWindowed = false;

		bool m_IsRenderingWireframe = false;
		bool m_IsUsingMaterialColors = false;
		bool m_IsUsingVertexColors = false;

		unsigned m_SceneIndex = 0;

		void LoadConfiguration();

	protected:

		// Adapter info.
		bool m_IsUsingWarpDevice;

	private: // Screen printing.

		unsigned m_IsPrintingScreen;
		Core::ByteVectorU m_PrintScreenBuffer;

	////////////////////////////////////////////////////////////////////////////

	private:

		static const bool c_IsUsingWindowedNoWaitingTrick = false;

		// Pipeline objects.

		// NOTE THAT SOME OF THESE OBJECT MIGHT SHOULD BE DUPLICATED IN ORDER TO AVOID WAITING
		// FOR SYNCHRONIZATION!

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;

		// Synchronization objects.
		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;

		void LoadPipeline();
		void LoadAssets();
		void PopulateCommandList();
		void MoveToNextFrame();
		void WaitForGpu();

		void UpdateLightingCBData(unsigned frameIndex);
		void UpdateRenderPassCBData(unsigned frameIndex);
		void UpdateMaterialCBData(unsigned frameIndex);
		void UpdateObjectCBData(unsigned frameIndex);

		DirectX12Render::Texture2D* GetTexture(const std::string& textureName);
		DirectX12Render::Texture2D* GetTexture(const std::string& diffuseTextureName,
			const std::string& opacityTextureName);

		DirectX12Render::RootSignature* GetDefaultRootSignature();
		DirectX12Render::GraphicsPipelineStateObject* GetOpaquePSO(
			const DirectX12Render::VertexInputLayout& vertexInputLayoutDescription,
			bool hasMaterialColors, bool hasVertexColors);
		DirectX12Render::GraphicsPipelineStateObject* GetTransparentPSO(
			const DirectX12Render::VertexInputLayout& vertexInputLayoutDescription,
			bool hasMaterialColors, bool hasVertexColors);

		unsigned GetMaterialIndex(unsigned objectIndex,
			const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes,
			const EngineBuildingBlocks::Graphics::ModelInstantiationResult& instRes,
			const DirectX12Render::VertexInputLayout& vertexInputLayoutDescription);
		unsigned AddRenderTask(unsigned objectIndex,
			const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes,
			const EngineBuildingBlocks::Graphics::ModelInstantiationResult& instRes,
			DirectX12Render::VertexBuffer* pVertexBuffer,
			DirectX12Render::IndexBuffer* pIndexBuffer);
	
		void RenderObjects();

		void OnFrameCompleted();
	};
}

#endif