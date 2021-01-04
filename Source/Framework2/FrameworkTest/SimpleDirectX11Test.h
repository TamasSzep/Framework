// FrameworkTest/SimpleDirectX11Test.h

#ifndef _FRAMEWORKTEST_SIMPLEDIRECTX11TEST_H_INCLUDED_
#define _FRAMEWORKTEST_SIMPLEDIRECTX11TEST_H_INCLUDED_

#include <WindowsApplication/Application.hpp>

#include <Core/DataStructures/Properties.h>
#include <EngineBuildingBlocks/Graphics/Camera/FreeCamera.h>
#include <EngineBuildingBlocks/SceneNode.h>
#include <EngineBuildingBlocks/Graphics/ViewFrustumCuller.h>
#include <EngineBuildingBlocks/Graphics/TaskSorter.h>
#include <DirectX11Render/Managers.h>
#include <DirectX11Render/Utilities/Utilities.h>
#include <DirectX11Render/Resources/VertexBuffer.h>
#include <DirectX11Render/Resources/IndexBuffer.h>
#include <DirectX11Render/Resources/ConstantBuffer.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace FrameworkTest
{
	struct DX11T_Material
	{
		unsigned PipelineStateIndex;
		unsigned ResourceStateIndex;
		
		unsigned MaterialCBIndex;

		bool operator==(const DX11T_Material& other) const;
		bool operator!=(const DX11T_Material& other) const;
		bool operator<(const DX11T_Material& other) const;
	};

	struct DX11T_RenderTask
	{
		unsigned SceneNodeIndex;
		EngineBuildingBlocks::Math::AABoundingBox BoundingBox;
		
		unsigned MaterialIndex;
		unsigned ObjectCBIndex;
		DirectX11Render::IndexedPrimitive Primitive;
	};

	struct RenderTaskGroup
	{
		Core::SimpleTypeUnorderedVectorU<DX11T_RenderTask> RenderTasks;
		
		bool AreSceneNodeVectorsUpToDate;
		Core::IndexVectorU RenderTaskIndices;
		Core::IndexVectorU ProcessedRTIndices;

		RenderTaskGroup();

		unsigned AddRenderTask(const DX11T_RenderTask& renderTask);

		void Update(
			EngineBuildingBlocks::Graphics::ViewFrustumCuller& culler,
			EngineBuildingBlocks::Graphics::TaskSorter& taskSorter,
			EngineBuildingBlocks::Graphics::Camera& camera,
			Core::ThreadPool& threadPool,
			EngineBuildingBlocks::SceneNodeHandler& sceneNodeHandler,
			EngineBuildingBlocks::Graphics::TaskSortType sortType);

		void UpdateObjectCBData(DirectX11Render::ConstantBuffer& objectCB,
			EngineBuildingBlocks::Graphics::Camera& camera,
			EngineBuildingBlocks::SceneNodeHandler& sceneNodeHandler);
	};

	class SimpleDirectX11Test : public WindowsApplication::Application<SimpleDirectX11Test>
	{
		DirectX11Render::Managers m_DX11M;
		DirectX11Render::Utilites m_DX11U;

		EngineBuildingBlocks::SceneNodeHandler m_SceneNodeHandler;
		
		EngineBuildingBlocks::Graphics::FreeCamera m_Camera;

		Core::SimpleTypeSetU<DX11T_Material> m_Materials;

	private: // Graphics resources.

		DirectX11Render::VertexBuffer m_VertexBuffer;
		DirectX11Render::IndexBuffer m_IndexBuffer;

		DirectX11Render::ConstantBuffer m_LightingCB;
		DirectX11Render::ConstantBuffer m_RenderPassCB;
		DirectX11Render::ConstantBuffer m_MaterialCB;
		DirectX11Render::ConstantBuffer m_ObjectCB;

		unsigned m_CountMaterialCBData;
		unsigned m_CountObjectCBData;

		// Depth and render targets for multisampled rendering.
		DirectX11Render::Texture2D m_ColorBuffer;
		DirectX11Render::Texture2D m_DepthBuffer;

	protected: // DirectX 11 API pointers.

		ComPtr<ID3D11Device> m_Device;
		ComPtr<ID3D11DeviceContext> m_DeviceContext;
		ComPtr<IDXGISwapChain1> m_SwapChain;

		ComPtr<ID3D11Texture2D> m_ColorBackBuffer;
		ComPtr<ID3D11RenderTargetView> m_ColorBackBufferRTV;

	private: // For writing statistical data in fullscreen mode.

		DirectX11Render::UpdatableTextRenderTask m_Statistics_TRT;

	private: // Render task handling.

		std::vector<RenderTaskGroup> m_RenderTaskGroups;

		Core::ThreadPool m_ThreadPool;
		EngineBuildingBlocks::Graphics::ViewFrustumCuller m_ViewFrustumCuller;
		EngineBuildingBlocks::Graphics::TaskSorter m_TaskSorter;

		unsigned m_CountDrawCalls;

		void UpdateRenderableSceneNodeData();

	private: // Event listening.

		unsigned m_ExitECI;
		unsigned m_ScreenshotECI;

		void RegisterEvents();

	public:

		unsigned GetExitECI() const;

		bool HandleEvent(const EngineBuildingBlocks::Event* _event);

	private: // Rendering.

		void RenderObjects();
		void RenderBackgroundCubemap();
		void RenderText();

	private: // Updating.

		// Pre-update.
		void UpdateRenderPassCBData();
		void UpdateObjectCBData();

		// Post-update.
		void UpdateCameras();
		void UpdateSceneNodes();

	private: // Configuration.

		Core::Properties m_Configuration;

		std::string m_Title;

		unsigned m_Width = 0;
		unsigned m_Height = 0;
		unsigned m_MultisampleCount = 0;

		unsigned m_BackbufferFrameCount = 0;
		bool m_IsWindowed = false;

		bool m_IsRenderingWireframe = false;
		bool m_IsUsingMaterialColors = false;

		unsigned m_SceneIndex = 0;

		void LoadConfiguration();

	private: // Scene loading.

		void LoadPipeline();
		void LoadAssets();

		void SetLightingCBData();

		DirectX11Render::Texture2D* GetTexture(const std::string& textureName);
		DirectX11Render::Texture2D* GetTexture(const std::string& diffuseTextureName,
			const std::string& opacityTextureName);

		unsigned GetPipelineStateIndex(
			const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes,
			const EngineBuildingBlocks::Graphics::MaterialData& materialData);
		unsigned GetResourceStateIndex(const EngineBuildingBlocks::Graphics::MaterialData& materialData);
		unsigned GetMaterialIndex(unsigned objectIndex,
			const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes,
			const EngineBuildingBlocks::Graphics::ModelInstantiationResult& instRes, bool& isOpaque);
		unsigned AddRenderTask(unsigned objectIndex,
			const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes,
			const EngineBuildingBlocks::Graphics::ModelInstantiationResult& instRes,
			const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& vertexData);
	
	public:

		SimpleDirectX11Test(int argc, char *argv[]);
		~SimpleDirectX11Test();

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
	};
}

#endif