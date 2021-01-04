// FrameworkTest/SimpleDirectX12Test.cpp

#include <FrameworkTest/SimpleDirectX12Test.h>

#include <EngineBuildingBlocks/SystemTime.h>
#include <EngineBuildingBlocks/Graphics/Resources/ImageHelper.h>
#include <EngineBuildingBlocks/Input/DefaultInputBinder.h>
#include <EngineBuildingBlocks/Math/AABoundingBox.h>
#include <EngineBuildingBlocks/Graphics/Lighting/Lighting1.h>
#include <DirectX12Render/GraphicsDevice.h>
#include <DirectX12Render/Resources/IndexBuffer.h>
#include <DirectX12Render/Resources/VertexBuffer.h>
#include <DirectX12Render/Resources/StaticSampler.h>
#include <DirectX12Render/Resources/Texture2D.h>
#include <DirectX12Render/Utilities/RenderPassCB.h>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace EngineBuildingBlocks::Input;
using namespace EngineBuildingBlocks::Math;
using namespace WindowsApplication;
using namespace DirectXRender;
using namespace DirectX12Render;
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
	unsigned DiffuseTextureIndex;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class DefaultRS_RootParameterIndices
{
	TextureSRVDescriptorTable = 0,
	LightingCBV,
	RenderPassCBV,
	MaterialCBV,
	ObjectCBV,
	MaterialIndexConstant,

	COUNT
};

enum class DefaultRS_ConstanRegisters
{
	ObjectCBV = 0,
	RenderPassCBV,
	MaterialCBV,
	LightingCBV,

	MaterialIndexConstant = 0
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DX12T_Material::operator==(const DX12T_Material& other) const
{
	NumericalEqualCompareBlock(PRootSignature);
	NumericalEqualCompareBlock(PPipelineStateObject);
	NumericalEqualCompareBlock(DiffuseTexture);
	BoolEqualCompareBlock(IsUsingMaterialColors);
	MemoryEqualCompareBlock(DiffuseColor);
	MemoryEqualCompareBlock(Specularcolor);
	BoolEqualCompareBlock(IsUsingVertexColor);
	return true;
}

bool DX12T_Material::operator!=(const DX12T_Material& other) const
{
	return !(*this == other);
}

bool DX12T_Material::operator<(const DX12T_Material& other) const
{
	NumericalLessCompareBlock(PRootSignature);
	NumericalLessCompareBlock(PPipelineStateObject);
	NumericalLessCompareBlock(DiffuseTexture);
	BoolLessCompareBlock(IsUsingMaterialColors);
	MemoryLessCompareBlock(DiffuseColor);
	MemoryLessCompareBlock(Specularcolor);
	BoolLessCompareBlock(IsUsingVertexColor);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

DX12T_FrameResources::DX12T_FrameResources()
	: FenceValue(0)
	, LightingConstantBuffer(ConstantBufferMappingType::AlwaysMapped, sizeof(Lighting_Dir_Spot_CBType1<c_MaxCountLights>), 1)
	, RenderPassConstantBuffer(CreateRenderPassCB())
	, MaterialConstantBuffer(ConstantBufferMappingType::AlwaysMapped, sizeof(MaterialCBType), c_MaxCountMaterials)
	, ObjectConstantBuffer(ConstantBufferMappingType::AlwaysMapped, sizeof(ObjectCBType), c_MaxCountObjects)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SimpleDirectX12Test::OnEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handling other events.

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SimpleDirectX12Test::DerivedParseCommandLineArgs(int argc, char *argv[])
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

SimpleDirectX12Test::SimpleDirectX12Test(int argc, char *argv[])
	: WindowsApplication::Application<SimpleDirectX12Test>(argc, argv)
	, m_DX12M(m_PathHandler)
	, m_Camera(&m_SceneNodeHandler, &m_KeyHandler, &m_MouseHandler)
	, m_IsSceneNodeVectorsUpToDate(false)
	, m_CountDrawCalls(0)
	, m_IsPrintingScreen(false)
	///////////////////////
	, m_IsUsingWarpDevice(false)
	///////////////////////
	, m_frameIndex(0)
	, m_Viewport()
	, m_ScissorRect()
{
	LoadConfiguration();

	m_Window.SetTitle(m_Title);

	///////////////////////////////////////////////////////////////////////////////////

	m_Viewport.Width = static_cast<float>(m_Width);
	m_Viewport.Height = static_cast<float>(m_Height);
	m_Viewport.MaxDepth = 1.0f;

	m_ScissorRect.right = static_cast<LONG>(m_Width);
	m_ScissorRect.bottom = static_cast<LONG>(m_Height);

	for (unsigned i = 0; i < m_BackbufferFrameCount; i++)
	{
		m_BackbufferResources.emplace_back();
	}
	for (unsigned i = 0; i < m_RenderQueueSize; i++)
	{
		m_FrameResources.emplace_back();
	}

	m_Statistics_TRT.SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	m_Statistics_TRT.SetPosition(TextPositionConstant::TopLeft);
}

SimpleDirectX12Test::~SimpleDirectX12Test()
{
}

void SimpleDirectX12Test::InitializeMain()
{
	m_Window.Initialize(m_Width, m_Height);

	GetKeyHandler()->SetKeyStateProvider(&m_Window);
	GetMouseHandler()->SetMouseStateProvider(&m_Window);

	RegisterEvents();
}

void SimpleDirectX12Test::DestroyMain()
{
}

void SimpleDirectX12Test::InitializeRendering()
{
	LoadPipeline();
	LoadAssets();

	UpdateCameras();
	UpdateSceneNodes();
	UpdateRenderableSceneNodeData();
}

void SimpleDirectX12Test::RegisterEvents()
{
	m_ExitECI = m_KeyHandler.RegisterStateKeyEventListener("Exit", this);
	m_ScreenshotECI = m_KeyHandler.RegisterStateKeyEventListener("Screenshot", this);

	DefaultInputBinder::Bind(this, &m_KeyHandler, &m_MouseHandler, &m_Camera);
	m_KeyHandler.BindEventToKey(m_ScreenshotECI, Keys::P);
}

unsigned SimpleDirectX12Test::GetExitECI() const
{
	return m_ExitECI;
}

bool SimpleDirectX12Test::HandleEvent(const Event* _event)
{
	if (_event->ClassId == m_ExitECI)
	{
		RequestExiting();
		return true;
	}
	else if (_event->ClassId == m_ScreenshotECI)
	{
		m_IsPrintingScreen = true;
		return true;
	}

	return false;
}

void SimpleDirectX12Test::UpdateCameras()
{
	m_Camera.Update(m_SystemTime);
}

void SimpleDirectX12Test::UpdateSceneNodes()
{
	m_SceneNodeHandler.UpdateTransformations();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Load the rendering pipeline dependencies.
void SimpleDirectX12Test::LoadPipeline()
{
#if defined(_DEBUG)
	EnableDebugLayer();
#endif

	ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory(factory, m_Window.GetHandle());
	CreateGraphicsDevice(GraphicsDeviceType::Hardware, D3D_FEATURE_LEVEL_11_0, factory, m_device);
	CreateCommandQueue(m_device, m_commandQueue);
	CreateSwapChain(factory, m_device, m_commandQueue, m_swapChain, m_Window.GetHandle(), m_Width, m_Height,
		m_IsWindowed, m_BackbufferFrameCount, m_MaximumFrameLatency);

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Initializing descriptor heap handlers.
	{
		m_RTV_DescriptorHeap.Initialize(m_device.Get(), DescriptorHeapType::RTV, m_BackbufferFrameCount + 1);
		m_DSV_DescriptorHeap.Initialize(m_device.Get(), DescriptorHeapType::DSV, 1);
		m_CBV_SRV_UAV_DescriptorHeap.Initialize(m_device.Get(), DescriptorHeapType::CBV_SRV_UAV, c_MaxCountTextures);
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE rtvGPUHandle;

		// Create a RTV and a command allocator for each frame.
		for (UINT n = 0; n < m_BackbufferFrameCount; n++)
		{
			m_RTV_DescriptorHeap.RequestHandles(rtvCPUHandle, rtvGPUHandle);

			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_BackbufferResources[n].RenderTarget)));
			m_device->CreateRenderTargetView(m_BackbufferResources[n].RenderTarget.Get(), nullptr, rtvCPUHandle);
		}
	}

	// Creating command allocators.
	for (unsigned i = 0; i < m_RenderQueueSize; i++)
	{
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_FrameResources[i].CommandAllocator)));
	}

	// Creating multisampled render target buffer.
	{
		DirectX12Render::Texture2DDescription description;
		description.Width = m_Width;
		description.Height = m_Height;
		description.SampleCount = m_MultisampleCount;
		m_RenderTargetBuffer.Initialize(m_device.Get(), description, &m_RTV_DescriptorHeap);
	}

	// Creating the depth stencil buffer.
	{
		DirectX12Render::Texture2DDescription description;
		description.Width = m_Width;
		description.Height = m_Height;
		description.SampleCount = m_MultisampleCount;
		m_DepthStencilBuffer.Initialize(m_device.Get(), description, &m_DSV_DescriptorHeap);
	}

	// Creating the color texture and readback buffer.
	{
		DirectX12Render::Texture2DDescription description;
		description.Width = m_Width;
		description.Height = m_Height;
		description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		for (unsigned i = 0; i < m_RenderQueueSize; i++)
		{
			m_FrameResources[i].ColorTexture.Initialize(m_device.Get(), description,
				D3D12_RESOURCE_STATE_RESOLVE_DEST);
			m_FrameResources[i].ColorReadbackBuffer = m_DX12M.TransferBufferManager.RequestReadbackBuffer(
					m_device.Get(), description);
			m_FrameResources[i].HasPrintScreenResult = false;
		}
	}

	// Creating the command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_FrameResources[m_frameIndex].CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
}

RootSignature* SimpleDirectX12Test::GetDefaultRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE ranges[1];
	CD3DX12_ROOT_PARAMETER rootParameters[(int)DefaultRS_RootParameterIndices::COUNT];

	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, c_MaxCountTextures, 0);

	rootParameters[(int)DefaultRS_RootParameterIndices::LightingCBV].InitAsConstantBufferView(
		(int)DefaultRS_ConstanRegisters::LightingCBV, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[(int)DefaultRS_RootParameterIndices::RenderPassCBV].InitAsConstantBufferView(
		(int)DefaultRS_ConstanRegisters::RenderPassCBV, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[(int)DefaultRS_RootParameterIndices::MaterialCBV].InitAsConstantBufferView(
		(int)DefaultRS_ConstanRegisters::MaterialCBV, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[(int)DefaultRS_RootParameterIndices::ObjectCBV].InitAsConstantBufferView(
		(int)DefaultRS_ConstanRegisters::ObjectCBV, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[(int)DefaultRS_RootParameterIndices::TextureSRVDescriptorTable].InitAsDescriptorTable(1,
		&ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[(int)DefaultRS_RootParameterIndices::MaterialIndexConstant].InitAsConstants(
		1, (int)DefaultRS_ConstanRegisters::MaterialIndexConstant, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	auto sampler = StaticSamplerHelper::GetDefaultAnisotropicSamplerDescription(0);
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootSignatureDescription rootSignatureDesc;
	rootSignatureDesc.Initialize(_countof(rootParameters), rootParameters, 1, &sampler,
		RootSignatureDescription::c_IA_VertexPixelShader_Flags);

	return m_DX12M.RootSignatureManager.GetRootSignature(m_device.Get(), rootSignatureDesc);
}

GraphicsPipelineStateObject* SimpleDirectX12Test::GetOpaquePSO(
	const DirectX12Render::VertexInputLayout& vertexInputLayoutDescription,
	bool hasMaterialColors, bool hasVertexColors)
{
	// Getting the root signature.
	auto pRootSignature = GetDefaultRootSignature();

	std::vector<ShaderDefine> shaderDefines = {
		{ "IS_USING_MATERIAL_COLORS", hasMaterialColors },
		{ "IS_USING_VERTEX_COLOR", hasVertexColors }
	};
	auto vertexShader = m_DX12M.ShaderManager.GetShader(m_device.Get(), { "Test/ShaderTest.hlsl", "VSMain",
		ShaderType::Vertex, "5_1", 0, shaderDefines });
	auto pixelShader = m_DX12M.ShaderManager.GetShader(m_device.Get(), { "Test/ShaderTest.hlsl", "PSMain",
		ShaderType::Pixel, "5_1", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, shaderDefines });

	GraphicsPipelineStateObjectDescription description;
	description.SampleDesc.Count = m_MultisampleCount;
	description.PRootSignature = pRootSignature;
	description.PVertexShader = vertexShader;
	description.PPixelShader = pixelShader;
	description.PVertexInputLayout = &vertexInputLayoutDescription;
	if (m_IsRenderingWireframe) description.RasterizerState.SetWireframeFillMode();

	return m_DX12M.PipelineStateObjectManager.GetGraphicsPipelineStateObject(m_device.Get(), description);
}

GraphicsPipelineStateObject* SimpleDirectX12Test::GetTransparentPSO(
	const DirectX12Render::VertexInputLayout& vertexInputLayoutDescription,
	bool hasMaterialColors, bool hasVertexColors)
{
	// Getting the root signature.
	auto pRootSignature = GetDefaultRootSignature();

	std::vector<ShaderDefine> shaderDefines = {
		{ "IS_USING_MATERIAL_COLORS", hasMaterialColors },
		{ "IS_USING_VERTEX_COLOR", hasVertexColors }
	};
	auto vertexShader = m_DX12M.ShaderManager.GetShader(m_device.Get(), { "Test/ShaderTest.hlsl", "VSMain",
		ShaderType::Vertex, "5_1", 0, shaderDefines });
	auto pixelShader = m_DX12M.ShaderManager.GetShader(m_device.Get(), { "Test/ShaderTest.hlsl", "PSMain",
		ShaderType::Pixel, "5_1", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, shaderDefines });

	GraphicsPipelineStateObjectDescription description;
	description.SampleDesc.Count = m_MultisampleCount;
	description.PRootSignature = pRootSignature;
	description.PVertexShader = vertexShader;
	description.PPixelShader = pixelShader;
	description.PVertexInputLayout = &vertexInputLayoutDescription;
	description.RasterizerState.DisableCulling();
	description.BlendState.EnableAlphaToCoverage();
	if (m_IsRenderingWireframe) description.RasterizerState.SetWireframeFillMode();

	return m_DX12M.PipelineStateObjectManager.GetGraphicsPipelineStateObject(m_device.Get(), description);
}

Texture2D* SimpleDirectX12Test::GetTexture(const std::string& textureName)
{
	auto textureGettingResult = m_DX12M.ConstantTextureManager.GetTexture2DFromFile(m_device.Get(),
		m_commandList.Get(), &m_CBV_SRV_UAV_DescriptorHeap, textureName);
	auto texture = textureGettingResult.Texture;
	if (textureGettingResult.IsTextureCreated)
	{
		texture->TransitionToPixelShaderResource(m_commandList.Get());
	}
	return texture;
}

Texture2D* SimpleDirectX12Test::GetTexture(const std::string& diffuseTextureName,
	const std::string& opacityTextureName)
{
	auto textureGettingResult = m_DX12M.ConstantTextureManager.GetDiffuseTexture2DWithOpacity(
		m_device.Get(), m_commandList.Get(), &m_CBV_SRV_UAV_DescriptorHeap,
		diffuseTextureName, opacityTextureName, c_SceneData[m_SceneIndex].OpacityChannelIndex);
	auto texture = textureGettingResult.Texture;
	if (textureGettingResult.IsTextureCreated)
	{
		texture->TransitionToPixelShaderResource(m_commandList.Get());
	}
	return texture;
}

unsigned SimpleDirectX12Test::GetMaterialIndex(unsigned objectIndex,
	const ModelLoadingResult& loadRes, const ModelInstantiationResult& instRes,
	const DirectX12Render::VertexInputLayout& vertexInputLayoutDescription)
{
	auto& model = m_ModelLoader.GetModel(loadRes.ModelIndex);
	unsigned objectMaterialIndex = model.Objects[objectIndex].MaterialIndex;
	auto& objectMaterial = model.Materials[objectMaterialIndex];

	DX12T_Material material;
	material.IsUsingMaterialColors = m_IsUsingMaterialColors;
	if (material.IsUsingMaterialColors)
	{
		material.DiffuseColor = objectMaterial.DiffuseColor;
		material.Specularcolor = objectMaterial.SpecularColor;
	}
	material.IsUsingVertexColor = vertexInputLayoutDescription.InputLayout.HasVertexColors();

	if (objectMaterial.OpacityTextureName.empty())
	{
		material.DiffuseTexture = GetTexture(objectMaterial.DiffuseTextureName);
		material.PPipelineStateObject = GetOpaquePSO(vertexInputLayoutDescription,
			material.IsUsingMaterialColors, material.IsUsingVertexColor);
	}
	else
	{
		material.PPipelineStateObject = GetTransparentPSO(vertexInputLayoutDescription,
			material.IsUsingMaterialColors, material.IsUsingVertexColor);
		material.DiffuseTexture = GetTexture(objectMaterial.DiffuseTextureName,
			objectMaterial.OpacityTextureName);
	}

	material.PRootSignature = material.PPipelineStateObject->GetRootSignature();

	unsigned materialIndex;
	if (!m_Materials.IsContaining(material, materialIndex))
	{
		materialIndex = m_Materials.Add(material);

		for (unsigned j = 0; j < m_RenderQueueSize; j++)
		{
			auto& cb = m_FrameResources[j].MaterialConstantBuffer;
			cb.AddConstantBuffer();
			auto& materialCBData = cb.Access<MaterialCBType>(materialIndex);
			materialCBData.DiffuseTextureIndex = material.DiffuseTexture->GetShaderResourceView().IndexOnDescriptorHeap;
			if (material.IsUsingMaterialColors)
			{
				materialCBData.DiffuseColor = material.DiffuseColor;
				materialCBData.SpecularColor = material.Specularcolor;
			}
		}
	}

	return materialIndex;
}

unsigned SimpleDirectX12Test::AddRenderTask(unsigned objectIndex, 
	const ModelLoadingResult& loadRes, const ModelInstantiationResult& instRes,
	VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer)
{
	auto& model = m_ModelLoader.GetModel(loadRes.ModelIndex);
	auto& objectData = model.Objects[objectIndex];
	auto& nodeMapping = instRes.GlobalSceneNodeMapping;

	// Creating a constant buffer for the render task.
	unsigned objectCBIndex = 0;
	for (unsigned i = 0; i < m_RenderQueueSize; i++)
	{
		// They all return the same index.
		objectCBIndex = m_FrameResources[i].ObjectConstantBuffer.AddConstantBuffer();
	}

	// Creating a scene node for the render task.
	DX12T_RenderTask renderTask;
	renderTask.SceneNodeIndex = nodeMapping[objectData.SceneNodeIndex];
	renderTask.ObjectCBIndex = objectCBIndex;
	renderTask.MaterialIndex = GetMaterialIndex(objectIndex, loadRes, instRes, pVertexBuffer->GetInputLayout());

	// Testing model loading.
	auto& geometryData = loadRes.Meshes[objectData.MeshIndex];
	renderTask.Primitive.PVertexBuffer = pVertexBuffer;
	renderTask.Primitive.PIndexBuffer = pIndexBuffer;
	renderTask.Primitive.CountVertices = geometryData.CountVertices;
	renderTask.Primitive.CountIndices = geometryData.CountIndices;
	renderTask.Primitive.BaseVertex = geometryData.BaseVertex;
	renderTask.Primitive.BaseIndex = geometryData.BaseIndex;
	
	renderTask.BoundingBox = AABoundingBox::GetBoundingBox(
		m_VertexData.GetPositions() + geometryData.BaseVertex, geometryData.CountVertices);

	return m_RenderTasks.Add(renderTask);
}

// Load the sample assets.
void SimpleDirectX12Test::LoadAssets()
{
	for (unsigned i = 0; i < m_RenderQueueSize; i++)
	{
		m_FrameResources[i].LightingConstantBuffer.Initialize(m_device.Get());
		m_FrameResources[i].RenderPassConstantBuffer.Initialize(m_device.Get());
		m_FrameResources[i].MaterialConstantBuffer.Initialize(m_device.Get());
		m_FrameResources[i].ObjectConstantBuffer.Initialize(m_device.Get());
	
		m_FrameResources[i].LightingConstantBuffer.AddConstantBuffer();
		m_FrameResources[i].RenderPassConstantBuffer.AddConstantBuffer();
	}

	// Loading model.
	{
		auto& sceneData = c_SceneData[m_SceneIndex];
		IndexData indexData;
		ModelLoadingDescription modelDesc;
		modelDesc.BuildingDescription.FilePath = sceneData.Name;
		modelDesc.BuildingDescription.GeometryOptions.IsOptimizingMeshes = c_IsOptimizingMeshes;
		modelDesc.BuildingDescription.GeometryOptions.AreVertexColorsAllowed = m_IsUsingVertexColors;
		auto modelLoadingResult = m_ModelLoader.Load(modelDesc, m_VertexData, indexData);
		auto modelInstantiationResult = m_ModelLoader.Instatiate(modelLoadingResult.ModelIndex, m_SceneNodeHandler);
		auto model = m_ModelLoader.GetModel(modelLoadingResult.ModelIndex);
		auto countObjects = modelLoadingResult.Meshes.GetSize();
		auto countMaterials = static_cast<unsigned>(model.Materials.size());

		auto pVB = m_DX12M.PrimitiveManager.CreateVertexBuffer(m_VertexData, &m_DX12M.TransferBufferManager,
			m_commandList.Get(), m_device.Get());
		auto pIB = m_DX12M.PrimitiveManager.CreateIndexBuffer(indexData, &m_DX12M.TransferBufferManager,
			m_commandList.Get(), m_device.Get());

		// Adding objects.
		for (unsigned passIndex = 0; passIndex < 2; passIndex++)
		{
			for (unsigned i = 0; i < countObjects; i++)
			{
				unsigned objectMaterialIndex = model.Objects[i].MaterialIndex;
				bool isOpaque = model.Materials[objectMaterialIndex].OpacityTextureName.empty();

				if (passIndex == 0 && isOpaque || passIndex == 1 && !isOpaque)
				{
					unsigned renderTaskIndex = AddRenderTask(i, modelLoadingResult, modelInstantiationResult,
						pVB, pIB);
				}
			}
		}

		// Setting position, orientation, scaler.
		auto orientationYPR = glm::radians(sceneData.OrientationYPR);
		auto orientation = glm::mat3(glm::yawPitchRoll(orientationYPR[0], orientationYPR[1], orientationYPR[2]));
		m_SceneNodeHandler.SetPosition(modelInstantiationResult.GlobalRootSceneNodeIndex, sceneData.Position);
		m_SceneNodeHandler.SetOrientation(modelInstantiationResult.GlobalRootSceneNodeIndex, orientation);
		m_SceneNodeHandler.SetLocalScaler(modelInstantiationResult.GlobalRootSceneNodeIndex, glm::vec3(sceneData.Scaler));
	}

	auto initContext = UtilityInitializationContext{ m_device.Get(), m_commandList.Get(), &m_PathHandler,
		&m_DX12M, m_RenderQueueSize, m_MultisampleCount };
	m_DX12U.Initialize(initContext, m_CBV_SRV_UAV_DescriptorHeap);
	m_DX12U.BackgroundCMRenderer.AddTexture(initContext, m_CBV_SRV_UAV_DescriptorHeap, "Cubemaps/cloudyNoon.dds");

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

	// Closing the command list and executing it to begin the initial GPU setup.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(m_FrameResources[m_frameIndex].FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_FrameResources[m_frameIndex].FenceValue++;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForGpu();
	}

	m_DX12M.TransferBufferManager.ReleaseAllUploadBuffers();

	for (unsigned i = 0; i < m_RenderQueueSize; i++)
	{
		UpdateLightingCBData(i);
		UpdateMaterialCBData(i);
	}
}

void SimpleDirectX12Test::UpdateLightingCBData(unsigned frameIndex)
{
	auto& cb = m_FrameResources[frameIndex].LightingConstantBuffer;
	cb.StartWrite();
	auto& lightingCBData = cb.Access<Lighting_Dir_Spot_CBType1<c_MaxCountLights>>(0);
	CreateDefaultLighting1(lightingCBData);
	cb.EndWrite();
}

void SimpleDirectX12Test::UpdateRenderPassCBData(unsigned frameIndex)
{
	UpdateRenderPassCB(&m_Camera, &m_FrameResources[frameIndex].RenderPassConstantBuffer);
}

void SimpleDirectX12Test::UpdateMaterialCBData(unsigned frameIndex)
{
	auto& cb = m_FrameResources[frameIndex].MaterialConstantBuffer;

	cb.StartWrite();
	cb.IndicateWrittenRangeWithViewIndex(0, m_Materials.GetSize());
	cb.EndWrite();
}

void SimpleDirectX12Test::UpdateObjectCBData(unsigned frameIndex)
{
	auto& cb = m_FrameResources[frameIndex].ObjectConstantBuffer;

	auto& viewProjectionMatrix = m_Camera.GetViewProjectionMatrix();

	glm::mat4 worldMatrix;

	cb.StartWrite();

	unsigned startObjectIndex = std::numeric_limits<unsigned>::max();
	unsigned endObjectIndex = 0;

	unsigned countRenderTasks = m_RenderTasks.GetSize();
	for (unsigned i = 0; i < countRenderTasks; i++)
	{
		auto& renderTask = m_RenderTasks[i];
		unsigned sceneNodeIndex = renderTask.SceneNodeIndex;
		auto& objectCBData = cb.Access<ObjectCBType>(renderTask.ObjectCBIndex);

		worldMatrix = m_SceneNodeHandler.GetScaledWorldTransformation(sceneNodeIndex).AsMatrix4();
		objectCBData.ModelMatrix = worldMatrix;
		objectCBData.InverseModelMatrix = m_SceneNodeHandler.GetInverseWorldTransformation(sceneNodeIndex).AsMatrix4();
		objectCBData.ModelViewProjectionMatrix = viewProjectionMatrix * worldMatrix;
	
		startObjectIndex = std::min(startObjectIndex, renderTask.ObjectCBIndex);
		endObjectIndex = std::max(endObjectIndex, renderTask.ObjectCBIndex + 1);
	}

	cb.IndicateWrittenRangeWithViewIndex(startObjectIndex, endObjectIndex);
	cb.EndWrite();
}

void SimpleDirectX12Test::UpdateRenderableSceneNodeData()
{
	// Updating renderable scene node indices.
	if (!m_IsSceneNodeVectorsUpToDate)
	{
		m_RenderTaskIndices.Resize(m_RenderTasks.GetSize());

		auto it = m_RenderTasks.GetBeginIterator();
		auto end = m_RenderTasks.GetEndIterator();
		for (int i = 0; it != end; ++i, ++it)
		{
			m_RenderTaskIndices[i] = m_RenderTasks.ToIndex(it);
		}

		m_IsSceneNodeVectorsUpToDate = true;
	}

	// View frustum culling.
	m_ViewFrustumCuller.ViewFrustumCull(m_Camera, m_ThreadPool, m_SceneNodeHandler,
		m_RenderTasks.GetArray(), m_RenderTaskIndices.GetArray(), m_ViewFrustumCulledRenderTaskIndices,
		m_RenderTasks.GetSize());
}

void SimpleDirectX12Test::DerivedPreUpdate()
{
	UpdateRenderPassCBData(m_frameIndex);
	UpdateObjectCBData(m_frameIndex);
	m_DX12U.TextRenderer.Update({ m_frameIndex });
}

// Update frame-based values.
void SimpleDirectX12Test::DerivedPostUpdate(EngineBuildingBlocks::PostUpdateContext& context)
{
	UpdateCameras();
	UpdateSceneNodes();
	UpdateRenderableSceneNodeData();

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
		m_Statistics_TRT.Update(m_DX12U.TextRenderer);
	}
}

// Render the scene.
void SimpleDirectX12Test::DerivedRender(RenderContext& context)
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void SimpleDirectX12Test::DerivedPresent()
{
	// In fullscreen mode we present all frames, in windowed mode, we only present, if no waiting is needed.
	if (!m_IsWindowed || !c_IsUsingWindowedNoWaitingTrick
		|| ((WaitForSingleObjectEx(m_swapChain->GetFrameLatencyWaitableObject(), 0, false)) == WAIT_OBJECT_0))
	{
		// Presenting the frame.
		ThrowIfFailed(m_swapChain->Present(m_SyncInterval, 0));
	
		MoveToNextFrame();
	}
}

void SimpleDirectX12Test::DestroyRendering()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForGpu();

	CloseHandle(m_fenceEvent);

	if (!m_IsWindowed) m_swapChain->SetFullscreenState(FALSE, nullptr);
}

void SimpleDirectX12Test::RenderObjects()
{
	ID3D12PipelineState* prevPSO = nullptr;
	unsigned prevMaterialIndex = Core::c_InvalidIndexU;

	unsigned countRenderTasks = m_ViewFrustumCulledRenderTaskIndices.GetSize();
	for (unsigned i = 0; i < countRenderTasks; i++)
	{
		auto& renderTask = m_RenderTasks[m_ViewFrustumCulledRenderTaskIndices[i]];
		auto& primitive = renderTask.Primitive;
		unsigned objectCBIndex = renderTask.ObjectCBIndex;
		unsigned materialIndex = renderTask.MaterialIndex;

		// This is material level, but we don't sort by material now.
		if(materialIndex != prevMaterialIndex)
		{
			auto& material = m_Materials[materialIndex];
			auto pso = material.PPipelineStateObject->GetPipelineStateObject();
			if (pso != prevPSO)
			{
				m_commandList->SetPipelineState(pso);
				m_commandList->SetGraphicsRootSignature(material.PRootSignature->GetRootSignature());
				
				// This is frame/render pass level, but must be done after the root signature has been bound.
				m_commandList->SetGraphicsRootConstantBufferView((int)DefaultRS_RootParameterIndices::LightingCBV,
					m_FrameResources[m_frameIndex].LightingConstantBuffer.GetConstantBufferGPUVirtualAddress(0));
				m_commandList->SetGraphicsRootConstantBufferView((int)DefaultRS_RootParameterIndices::RenderPassCBV,
					m_FrameResources[m_frameIndex].RenderPassConstantBuffer.GetConstantBufferGPUVirtualAddress(0));
				
				prevPSO = pso;
			}

			m_commandList->SetGraphicsRoot32BitConstant((int)DefaultRS_RootParameterIndices::MaterialIndexConstant,
				materialIndex, 0);
			m_commandList->SetGraphicsRootConstantBufferView((int)DefaultRS_RootParameterIndices::MaterialCBV,
				m_FrameResources[m_frameIndex].MaterialConstantBuffer.
				GetConstantBufferGPUVirtualAddress(materialIndex));

			m_commandList->SetGraphicsRootDescriptorTable((int)DefaultRS_RootParameterIndices::TextureSRVDescriptorTable,
				m_CBV_SRV_UAV_DescriptorHeap.GetStartGPUDescriptorHandle());

			prevMaterialIndex = materialIndex;
		}

		SetIndexedPrimitiveForInputAssembler(primitive, m_commandList.Get());

		// Setting object level constant buffers.
		m_commandList->SetGraphicsRootConstantBufferView((int)DefaultRS_RootParameterIndices::ObjectCBV,
			m_FrameResources[m_frameIndex].ObjectConstantBuffer.GetConstantBufferGPUVirtualAddress(objectCBIndex));

		// Drawing primitive.
		m_commandList->DrawIndexedInstanced(primitive.CountIndices, 1,
			primitive.BaseIndex, primitive.BaseVertex, 0);
	}

	m_CountDrawCalls += countRenderTasks;
}

void SimpleDirectX12Test::RenderBackgroundCubemap()
{
	m_DX12U.BackgroundCMRenderer.Render({ m_commandList.Get(), &m_DX12M, m_frameIndex },
		&m_FrameResources[m_frameIndex].RenderPassConstantBuffer, 0, 0);
	m_CountDrawCalls++;
}

void SimpleDirectX12Test::RenderText()
{
	m_DX12U.TextRenderer.Render({ m_commandList.Get(), &m_DX12M, m_frameIndex });
}

void SimpleDirectX12Test::PopulateCommandList()
{
	// Command list level.
	{
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		ThrowIfFailed(m_FrameResources[m_frameIndex].CommandAllocator->Reset());

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.

		// We don't set the initial pipeline state. Note that it might worth it to do it for bundles.

		ThrowIfFailed(m_commandList->Reset(m_FrameResources[m_frameIndex].CommandAllocator.Get(), nullptr));

		ID3D12DescriptorHeap* ppHeaps[] = { m_CBV_SRV_UAV_DescriptorHeap.GetHeap() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	}

	m_CountDrawCalls = 0;

	// Render target level.
	{
		m_commandList->RSSetViewports(1, &m_Viewport);
		m_commandList->RSSetScissorRects(1, &m_ScissorRect);

		auto rtvHandle = m_RTV_DescriptorHeap.GetCPUDescriptorHandle(m_BackbufferFrameCount); // The multisampled render target.
		auto dsvHandle = m_DSV_DescriptorHeap.GetStartCPUDescriptorHandle();
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	
		auto clearColor = c_DefaultRenderTargetClearColor;
		if(c_IsClearingRTV) m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	RenderObjects();
	RenderBackgroundCubemap();
	RenderText();

	// Resolving subresource.
	{
		auto& frameResources = m_FrameResources[m_frameIndex];
		auto& backbufferResources = m_BackbufferResources[m_swapChain->GetCurrentBackBufferIndex()];

		auto sourceResource = m_RenderTargetBuffer.GetResource();
		auto destinationResource = backbufferResources.RenderTarget.Get();

		CD3DX12_RESOURCE_BARRIER startBarriers[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(sourceResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(destinationResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST)
		};
		CD3DX12_RESOURCE_BARRIER endBarriers[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(sourceResource, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
			CD3DX12_RESOURCE_BARRIER::Transition(destinationResource, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PRESENT)
		};

		m_commandList->ResourceBarrier(2, startBarriers);	
		m_commandList->ResolveSubresource(destinationResource, 0, m_RenderTargetBuffer.GetResource(), 0,
			m_RenderTargetBuffer.GetDescription().Format);

		if (m_IsPrintingScreen)
		{
			m_commandList->ResolveSubresource(
				m_FrameResources[m_frameIndex].ColorTexture.GetResource(), 0,
				m_RenderTargetBuffer.GetResource(), 0,
				m_RenderTargetBuffer.GetDescription().Format);
			frameResources.ColorTexture.CopyData(m_commandList.Get(), frameResources.ColorReadbackBuffer);

			m_IsPrintingScreen = false;
			m_FrameResources[m_frameIndex].HasPrintScreenResult = true;
		}

		m_commandList->ResourceBarrier(2, endBarriers);
	}

	// Command list level.
	{
		ThrowIfFailed(m_commandList->Close());
	}
}

// Wait for pending GPU work to complete.
void SimpleDirectX12Test::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_FrameResources[m_frameIndex].FenceValue));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_FrameResources[m_frameIndex].FenceValue, m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_FrameResources[m_frameIndex].FenceValue++;
}

// Prepare to render the next frame.
void SimpleDirectX12Test::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_FrameResources[m_frameIndex].FenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	// Update the frame index.
	m_frameIndex = (m_frameIndex + 1) % m_RenderQueueSize;

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fence->GetCompletedValue() < m_FrameResources[m_frameIndex].FenceValue)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_FrameResources[m_frameIndex].FenceValue, m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_FrameResources[m_frameIndex].FenceValue = currentFenceValue + 1;

	if (currentFenceValue > 2) // In the first call we haven't rendered anything into the frame with index 1.
	{
		OnFrameCompleted();
	}
}

void SimpleDirectX12Test::OnFrameCompleted()
{
	auto& frameResources = m_FrameResources[m_frameIndex];
	if (frameResources.HasPrintScreenResult)
	{
		frameResources.HasPrintScreenResult = false;

		unsigned colorBufferSize = m_Width * m_Height * 4 * sizeof(unsigned char);
		m_PrintScreenBuffer.Resize(colorBufferSize);

		frameResources.ColorReadbackBuffer->GetData(m_PrintScreenBuffer.GetArray(), colorBufferSize);

		auto& tDescription = frameResources.ColorTexture.GetDescription();
		auto description = EngineBuildingBlocks::Graphics::Image2DDescription::ColorImage(
			tDescription.Width, tDescription.Height, tDescription.GetCountChannels());
		SaveImageToFile(m_PrintScreenBuffer.GetArray(),
			description, m_PathHandler.GetPathFromExecutableDirectory("screenshot.png"),
			ImageSaveFlags::IsConverting_RGB_BGR | ImageSaveFlags::IsFlippingY);
	}
}

template <typename T>
inline void InitializeRootProperty(const Core::Properties& configuration, const char* name, T& property)
{
	configuration.TryGetPropertyValue((std::string("Configuration.") + name).c_str(), property);
}

void SimpleDirectX12Test::LoadConfiguration()
{
	m_Configuration.LoadFromXml(
		m_PathHandler.GetPathFromResourcesDirectory("Configurations/SimpleDX12App_Configuration.xml").c_str());
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
	InitializeRootProperty(m_Configuration, "MaximumFrameLatency", m_MaximumFrameLatency);
	InitializeRootProperty(m_Configuration, "RenderQueueSize", m_RenderQueueSize);
	InitializeRootProperty(m_Configuration, "IsRenderingWireframe", m_IsRenderingWireframe);
	InitializeRootProperty(m_Configuration, "IsUsingMaterialColors", m_IsUsingMaterialColors);
	InitializeRootProperty(m_Configuration, "SceneIndex", m_SceneIndex);
	InitializeRootProperty(m_Configuration, "IsUsingVertexColors", m_IsUsingVertexColors);
}