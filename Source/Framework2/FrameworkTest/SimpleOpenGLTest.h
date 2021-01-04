// FrameworkTest/SimpleOpenGLTest.h

#ifndef _FRAMEWORKTEST_SIMPLEOPENGLTEST_H_INCLUDED_
#define _FRAMEWORKTEST_SIMPLEOPENGLTEST_H_INCLUDED_

#include <EngineBuildingBlocks/Application/Application.hpp>

#include <Core/DataStructures/Properties.h>
#include <EngineBuildingBlocks/Graphics/Camera/FreeCamera.h>
#include <EngineBuildingBlocks/SceneNode.h>
#include <EngineBuildingBlocks/Graphics/ViewFrustumCuller.h>
#include <EngineBuildingBlocks/Graphics/TaskSorter.h>
#include <OpenGLRender/Managers.h>
#include <OpenGLRender/Utilities/Utilities.h>
#include <OpenGLRender/Resources/VertexBuffer.h>
#include <OpenGLRender/Resources/IndexBuffer.h>
#include <OpenGLRender/Resources/UniformBuffer.h>
#include <OpenGLRender/Window.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace FrameworkTest
{
	enum class OGLT_PS_Flags : unsigned char
	{
		Invalid = 0x80,
		None = 0x00,
		CullingEnabled = 0x01,
		AlphaToCoverageEnabled = 0x02
	};

	UseEnumAsFlagSet(OGLT_PS_Flags);

	struct OGLT_Material
	{
		// Pipeline state.
		OpenGLRender::ShaderProgram* ShaderProgram;
		OGLT_PS_Flags PS_Flags;

		// Resource state.

		std::vector<OpenGLRender::UniformBuffer*> UniformBuffers;
		OpenGLRender::Texture2D* DiffuseTexture;
		
		unsigned MaterialCBIndex;

		bool operator==(const OGLT_Material& other) const;
		bool operator!=(const OGLT_Material& other) const;
		bool operator<(const OGLT_Material& other) const;
	};

	struct OGLT_RenderTask
	{
		unsigned SceneNodeIndex;
		EngineBuildingBlocks::Math::AABoundingBox BoundingBox;
		
		unsigned MaterialIndex;
		unsigned ObjectCBIndex;
		OpenGLRender::IndexedPrimitive Primitive;
	};

	struct ObjectCBType
	{
		glm::mat4 ModelMatrix;
		glm::mat4 InverseModelMatrix;
		glm::mat4 ModelViewProjectionMatrix;
	};

	struct MaterialCBType
	{
		glm::vec3 DiffuseColor;
		float _Padding0;
		glm::vec3 SpecularColor;
		float _Padding1;
	};

	struct OGLT_RenderTaskGroup
	{
		Core::SimpleTypeUnorderedVectorU<OGLT_RenderTask> RenderTasks;
		
		bool AreSceneNodeVectorsUpToDate;
		Core::IndexVectorU RenderTaskIndices;
		Core::IndexVectorU ProcessedRTIndices;

		OGLT_RenderTaskGroup();

		unsigned AddRenderTask(const OGLT_RenderTask& renderTask);

		void Update(
			EngineBuildingBlocks::Graphics::ViewFrustumCuller& culler,
			EngineBuildingBlocks::Graphics::TaskSorter& taskSorter,
			EngineBuildingBlocks::Graphics::Camera& camera,
			Core::ThreadPool& threadPool,
			EngineBuildingBlocks::SceneNodeHandler& sceneNodeHandler,
			EngineBuildingBlocks::Graphics::TaskSortType sortType);

		void UpdateObjectCBData(
			Core::SimpleTypeVectorU<ObjectCBType>& objectCBDatas,
			EngineBuildingBlocks::Graphics::Camera& camera,
			EngineBuildingBlocks::SceneNodeHandler& sceneNodeHandler);
	};

	class SimpleOpenGLTest
		: public EngineBuildingBlocks::Application<SimpleOpenGLTest>
		, public EngineBuildingBlocks::IEventListener
	{
		OpenGLRender::Window<SimpleOpenGLTest> m_Window;

		OpenGLRender::Managers m_OGLM;
		OpenGLRender::Utilites m_OGLU;

		EngineBuildingBlocks::SceneNodeHandler m_SceneNodeHandler;
		
		EngineBuildingBlocks::Graphics::FreeCamera m_Camera;

		Core::ResourceUnorderedVectorU<OGLT_Material> m_Materials;
		std::map<OGLT_Material, unsigned> m_MaterialMap;

	private: // Graphics resources.

		OpenGLRender::VertexArrayObject m_VertexArrayObject;
		OpenGLRender::VertexBuffer m_VertexBuffer;
		OpenGLRender::IndexBuffer m_IndexBuffer;
		EngineBuildingBlocks::Graphics::VertexInputLayout m_InputLayout;

		OpenGLRender::UniformBuffer m_LightingCB;
		OpenGLRender::UniformBuffer m_RenderPassCB;
		OpenGLRender::UniformBuffer m_MaterialCB;
		OpenGLRender::UniformBuffer m_ObjectCB;
		Core::SimpleTypeVectorU<ObjectCBType> m_ObjectCBData;
		Core::SimpleTypeVectorU<MaterialCBType> m_MaterialCBData;

		// Depth and render targets for multisampled rendering.
		OpenGLRender::Texture2D m_ColorBuffer;
		OpenGLRender::Texture2D m_DepthBuffer;

	private: // For writing statistical data in fullscreen mode.

		OpenGLRender::UpdatableTextRenderTask m_Statistics_TRT;

	private: // Render task handling.

		std::vector<OGLT_RenderTaskGroup> m_RenderTaskGroups;

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

		bool m_IsWindowed = false;

		bool m_IsRenderingWireframe = false;
		bool m_IsUsingMaterialColors = false;

		unsigned m_SceneIndex = 0;

		void LoadConfiguration();

	private: // Scene loading.

		void LoadPipeline();
		void LoadAssets();

		void SetLightingCBData();

		OpenGLRender::Texture2D* GetTexture(const std::string& textureName);
		OpenGLRender::Texture2D* GetTexture(const std::string& diffuseTextureName,
			const std::string& opacityTextureName);

		void SetPipelineState(
			const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes,
			const EngineBuildingBlocks::Graphics::MaterialData& materialData,
			OGLT_Material* pMaterial);
		void SetResourceState(
			const EngineBuildingBlocks::Graphics::MaterialData& materialData,
			OGLT_Material* pMaterial);
		unsigned GetMaterialIndex(unsigned objectIndex,
			const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes,
			const EngineBuildingBlocks::Graphics::ModelInstantiationResult& instRes, bool& isOpaque);
		unsigned AddRenderTask(unsigned objectIndex,
			const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes,
			const EngineBuildingBlocks::Graphics::ModelInstantiationResult& instRes,
			const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& vertexData);
	
	public:

		SimpleOpenGLTest(int argc, char *argv[]);
		~SimpleOpenGLTest();

		void InitializeMain();
		void InitializeRendering();
		void DestroyMain();
		void DestroyRendering();
		int EnterMainEventLoop();

		void DerivedPreUpdate();
		void DerivedPostUpdate(EngineBuildingBlocks::PostUpdateContext& context);
		void DerivedRender(EngineBuildingBlocks::RenderContext& context);

		void DerivedPresent();

		void DerivedParseCommandLineArgs(int argc, char *argv[]);
	};
}

#endif