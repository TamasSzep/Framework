// FrameworkTest/SimpleDirectX11Test.cpp

#include <FrameworkTest/SimpleDirectX11Test.h>

#include <EngineBuildingBlocks/Input/DefaultInputBinder.h>
#include <EngineBuildingBlocks/Graphics/Lighting/Lighting1.h>
#include <DirectX11Render/GraphicsDevice.h>
#include <DirectX11Render/Utilities/RenderPassCB.h>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace EngineBuildingBlocks::Input;
using namespace EngineBuildingBlocks::Math;
using namespace WindowsApplication;
using namespace DirectXRender;
using namespace DirectX11Render;
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
const unsigned c_MaxCountObjects = 1024 * 64;
const unsigned c_MaxCountMaterials = 1024;
const unsigned c_MaxCountTextures = 1024;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DX11T_Material::operator==(const DX11T_Material& other) const
{
	NumericalEqualCompareBlock(PipelineStateIndex);
	NumericalEqualCompareBlock(ResourceStateIndex);
	NumericalEqualCompareBlock(MaterialCBIndex);
	return true;
}

bool DX11T_Material::operator!=(const DX11T_Material& other) const
{
	return !(*this == other);
}

bool DX11T_Material::operator<(const DX11T_Material& other) const
{
	NumericalLessCompareBlock(PipelineStateIndex);
	NumericalLessCompareBlock(ResourceStateIndex);
	NumericalLessCompareBlock(MaterialCBIndex);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SimpleDirectX11Test::OnEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handling other events.

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SimpleDirectX11Test::DerivedParseCommandLineArgs(int argc, char *argv[])
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTaskGroup::RenderTaskGroup()
	: AreSceneNodeVectorsUpToDate(true)
{
}

unsigned RenderTaskGroup::AddRenderTask(const DX11T_RenderTask& renderTask)
{
	AreSceneNodeVectorsUpToDate = false;
	return RenderTasks.Add(renderTask);
}

void RenderTaskGroup::Update(ViewFrustumCuller& culler, TaskSorter& taskSorter, Camera& camera,
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

void RenderTaskGroup::UpdateObjectCBData(ConstantBuffer& objectCB, Camera& camera, SceneNodeHandler& sceneNodeHandler)
{
	auto& viewProjectionMatrix = camera.GetViewProjectionMatrix();

	glm::mat4 worldMatrix;

	auto it = RenderTasks.GetBeginIterator();
	auto end = RenderTasks.GetEndIterator();
	for (; it != end; ++it)
	{
		auto& renderTask = *it;
		unsigned sceneNodeIndex = renderTask.SceneNodeIndex;
		auto& objectCBData = objectCB.Access<ObjectCBType>(it->ObjectCBIndex);

		worldMatrix = sceneNodeHandler.UnsafeGetScaledWorldTransformation(sceneNodeIndex).AsMatrix4();
		objectCBData.ModelMatrix = worldMatrix;
		objectCBData.InverseModelMatrix = sceneNodeHandler.UnsafeGetInverseScaledWorldTransformation(sceneNodeIndex).AsMatrix4();
		objectCBData.ModelViewProjectionMatrix = viewProjectionMatrix * worldMatrix;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

SimpleDirectX11Test::SimpleDirectX11Test(int argc, char *argv[])
	: WindowsApplication::Application<SimpleDirectX11Test>(argc, argv)
	, m_DX11M(m_PathHandler)
	, m_Camera(&m_SceneNodeHandler, &m_KeyHandler, &m_MouseHandler)
	, m_CountDrawCalls(0)
	, m_CountMaterialCBData(0)
	, m_CountObjectCBData(0)
{
	LoadConfiguration();

	m_Window.SetTitle(m_Title);

	m_Statistics_TRT.SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	m_Statistics_TRT.SetPosition(TextPositionConstant::TopLeft);
}

SimpleDirectX11Test::~SimpleDirectX11Test()
{
}

void SimpleDirectX11Test::InitializeMain()
{
	m_Window.Initialize(m_Width, m_Height);

	GetKeyHandler()->SetKeyStateProvider(&m_Window);
	GetMouseHandler()->SetMouseStateProvider(&m_Window);

	RegisterEvents();
}

void SimpleDirectX11Test::DestroyMain()
{
}

void SimpleDirectX11Test::InitializeRendering()
{
	LoadPipeline();
	LoadAssets();

	UpdateCameras();
	UpdateSceneNodes();
	UpdateRenderableSceneNodeData();
}

void SimpleDirectX11Test::RegisterEvents()
{
	m_ExitECI = m_KeyHandler.RegisterStateKeyEventListener("Exit", this);
	m_ScreenshotECI = m_KeyHandler.RegisterStateKeyEventListener("Screenshot", this);

	DefaultInputBinder::Bind(this, &m_KeyHandler, &m_MouseHandler, &m_Camera);
	m_KeyHandler.BindEventToKey(m_ScreenshotECI, Keys::P);
}

unsigned SimpleDirectX11Test::GetExitECI() const
{
	return m_ExitECI;
}

bool SimpleDirectX11Test::HandleEvent(const Event* _event)
{
	if (_event->ClassId == m_ExitECI)
	{
		RequestExiting();
		return true;
	}
	else if (_event->ClassId == m_ScreenshotECI)
	{
		m_DX11U.ScreenshotHandler.MakeScreenshot(m_Device.Get(), m_DeviceContext.Get(), m_ColorBackBuffer.Get(),
			m_Width, m_Height, c_DefaultColorBackBufferFormat);
		return true;
	}

	return false;
}

void SimpleDirectX11Test::UpdateCameras()
{
	m_Camera.Update(m_SystemTime);
}

void SimpleDirectX11Test::UpdateSceneNodes()
{
	m_SceneNodeHandler.UpdateTransformations();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Load the rendering pipeline dependencies.
void SimpleDirectX11Test::LoadPipeline()
{
#ifdef _DEBUG
	bool isCreatingDegviceDebug = true;
#else
	bool isCreatingDegviceDebug = false;
#endif

	ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory(factory, m_Window.GetHandle());
	CreateGraphicsDevice(GraphicsDeviceType::Hardware, D3D_FEATURE_LEVEL_11_0, factory, m_Device, m_DeviceContext, isCreatingDegviceDebug);
	CreateSwapChain(factory, m_Device, m_SwapChain, m_ColorBackBuffer, m_ColorBackBufferRTV, m_Window.GetHandle(), m_Width, m_Height, m_IsWindowed,
		m_BackbufferFrameCount, c_DefaultColorBackBufferFormat, true);

	// Creating multisampled color and depth buffers.
	DirectX11Render::Texture2DDescription colorBufferDesc(m_Width, m_Height, c_DefaultColorBackBufferFormat, 1,
		m_MultisampleCount, D3D11_USAGE_DEFAULT, TextureBindFlag::RenderTarget);
	m_ColorBuffer.Initialize(m_Device.Get(), colorBufferDesc, nullptr, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
		c_DefaultColorBackBufferFormat);

	DirectX11Render::Texture2DDescription depthBufferDesc(m_Width, m_Height, DXGI_FORMAT_R32_TYPELESS, 1,
		m_MultisampleCount, D3D11_USAGE_DEFAULT, TextureBindFlag::DepthStencil);
	m_DepthBuffer.Initialize(m_Device.Get(), depthBufferDesc, nullptr, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D32_FLOAT);
}

Texture2D* SimpleDirectX11Test::GetTexture(const std::string& textureName)
{
	return m_DX11M.ConstantTextureManager.GetTexture2DFromFile(m_Device.Get(), textureName).Texture;
}

Texture2D* SimpleDirectX11Test::GetTexture(const std::string& diffuseTextureName,
	const std::string& opacityTextureName)
{
	return m_DX11M.ConstantTextureManager.GetDiffuseTexture2DWithOpacity(m_Device.Get(),
		diffuseTextureName, opacityTextureName, c_SceneData[m_SceneIndex].OpacityChannelIndex).Texture;
}

unsigned SimpleDirectX11Test::GetPipelineStateIndex(const EngineBuildingBlocks::Graphics::ModelLoadingResult& loadRes, 
	const MaterialData& materialData)
{
	auto inputLayout = DirectX11Render::VertexInputLayout::Create(
		m_ModelLoader.GetModel(loadRes.ModelIndex).Vertices.InputLayout);

	std::vector<ShaderDefine> shaderDefines = {
		{ "IS_USING_MATERIAL_COLORS", m_IsUsingMaterialColors },
		{ "IS_USING_VERTEX_COLOR", inputLayout.InputLayout.HasVertexColors() }
	};
	auto vs = m_DX11M.ShaderManager.GetShader(m_Device.Get(), { "Test/ShaderTest.hlsl",
		"VSMain", ShaderType::Vertex, "5_0", 0, shaderDefines });
	auto ps = m_DX11M.ShaderManager.GetShader(m_Device.Get(), { "Test/ShaderTest.hlsl",
		"PSMain", ShaderType::Pixel, "5_0", 0, shaderDefines });

	SamplerStateDescription diffuseTextureSampler;
	diffuseTextureSampler.SetToWrap();
	diffuseTextureSampler.SetToAnisotropic();

	PipelineStateDescription description;
	description.Shaders = { vs, ps };
	description.InputLayout = std::move(inputLayout);
	description.SamplerStates = { { diffuseTextureSampler, ShaderFlagType::Pixel } };

	if (materialData.Opacity == 1.0f)
	{
		// Opaque material.
		if (!materialData.OpacityTextureName.empty())
		{
			description.RasterizerState.DisableCulling();
			description.BlendState.EnableAlphaToCoverage();
		}
	}
	else
	{
		// Transparent material.
	}

	if (m_MultisampleCount > 1) description.RasterizerState.EnableMultisampling();
	if (m_IsRenderingWireframe) description.RasterizerState.SetWireframeFillMode();

	return m_DX11M.PipelineStateManager.GetPipelineStateIndex(m_Device.Get(), description);
}

unsigned SimpleDirectX11Test::GetResourceStateIndex(const MaterialData& materialData)
{
	ResourceState resourceState(ShaderFlagType::Vertex | ShaderFlagType::Pixel, 4, 1);
	resourceState.AddConstantBuffer(m_RenderPassCB.GetBuffer(), 0, ShaderFlagType::Pixel);
	resourceState.AddConstantBuffer(m_ObjectCB.GetBuffer(), 1, ShaderFlagType::Vertex);
	resourceState.AddConstantBuffer(m_MaterialCB.GetBuffer(), 2, ShaderFlagType::Pixel);
	resourceState.AddConstantBuffer(m_LightingCB.GetBuffer(), 3, ShaderFlagType::Pixel);

	if (materialData.OpacityTextureName.empty())
	{
		resourceState.AddShaderResourceView(
			GetTexture(materialData.DiffuseTextureName)->GetSRV(),
			0, ShaderFlagType::Pixel);
	}
	else
	{
		resourceState.AddShaderResourceView(
			GetTexture(materialData.DiffuseTextureName, materialData.OpacityTextureName)->GetSRV(),
			0, ShaderFlagType::Pixel);
	}

	return m_DX11M.ResourceStateManager.AddResourceState(resourceState);
}

unsigned SimpleDirectX11Test::GetMaterialIndex(unsigned objectIndex,
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
		for (unsigned i = 0; i < m_CountMaterialCBData; i++)
		{
			auto& otherCBData = m_MaterialCB.Access<MaterialCBType>(i);
			if (cbData.DiffuseColor == otherCBData.DiffuseColor && cbData.SpecularColor == otherCBData.SpecularColor)
			{
				materialCBIndex = i;
				break;
			}
		}
		if (materialCBIndex == Core::c_InvalidIndexU)
		{
			materialCBIndex = m_CountMaterialCBData++;
			memcpy(&m_MaterialCB.Access<MaterialCBType>(materialCBIndex), &cbData, sizeof(MaterialCBType));
		}
	}

	DX11T_Material material;
	material.PipelineStateIndex = GetPipelineStateIndex(loadRes, objectMaterial);
	material.ResourceStateIndex = GetResourceStateIndex(objectMaterial);
	material.MaterialCBIndex = materialCBIndex;

	unsigned materialIndex;
	if (!m_Materials.IsContaining(material, materialIndex))
	{
		materialIndex = m_Materials.Add(material);
	}

	return materialIndex;
}

unsigned SimpleDirectX11Test::AddRenderTask(unsigned objectIndex,
	const ModelLoadingResult& loadRes, const ModelInstantiationResult& instRes,
	const Vertex_SOA_Data& vertexData)
{
	auto& model = m_ModelLoader.GetModel(loadRes.ModelIndex);
	auto& objectData = model.Objects[objectIndex];
	auto& nodeMapping = instRes.GlobalSceneNodeMapping;
	bool isOpaque;

	// Creating a scene node for the render task.
	DX11T_RenderTask renderTask;
	renderTask.SceneNodeIndex = nodeMapping[objectData.SceneNodeIndex];
	renderTask.MaterialIndex = GetMaterialIndex(objectIndex, loadRes, instRes, isOpaque);
	renderTask.ObjectCBIndex = m_CountObjectCBData++;

	auto& geometryData = loadRes.Meshes[objectData.MeshIndex];
	renderTask.Primitive.PVertexBuffer = &m_VertexBuffer;
	renderTask.Primitive.PIndexBuffer = &m_IndexBuffer;
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
void SimpleDirectX11Test::LoadAssets()
{
	m_RenderPassCB = CreateRenderPassCB(m_Device.Get());
	m_LightingCB.Initialize(m_Device.Get(), sizeof(Lighting_Dir_Spot_CBType1<c_MaxCountLights>), 1);
	m_ObjectCB.Initialize(m_Device.Get(), sizeof(ObjectCBType), c_MaxCountObjects);
	m_MaterialCB.Initialize(m_Device.Get(), sizeof(MaterialCBType), c_MaxCountMaterials);

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

		m_VertexBuffer.Initialize(m_Device.Get(), model.Vertices);
		m_IndexBuffer.Initialize(m_Device.Get(), model.Indices);

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

	auto initContext = UtilityInitializationContext{ m_Device.Get(), m_DeviceContext.Get(),
		&m_PathHandler, &m_DX11M, m_MultisampleCount };
	m_DX11U.Initialize(initContext);
	m_DX11U.BackgroundCMRenderer.AddTexture(initContext, "Cubemaps/cloudyNoon.dds");

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

void SimpleDirectX11Test::SetLightingCBData()
{
	auto& cb = m_LightingCB;
	auto& lightingCBData = cb.Access<Lighting_Dir_Spot_CBType1<c_MaxCountLights>>(0);
	CreateDefaultLighting1(lightingCBData);
	m_LightingCB.Update(m_DeviceContext.Get());
}

void SimpleDirectX11Test::UpdateRenderPassCBData()
{
	UpdateRenderPassCB(&m_Camera, &m_RenderPassCB);
}

void SimpleDirectX11Test::UpdateObjectCBData()
{
	for (size_t i = 0; i < m_RenderTaskGroups.size(); i++)
	{
		m_RenderTaskGroups[i].UpdateObjectCBData(m_ObjectCB, m_Camera, m_SceneNodeHandler);
	}
}

void SimpleDirectX11Test::UpdateRenderableSceneNodeData()
{
	for (size_t i = 0; i < m_RenderTaskGroups.size(); i++)
	{
		m_RenderTaskGroups[i].Update(m_ViewFrustumCuller, m_TaskSorter, m_Camera, m_ThreadPool, m_SceneNodeHandler,
			(i == 0 ? TaskSortType::MaximumOcclusion : TaskSortType::BackToFront));
	}
}

void SimpleDirectX11Test::DerivedPreUpdate()
{
	UpdateRenderPassCBData();
	UpdateObjectCBData();
	
	m_DX11U.PreUpdate({ m_DeviceContext.Get() });
}

// Update frame-based values.
void SimpleDirectX11Test::DerivedPostUpdate(EngineBuildingBlocks::PostUpdateContext& context)
{
	UpdateCameras();
	UpdateSceneNodes();
	UpdateRenderableSceneNodeData();

	m_DX11U.PostUpdate({ m_DeviceContext.Get() });

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
		m_Statistics_TRT.Update(m_DX11U.TextRenderer);
	}
}

// Render the scene.
void SimpleDirectX11Test::DerivedRender(RenderContext& context)
{
	m_CountDrawCalls = 0;

	// Setting render target, depth buffer, viewport, clearing render target and depth buffers.
	ID3D11RenderTargetView* rtvs[]{ m_ColorBuffer.GetRTV() };
	m_DeviceContext->OMSetRenderTargets(1, rtvs, m_DepthBuffer.GetDSV());
	D3D11_VIEWPORT viewPort = { 0.0f, 0.0f, static_cast<float>(m_Width), static_cast<float>(m_Height), 0.0f, 1.0f };
	m_DeviceContext->RSSetViewports(1, &viewPort);
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if(c_IsClearingRTV) m_DeviceContext->ClearRenderTargetView(rtvs[0], clearColor);
	m_DeviceContext->ClearDepthStencilView(m_DepthBuffer.GetDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_RenderPassCB.Update(m_DeviceContext.Get());

	RenderObjects();
	RenderBackgroundCubemap();
	RenderText();

	// Resolving subresource.
	m_DeviceContext->ResolveSubresource(m_ColorBackBuffer.Get(), 0, m_ColorBuffer.GetResource(), 0,
		c_DefaultColorBackBufferFormat);
}

void SimpleDirectX11Test::DerivedPresent()
{
	auto hr = m_SwapChain->Present(m_SyncInterval, 0);
	if (FAILED(hr))
	{
		auto dhr = m_Device->GetDeviceRemovedReason();
		std::stringstream ss;
		ss << "Failed to present:\n\nError:" << Core::ToString(_com_error(hr).ErrorMessage()) << "\n"
			<< "Device remove reason: " << Core::ToString(_com_error(dhr).ErrorMessage());
		EngineBuildingBlocks::RaiseException(ss);
	}
}

void SimpleDirectX11Test::DestroyRendering()
{
	if(!m_IsWindowed) m_SwapChain->SetFullscreenState(FALSE, nullptr);
}

void SimpleDirectX11Test::RenderObjects()
{
	auto d3dContext = m_DeviceContext.Get();

	unsigned prevMaterialIndex = Core::c_InvalidIndexU;
	unsigned prevPSIndex = Core::c_InvalidIndexU;
	unsigned prevRSIndex = Core::c_InvalidIndexU;
	unsigned prevMCBIndex = Core::c_InvalidIndexU;

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
				auto psIndex = material.PipelineStateIndex;
				auto rsIndex = material.ResourceStateIndex;
				auto mcbIndex = material.MaterialCBIndex;

				if (prevPSIndex != psIndex)
				{
					m_DX11M.PipelineStateManager.GetPipelineState(psIndex).SetForContext(d3dContext);
					prevPSIndex = psIndex;
				}
				if (prevRSIndex != rsIndex)
				{
					m_DX11M.ResourceStateManager.GetResourceState(rsIndex).SetForContext(d3dContext);
					prevRSIndex = rsIndex;
				}
				if (prevMCBIndex != mcbIndex)
				{
					m_MaterialCB.Update(d3dContext, mcbIndex);
					prevMCBIndex = mcbIndex;
				}

				prevMaterialIndex = materialIndex;
			}

			m_ObjectCB.Update(d3dContext, renderTask.ObjectCBIndex);

			DrawPrimitive(renderTask.Primitive, d3dContext);
		}

		m_CountDrawCalls += countRenderTasks;
	}
}

void SimpleDirectX11Test::RenderBackgroundCubemap()
{
	m_DX11U.BackgroundCMRenderer.Render({ m_DeviceContext.Get(), &m_DX11M }, m_RenderPassCB, 0, 0);
	m_CountDrawCalls++;
}

void SimpleDirectX11Test::RenderText()
{
	m_DX11U.TextRenderer.Render({ m_DeviceContext.Get(), &m_DX11M });
}

template <typename T>
inline void InitializeRootProperty(const Core::Properties& configuration, const char* name, T& property)
{
	configuration.TryGetPropertyValue((std::string("Configuration.") + name).c_str(), property);
}

void SimpleDirectX11Test::LoadConfiguration()
{
	m_Configuration.LoadFromXml(
		m_PathHandler.GetPathFromResourcesDirectory("Configurations/SimpleDX11App_Configuration.xml").c_str());
	InitializeRootProperty(m_Configuration, "Title", m_Title);
	InitializeRootProperty(m_Configuration, "IsWindowed", m_IsWindowed);
	if (m_IsWindowed)
	{
		InitializeRootProperty(m_Configuration, "WindowedWidth", m_Width);
		InitializeRootProperty(m_Configuration, "WindowedHeight", m_Height);
	}
	else
	{
		auto fullScreenRes = GetPrimaryMonitorResolution();
		m_Width = fullScreenRes.x;
		m_Height = fullScreenRes.y;
	}
	InitializeRootProperty(m_Configuration, "MultisampleCount", m_MultisampleCount);
	InitializeRootProperty(m_Configuration, "BackbufferFrameCount", m_BackbufferFrameCount);
	InitializeRootProperty(m_Configuration, "IsRenderingWireframe", m_IsRenderingWireframe);
	InitializeRootProperty(m_Configuration, "IsUsingMaterialColors", m_IsUsingMaterialColors);
	InitializeRootProperty(m_Configuration, "SceneIndex", m_SceneIndex);
}