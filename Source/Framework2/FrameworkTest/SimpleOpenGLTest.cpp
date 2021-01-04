// FrameworkTest/SimpleOpenGLTest.cpp

#include <FrameworkTest/SimpleOpenGLTest.h>

#include <EngineBuildingBlocks/Input/DefaultInputBinder.h>
#include <EngineBuildingBlocks/Graphics/Lighting/Lighting1.h>
#include <OpenGLRender/GLFW.h>
#include <OpenGLRender/Utilities/RenderPassCB.h>
#include <DirectX11Render/GraphicsDevice.h>
#include <DirectX11Render/Utilities/RenderPassCB.h>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace EngineBuildingBlocks::Input;
using namespace EngineBuildingBlocks::Math;
using namespace OpenGLRender;
using namespace FrameworkTest;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SceneData
{
	std::string Name;
	glm::vec3 OrientationYPR;
	glm::vec3 Position;
	float Scaler;
	unsigned OpacityChannelIndex;
};

const SceneData c_SceneData[] =
{
	/* Crytek sponza */ { "crytek-sponza/sponza.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.02f, 0 },
	/* San Miguel    */ { "san_miguel/san-miguel.obj", glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 3 },
	/* Molecules     */ { "molecules/1/vmdspheres.obj", glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0 }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

const bool c_IsOptimizingMeshes = true;
const bool c_IsClearingRTV = false;

const unsigned c_MaxCountLights = 16;
const unsigned c_MaxCountTextures = 1024;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool OGLT_Material::operator==(const OGLT_Material& other) const
{
	NumericalEqualCompareBlock(ShaderProgram);
	NumericalEqualCompareBlock(PS_Flags);
	NumericalEqualCompareBlock(UniformBuffers);
	NumericalEqualCompareBlock(DiffuseTexture);
	NumericalEqualCompareBlock(MaterialCBIndex);
	return true;
}

bool OGLT_Material::operator!=(const OGLT_Material& other) const
{
	return !(*this == other);
}

bool OGLT_Material::operator<(const OGLT_Material& other) const
{
	NumericalLessCompareBlock(ShaderProgram);
	NumericalLessCompareBlock(PS_Flags);
	NumericalLessCompareBlock(UniformBuffers);
	NumericalLessCompareBlock(DiffuseTexture);
	NumericalLessCompareBlock(MaterialCBIndex);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SimpleOpenGLTest::DerivedParseCommandLineArgs(int argc, char *argv[])
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

OGLT_RenderTaskGroup::OGLT_RenderTaskGroup()
	: AreSceneNodeVectorsUpToDate(true)
{
}

unsigned OGLT_RenderTaskGroup::AddRenderTask(const OGLT_RenderTask& renderTask)
{
	AreSceneNodeVectorsUpToDate = false;
	return RenderTasks.Add(renderTask);
}

void OGLT_RenderTaskGroup::Update(ViewFrustumCuller& culler, TaskSorter& taskSorter, Camera& camera,
	Core::ThreadPool& threadPool, SceneNodeHandler& sceneNodeHandler, TaskSortType sortType)
{
	// Updating renderable scene node indices.
	if (!AreSceneNodeVectorsUpToDate)
	{
		RenderTaskIndices.Resize(RenderTasks.GetSize());

		auto it = RenderTasks.GetBeginIterator();
		auto end = RenderTasks.GetEndIterator();
		for (int i = 0; it != end; ++i, ++it)
		{
			RenderTaskIndices[i] = RenderTasks.ToIndex(it);
		}

		AreSceneNodeVectorsUpToDate = true;
	}

	culler.ViewFrustumCull(camera, threadPool, sceneNodeHandler, RenderTasks.GetArray(),
		RenderTaskIndices.GetArray(), ProcessedRTIndices, RenderTasks.GetSize());

	taskSorter.Sort(camera, threadPool, sceneNodeHandler, RenderTasks.GetArray(), ProcessedRTIndices.GetArray(),
		ProcessedRTIndices.GetSize(), sortType);
}

void OGLT_RenderTaskGroup::UpdateObjectCBData(Core::SimpleTypeVectorU<ObjectCBType>& objectCBDatas,
	Camera& camera, SceneNodeHandler& sceneNodeHandler)
{
	auto& viewProjectionMatrix = camera.GetViewProjectionMatrix();

	glm::mat4 worldMatrix;

	auto it = RenderTasks.GetBeginIterator();
	auto end = RenderTasks.GetEndIterator();
	for (; it != end; ++it)
	{
		auto& renderTask = *it;
		unsigned sceneNodeIndex = renderTask.SceneNodeIndex;
		auto& objectCBData = objectCBDatas[it->ObjectCBIndex];

		worldMatrix = sceneNodeHandler.UnsafeGetScaledWorldTransformation(sceneNodeIndex).AsMatrix4();
		objectCBData.ModelMatrix = worldMatrix;
		objectCBData.InverseModelMatrix = sceneNodeHandler.UnsafeGetInverseScaledWorldTransformation(sceneNodeIndex).AsMatrix4();
		objectCBData.ModelViewProjectionMatrix = viewProjectionMatrix * worldMatrix;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

SimpleOpenGLTest::SimpleOpenGLTest(int argc, char *argv[])
	: EngineBuildingBlocks::Application<SimpleOpenGLTest>(argc, argv)
	, m_Window(this)
	, m_OGLM(m_PathHandler)
	, m_Camera(&m_SceneNodeHandler, &m_KeyHandler, &m_MouseHandler)
	, m_CountDrawCalls(0)
{
	LoadConfiguration();

	m_Window.SetTitle(m_Title);

	m_Statistics_TRT.SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	m_Statistics_TRT.SetPosition(TextPositionConstant::TopLeft);
}

SimpleOpenGLTest::~SimpleOpenGLTest()
{
}

void SimpleOpenGLTest::InitializeMain()
{
	m_Window.Initialize(m_Width, m_Height, m_MultisampleCount, true, !m_IsWindowed);
	m_Window.SetVerticalSyncEnabled(false);

	GetKeyHandler()->SetKeyStateProvider(&m_Window);
	GetMouseHandler()->SetMouseStateProvider(&m_Window);
	RegisterEvents();

	m_Window.UnmakeGLCurrent();
}

void SimpleOpenGLTest::DestroyMain()
{
	m_Window.MakeGLCurrent();
}

void SimpleOpenGLTest::InitializeRendering()
{
	m_Window.MakeGLCurrent();

	LoadPipeline();
	LoadAssets();

	UpdateCameras();
	UpdateSceneNodes();
	UpdateRenderableSceneNodeData();
}

void SimpleOpenGLTest::DestroyRendering()
{
	m_Window.UnmakeGLCurrent();
}

int SimpleOpenGLTest::EnterMainEventLoop()
{
	return m_Window.EnterMainEventLoop();
}

void SimpleOpenGLTest::RegisterEvents()
{
	m_ExitECI = m_KeyHandler.RegisterStateKeyEventListener("Exit", this);
	m_ScreenshotECI = m_KeyHandler.RegisterStateKeyEventListener("Screenshot", this);

	DefaultInputBinder::Bind(this, &m_KeyHandler, &m_MouseHandler, &m_Camera);
	m_KeyHandler.BindEventToKey(m_ScreenshotECI, Keys::P);
}

unsigned SimpleOpenGLTest::GetExitECI() const
{
	return m_ExitECI;
}

bool SimpleOpenGLTest::HandleEvent(const Event* _event)
{
	if (_event->ClassId == m_ExitECI)
	{
		RequestExiting();
		return true;
	}
	else if (_event->ClassId == m_ScreenshotECI)
	{
		m_OGLU.ScreenshotHandler.MakeScreenshotFromFBO(0, GL_BACK, m_Width, m_Height, PixelDataFormat::BGRA);
		return true;
	}

	return false;
}

void SimpleOpenGLTest::UpdateCameras()
{
	m_Camera.Update(m_SystemTime);
}

void SimpleOpenGLTest::UpdateSceneNodes()
{
	m_SceneNodeHandler.UpdateTransformations();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Load the rendering pipeline dependencies.
void SimpleOpenGLTest::LoadPipeline()
{
	InitializeGLEW();
	InitializeGLDebugging();
}

Texture2D* SimpleOpenGLTest::GetTexture(const std::string& textureName)
{
	return m_OGLM.ConstantTextureManager.GetTexture2DFromFile(textureName).Texture;
}

Texture2D* SimpleOpenGLTest::GetTexture(const std::string& diffuseTextureName,
	const std::string& opacityTextureName)
{
	return m_OGLM.ConstantTextureManager.GetDiffuseTexture2DWithOpacity(
		diffuseTextureName, opacityTextureName, c_SceneData[m_SceneIndex].OpacityChannelIndex).Texture;
}

void SimpleOpenGLTest::SetPipelineState(const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes,
	const MaterialData& materialData, OGLT_Material* pMaterial)
{
	auto& inputLayout = m_ModelLoader.GetModel(loadRes.ModelIndex).Vertices.InputLayout;

	std::vector<ShaderDefine> shaderDefines = {
		{ "IS_USING_MATERIAL_COLORS", m_IsUsingMaterialColors },
		{ "IS_USING_VERTEX_COLOR", inputLayout.HasVertexColors() }
	};

	ShaderProgramDescription spd;
	spd.Shaders = {
		ShaderDescription::FromFile(m_PathHandler, "GLSL/Test/ShaderTest_vs.glsl", ShaderType::Vertex, "", shaderDefines),
		ShaderDescription::FromFile(m_PathHandler, "GLSL/Test/ShaderTest_ps.glsl", ShaderType::Fragment, "", shaderDefines)
	};

	pMaterial->ShaderProgram = m_OGLM.ShaderManager.GetShaderProgram(spd);

	pMaterial->PS_Flags = OGLT_PS_Flags::CullingEnabled;

	if (materialData.Opacity == 1.0f)
	{
		// Opaque material.
		if (!materialData.OpacityTextureName.empty())
		{
			pMaterial->PS_Flags &= ~OGLT_PS_Flags::CullingEnabled;
			pMaterial->PS_Flags |= OGLT_PS_Flags::AlphaToCoverageEnabled;
		}
	}
	else
	{
		// Transparent material.
		assert(false);
	}
}

void SimpleOpenGLTest::SetResourceState(const MaterialData& materialData, OGLT_Material* pMaterial)
{
	pMaterial->UniformBuffers.push_back(&m_RenderPassCB);
	pMaterial->UniformBuffers.push_back(&m_ObjectCB);
	pMaterial->UniformBuffers.push_back(&m_MaterialCB);
	pMaterial->UniformBuffers.push_back(&m_LightingCB);

	if (materialData.OpacityTextureName.empty())
	{
		pMaterial->DiffuseTexture = GetTexture(materialData.DiffuseTextureName);
	}
	else
	{
		pMaterial->DiffuseTexture = GetTexture(materialData.DiffuseTextureName, materialData.OpacityTextureName);
	}
}

unsigned SimpleOpenGLTest::GetMaterialIndex(unsigned objectIndex,
	const ModelLoadingResult& loadRes, const ModelInstantiationResult& instRes, bool& isOpaque)
{
	auto& model = m_ModelLoader.GetModel(loadRes.ModelIndex);
	unsigned objectMaterialIndex = model.Objects[objectIndex].MaterialIndex;
	auto& objectMaterial = model.Materials[objectMaterialIndex];

	isOpaque = (objectMaterial.Opacity == 1.0f);

	unsigned materialCBIndex = Core::c_InvalidIndexU;
	{
		MaterialCBType cbData;
		cbData.DiffuseColor = objectMaterial.DiffuseColor;
		cbData.SpecularColor = objectMaterial.SpecularColor;

		// Note: this is slow for many materials.
		for (unsigned i = 0; i < m_MaterialCBData.GetSize(); i++)
		{
			auto& otherCBData = m_MaterialCBData[i];
			if (cbData.DiffuseColor == otherCBData.DiffuseColor && cbData.SpecularColor == otherCBData.SpecularColor)
			{
				materialCBIndex = i;
				break;
			}
		}
		if (materialCBIndex == Core::c_InvalidIndexU)
		{
			materialCBIndex = m_MaterialCBData.GetSize();
			m_MaterialCBData.PushBack(cbData);
		}
	}

	OGLT_Material material;
	SetPipelineState(loadRes, objectMaterial, &material);
	SetResourceState(objectMaterial, &material);
	material.MaterialCBIndex = materialCBIndex;

	auto mIt = m_MaterialMap.find(material);
	if (mIt == m_MaterialMap.end())
	{
		mIt = m_MaterialMap.insert({ material, m_Materials.Add(material) }).first;
	}
	return mIt->second;
}

unsigned SimpleOpenGLTest::AddRenderTask(unsigned objectIndex,
	const ModelLoadingResult& loadRes, const ModelInstantiationResult& instRes,
	const Vertex_SOA_Data& vertexData)
{
	auto& model = m_ModelLoader.GetModel(loadRes.ModelIndex);
	auto& objectData = model.Objects[objectIndex];
	auto& nodeMapping = instRes.GlobalSceneNodeMapping;
	bool isOpaque;

	// Creating a scene node for the render task.
	OGLT_RenderTask renderTask;
	renderTask.SceneNodeIndex = nodeMapping[objectData.SceneNodeIndex];
	renderTask.MaterialIndex = GetMaterialIndex(objectIndex, loadRes, instRes, isOpaque);
	renderTask.ObjectCBIndex = m_ObjectCBData.GetSize();

	m_ObjectCBData.PushBackPlaceHolder();

	auto& geometryData = loadRes.Meshes[objectData.MeshIndex];
	renderTask.Primitive.PVertexArrayObject = &m_VertexArrayObject;
	renderTask.Primitive.PVertexBuffer = &m_VertexBuffer;
	renderTask.Primitive.PIndexBuffer = &m_IndexBuffer;
	renderTask.Primitive.Topology = model.Indices.Topology;
	renderTask.Primitive.BaseVertex = geometryData.BaseVertex;
	renderTask.Primitive.BaseIndex = geometryData.BaseIndex;
	renderTask.Primitive.CountVertices = geometryData.CountVertices;
	renderTask.Primitive.CountIndices = geometryData.CountIndices;
	
	renderTask.BoundingBox = AABoundingBox::GetBoundingBox(vertexData.GetPositions() + geometryData.BaseVertex,
		geometryData.CountVertices);

	unsigned rtgIndex = (isOpaque ? 0 : 1);

	return m_RenderTaskGroups[rtgIndex].AddRenderTask(renderTask);
}

// Load the sample assets.
void SimpleOpenGLTest::LoadAssets()
{
	m_RenderPassCB.Initialize(BufferUsage::DynamicDraw, sizeof(RenderPassCBType));
	m_ObjectCB.Initialize(BufferUsage::DynamicDraw, sizeof(ObjectCBType));
	m_MaterialCB.Initialize(BufferUsage::DynamicDraw, sizeof(MaterialCBType));
	m_LightingCB.Initialize(BufferUsage::DynamicDraw, sizeof(Lighting_Dir_Spot_CBType1<c_MaxCountLights>));

	m_RenderTaskGroups.resize(2); // Opaque and transparent.

	// Loading model.
	{
		auto& sceneData = c_SceneData[m_SceneIndex];
		Vertex_SOA_Data vertexData;
		IndexData indexData;
		ModelLoadingDescription modelDesc;
		modelDesc.BuildingDescription.FilePath = sceneData.Name;
		modelDesc.BuildingDescription.GeometryOptions.IsOptimizingMeshes = c_IsOptimizingMeshes;
		auto modelLoadingResult = m_ModelLoader.Load(modelDesc, vertexData, indexData);
		auto modelInstantiationResult = m_ModelLoader.Instatiate(modelLoadingResult.ModelIndex, m_SceneNodeHandler);
		auto model = m_ModelLoader.GetModel(modelLoadingResult.ModelIndex);
		auto countObjects = modelLoadingResult.Meshes.GetSize();
		auto countMaterials = static_cast<unsigned>(model.Materials.size());

		m_VertexArrayObject.Initialize();
		m_VertexArrayObject.Bind();
		m_VertexBuffer.Initialize(BufferUsage::StaticDraw, model.Vertices);
		m_IndexBuffer.Initialize(BufferUsage::StaticDraw, model.Indices);
		m_InputLayout = model.Vertices.InputLayout;

		// Adding objects.
		for (unsigned i = 0; i < countObjects; i++)
		{
			AddRenderTask(i, modelLoadingResult, modelInstantiationResult, vertexData);
		}

		// Setting position, orientation, scaler.
		auto orientationYPR = glm::radians(sceneData.OrientationYPR);
		auto orientation = glm::mat3(glm::yawPitchRoll(orientationYPR[0], orientationYPR[1], orientationYPR[2]));
		m_SceneNodeHandler.SetPosition(modelInstantiationResult.GlobalRootSceneNodeIndex, sceneData.Position);
		m_SceneNodeHandler.SetOrientation(modelInstantiationResult.GlobalRootSceneNodeIndex, orientation);
		m_SceneNodeHandler.SetLocalScaler(modelInstantiationResult.GlobalRootSceneNodeIndex, glm::vec3(sceneData.Scaler));
	}

	auto initContext = UtilityInitializationContext{ &m_PathHandler, &m_OGLM, m_MultisampleCount };
	m_OGLU.Initialize(initContext);
	m_OGLU.BackgroundCMRenderer.AddTexture(initContext, "Cubemaps/cloudyNoon.dds");

	// Initializing camera.
	{
		glm::vec3 cameraPosition(0.0f, 2.0f, 0.0f);
		glm::vec3 cameraDirection(0.0f, 0.0f, -1.0);
		float fovY = glm::radians(60.0f);
		float nearPlaneDistance = 0.1f;
		float farPlaneDistance = 1000.0f;

		float aspectRatio = m_Window.GetAspectRatio();

		m_Camera.SetLocation(cameraPosition, cameraDirection);
		m_Camera.SetPerspectiveProjection(fovY, aspectRatio, nearPlaneDistance, farPlaneDistance, true);
	}

	SetLightingCBData();
}

void SimpleOpenGLTest::SetLightingCBData()
{
	Lighting_Dir_Spot_CBType1<c_MaxCountLights> lightingCBData;
	CreateDefaultLighting1(lightingCBData);
	m_LightingCB.Write(&lightingCBData, sizeof(Lighting_Dir_Spot_CBType1<c_MaxCountLights>));
}

void SimpleOpenGLTest::UpdateRenderPassCBData()
{
	RenderPassCBType data;
	UpdateRenderPassCB(&m_Camera, &data);
	m_RenderPassCB.Write(&data, sizeof(RenderPassCBType));
}

void SimpleOpenGLTest::UpdateObjectCBData()
{
	for (size_t i = 0; i < m_RenderTaskGroups.size(); i++)
	{
		m_RenderTaskGroups[i].UpdateObjectCBData(m_ObjectCBData, m_Camera, m_SceneNodeHandler);
	}
}

void SimpleOpenGLTest::UpdateRenderableSceneNodeData()
{
	for (size_t i = 0; i < m_RenderTaskGroups.size(); i++)
	{
		m_RenderTaskGroups[i].Update(m_ViewFrustumCuller, m_TaskSorter, m_Camera, m_ThreadPool, m_SceneNodeHandler,
			(i == 0 ? TaskSortType::MaximumOcclusion : TaskSortType::BackToFront));
	}
}

void SimpleOpenGLTest::DerivedPreUpdate()
{
	UpdateRenderPassCBData();
	UpdateObjectCBData();
	
	m_OGLU.PreUpdate({});
}

// Update frame-based values.
void SimpleOpenGLTest::DerivedPostUpdate(EngineBuildingBlocks::PostUpdateContext& context)
{
	UpdateCameras();
	UpdateSceneNodes();
	UpdateRenderableSceneNodeData();

	m_OGLU.PostUpdate({});

	if (context.IsFPSRefreshed)
	{
		auto fps = m_FPSController.GetFPS();

		if (m_IsWindowed)
		{
			std::stringstream ssTitle;
			ssTitle << m_Title << " :: FPS: " << fps << " :: Draw calls: " << m_CountDrawCalls;
			m_Window.SetTitle(ssTitle.str());
		}
		std::stringstream ssTRT;
		ssTRT << "FPS: " << fps << "\nDraw calls : " << m_CountDrawCalls;
		m_Statistics_TRT.SetText(ssTRT.str().c_str());
		m_Statistics_TRT.Update(m_OGLU.TextRenderer);
	}
}

// Render the scene.
void SimpleOpenGLTest::DerivedRender(RenderContext& context)
{
	m_CountDrawCalls = 0;

	glViewport(0, 0, m_Width, m_Height);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, m_IsRenderingWireframe ? GL_LINE : GL_FILL);
	if (m_MultisampleCount > 1) glEnable(GL_MULTISAMPLE);
	else glDisable(GL_MULTISAMPLE);

	RenderObjects();
	RenderBackgroundCubemap();
	RenderText();
}

void SimpleOpenGLTest::DerivedPresent()
{
	m_Window.Present();
}

inline void SetPipelineState(OGLT_PS_Flags flags)
{
	if(HasFlag(flags, OGLT_PS_Flags::CullingEnabled)) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	if (HasFlag(flags, OGLT_PS_Flags::AlphaToCoverageEnabled)) glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	else glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
}

inline void ResetPipelineState()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
}

inline void SetShaderProgram(OGLT_Material* material, VertexArrayObject* vertexArrayObject,
	VertexBuffer* vertexBuffer, const VertexInputLayout& inputLayout)
{
	material->ShaderProgram->Bind();

	auto countUBs = static_cast<unsigned>(material->UniformBuffers.size());
	for (unsigned i = 0; i < countUBs; i++)
	{
		material->UniformBuffers[i]->Bind(i);
	}

	material->ShaderProgram->SetUniformValue("DiffuseTexture", 0);
	
	vertexArrayObject->Bind();
	vertexBuffer->Bind();
	material->ShaderProgram->SetInputLayout(inputLayout);
}

void SimpleOpenGLTest::RenderObjects()
{
	unsigned prevMaterialIndex = Core::c_InvalidIndexU;
	ShaderProgram* currProgram = nullptr;
	auto currPSFlags = OGLT_PS_Flags::Invalid;
	OpenGLRender::Texture2D* currDiffTex = nullptr;
	unsigned prevRSIndex = Core::c_InvalidIndexU;
	unsigned currMCBIndex = Core::c_InvalidIndexU;

	for (size_t i = 0; i < m_RenderTaskGroups.size(); i++)
	{
		auto& processedRTIndices = m_RenderTaskGroups[i].ProcessedRTIndices;
		auto& renderTasks = m_RenderTaskGroups[i].RenderTasks;

		unsigned countRenderTasks = processedRTIndices.GetSize();
		for (unsigned j = 0; j < countRenderTasks; j++)
		{
			auto& renderTask = renderTasks[processedRTIndices[j]];
			auto materialIndex = renderTask.MaterialIndex;

			// This is material level, but we don't sort by material now.
			if (materialIndex != prevMaterialIndex)
			{
				auto& material = m_Materials[materialIndex];
				auto program = material.ShaderProgram;
				auto psFlags = material.PS_Flags;
				auto diffTex = material.DiffuseTexture;
				auto mcbIndex = material.MaterialCBIndex;

				if (currProgram != program)
				{
					currProgram = program;
					SetShaderProgram(&material, renderTask.Primitive.PVertexArrayObject,
						renderTask.Primitive.PVertexBuffer, m_InputLayout);
				}
				if (currPSFlags != psFlags)
				{
					currPSFlags = psFlags;
					::SetPipelineState(psFlags);
				}
				if (currDiffTex != diffTex)
				{
					currDiffTex = diffTex;
					currDiffTex->Bind(0);
				}
				if (currMCBIndex != mcbIndex)
				{
					m_MaterialCB.Write(&m_MaterialCBData[mcbIndex], sizeof(MaterialCBType));
					currMCBIndex = mcbIndex;
				}

				prevMaterialIndex = materialIndex;
			}

			m_ObjectCB.Write(&m_ObjectCBData[renderTask.ObjectCBIndex], sizeof(ObjectCBType));

			DrawPrimitive(renderTask.Primitive);
		}

		m_CountDrawCalls += countRenderTasks;
	}

	ResetPipelineState();
}

void SimpleOpenGLTest::RenderBackgroundCubemap()
{
	m_OGLU.BackgroundCMRenderer.Render({}, &m_RenderPassCB, 0);
	m_CountDrawCalls++;
}

void SimpleOpenGLTest::RenderText()
{
	m_OGLU.TextRenderer.Render({ &m_OGLM });
}

void SimpleOpenGLTest::LoadConfiguration()
{
	m_Configuration.LoadFromXml(
		m_PathHandler.GetPathFromResourcesDirectory("Configurations/SimpleOGLApp_Configuration.xml").c_str());
	
	m_Configuration.TryGetRootConfigurationValue("Title", m_Title);
	m_Configuration.TryGetRootConfigurationValue("IsWindowed", m_IsWindowed);
	if (m_IsWindowed)
	{
		m_Configuration.TryGetRootConfigurationValue("WindowedWidth", m_Width);
		m_Configuration.TryGetRootConfigurationValue("WindowedHeight", m_Height);
	}
	else
	{
		auto fullscreenRes = m_Window.GetPrimaryMonitorResolution();
		m_Width = fullscreenRes.x;
		m_Height = fullscreenRes.y;
	}
	m_Configuration.TryGetRootConfigurationValue("MultisampleCount", m_MultisampleCount);
	m_Configuration.TryGetRootConfigurationValue("IsRenderingWireframe", m_IsRenderingWireframe);
	m_Configuration.TryGetRootConfigurationValue("IsUsingMaterialColors", m_IsUsingMaterialColors);
	m_Configuration.TryGetRootConfigurationValue("SceneIndex", m_SceneIndex);
}