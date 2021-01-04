// DirectX12Render/Utilities/TextRenderer.cpp

#include <DirectX12Render/Utilities/TextRenderer.h>

#include <Core/String.hpp>
#include <Core/Parse.hpp>
#include <Core/System/SimpleIO.h>
#include <Core/Constants.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <DirectX12Render/Resources/StaticSampler.h>
#include <DirectX12Render/Resources/RootSignature.h>
#include <DirectX12Render/Resources/Shader.h>
#include <DirectX12Render/Resources/PipelineStateObject.h>
#include <DirectX12Render/Resources/ConstantTextureManager.h>
#include <DirectX12Render/Resources/Texture2D.h>
#include <DirectX12Render/Resources/DescriptorHeap.h>
#include <DirectX12Render/Resources/IndexBuffer.h>

#include <filesystem>
#include <algorithm>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace DirectXRender;
using namespace DirectX12Render;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void LoadFont(const std::string& fileName, DirectX12Render::Font& font)
{
	auto& glyphs = font.Glyphs;

	int maximumHeight = 0;

	std::string text;
	Core::ReadAllText(fileName, text);
	auto lines = Core::Split(text, Core::IsNewLine(), true);
	auto countLines = static_cast<unsigned>(lines.size());
	for (unsigned i = 0; i < countLines; i++)
	{
		auto& line = lines[i];
		auto parts = Core::Split(line, true);
		auto& descriptor = parts[0];
		if (descriptor == "info")
		{
			auto facePosition = line.find("face");
			auto dataPosition = line.find("=", facePosition);
			auto firstQuotePosition = line.find("\"", dataPosition);
			auto secondQuotePosition = line.find("\"", firstQuotePosition + 1);
			font.Name = line.substr(firstQuotePosition + 1, secondQuotePosition - firstQuotePosition - 1);
		}
		else if (descriptor == "char")
		{
			Glyph glyph;
			auto countParts = static_cast<unsigned>(parts.size());
			for (unsigned j = 1; j < countParts; j++)
			{
				auto currentData = Core::Split(parts[j], Core::IsCharacter('='), true);

				if (currentData[0] == "id")
				{
					glyph.ID = Core::Parse<int>(currentData[1].c_str());
				}
				else if (currentData[0] == "x")
				{
					glyph.X = Core::Parse<int>(currentData[1].c_str());
				}
				else if (currentData[0] == "y")
				{
					glyph.Y = Core::Parse<int>(currentData[1].c_str());
				}
				else if (currentData[0] == "width")
				{
					glyph.Width = Core::Parse<int>(currentData[1].c_str());
				}
				else if (currentData[0] == "height")
				{
					auto height = Core::Parse<int>(currentData[1].c_str());
					glyph.Height = height;
					maximumHeight = std::max(maximumHeight, height);
				}
				else if (currentData[0] == "xoffset")
				{
					glyph.XOffset = Core::Parse<float>(currentData[1].c_str());
				}
				else if (currentData[0] == "yoffset")
				{
					glyph.YOffset = Core::Parse<float>(currentData[1].c_str());
				}
				else if (currentData[0] == "xadvance")
				{
					glyph.XAdvance = Core::Parse<float>(currentData[1].c_str());
				}
				else if (currentData[0] == "page")
				{
					assert(Core::Parse<int>(currentData[1].c_str()) == 0);
				}
				else if (currentData[0] == "chnl")
				{
					assert(Core::Parse<int>(currentData[1].c_str()) == 0);
				}
			}

			if (glyph.ID >= static_cast<int>(glyphs.GetSize()))
			{
				glyphs.ResizeWithGrowing(glyph.ID + 1);
			}
			glyphs[glyph.ID] = glyph;
		}
	}

	font.MaximumHeight = maximumHeight;
}

inline bool IsPrintableCharacter(unsigned id)
{
	if (id > 0x20 && id < 0xff && id != 0x7f)
	{
		return true;
	}
	return false;
}

inline bool PreparePositionForCharacter(glm::vec2& position, float xSize, float spacedLineHeight, float xStart)
{
	if (position.x + xSize > 1.0f)
	{
		position.x = xStart;
		position.y -= spacedLineHeight;
		return false;
	}
	return true;
}

inline void IncrementPosition(glm::vec2& position, float xIncrement)
{
	position.x += xIncrement;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

enum class Ranges
{
	Textures_SRV = 0,

	COUNT
};

enum class RootParameters
{
	TextureSRVDescriptorTable = 0,
	DepthRC,
	ColorRC,

	COUNT
};

enum class Registers
{
	TextureSRV = 0,
	DepthRC = 0,
	ColorRC = 0
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

TextRenderFrameResources::TextRenderFrameResources()
	: IsDirty(true)
{
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

TextRenderer::TextRenderer()
{
}

void TextRenderer::Initialize(const UtilityInitializationContext& context,
	StreamedDescriptorHeap& CBV_SRV_UAV_DescriptorHeap)
{
	LoadFonts(context);

	m_FrameResources.resize(context.FrameCount);

	CreatePrimitive(context);
	CreateSymbolInstaceBuffer(context);
	CreateRootSignature(context);
	CreatePipelineStateObject(context);
	CreateTextures(context, CBV_SRV_UAV_DescriptorHeap);
}

void TextRenderer::LoadFonts(const UtilityInitializationContext& context)
{
	auto fontDirectory = context.PathHandler->GetPathFromResourcesDirectory("Fonts/SDFF");
	std::filesystem::directory_iterator endFileIt;
	std::filesystem::directory_iterator fileIt(fontDirectory);
	for (; fileIt != endFileIt; ++fileIt)
	{
		auto currentPath = fileIt->path();
		if (currentPath.has_extension() && currentPath.extension() == ".txt") // Processing the glyph files.
		{
			FontData fontData;
			fontData.FileName = currentPath.stem().string();
			LoadFont(currentPath.string(), fontData.Font);
			m_Fonts.push_back(std::move(fontData));
		}
	}
}

void TextRenderer::CreatePrimitive(const UtilityInitializationContext& context)
{
	m_Primitive = context.DX12M->PrimitiveManager.GetIndexedPrimitive(
			QuadDescription(PrimitiveRange::_0_To_Plus1), &context.DX12M->TransferBufferManager,
			context.CommandList, context.Device);
}

void TextRenderer::CreateRootSignature(const UtilityInitializationContext& context)
{
	CD3DX12_DESCRIPTOR_RANGE ranges[(int)Ranges::COUNT];
	CD3DX12_ROOT_PARAMETER rootParameters[(int)RootParameters::COUNT];

	ranges[(int)Ranges::Textures_SRV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, (int)Registers::TextureSRV);
	rootParameters[(int)RootParameters::TextureSRVDescriptorTable].InitAsDescriptorTable(1, &ranges[(int)Ranges::Textures_SRV],
		D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[(int)RootParameters::DepthRC].InitAsConstants(1, (int)Registers::DepthRC, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[(int)RootParameters::ColorRC].InitAsConstants(4, (int)Registers::ColorRC, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	auto sampler = DirectX12Render::StaticSamplerHelper::GetDefaultTrilinearSamplerDescription(0);
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	DirectX12Render::RootSignatureDescription rootSignatureDesc;
	rootSignatureDesc.Initialize(_countof(rootParameters), rootParameters, 1, &sampler,
		RootSignatureDescription::c_IA_VertexPixelShader_Flags);

	m_RootSignature = context.DX12M->RootSignatureManager.GetRootSignature(context.Device, rootSignatureDesc);
}

void TextRenderer::CreatePipelineStateObject(const UtilityInitializationContext& context)
{
	auto vertexShader = context.DX12M->ShaderManager.GetShaderSimple(context.Device,
	{ "TextRender.hlsl", "SDF_VS_Main", ShaderType::Vertex });
	auto pixelShader = context.DX12M->ShaderManager.GetShaderSimple(context.Device,
	{ "TextRender.hlsl", "SDF_PS_Main", ShaderType::Pixel });

	DirectX12Render::GraphicsPipelineStateObjectDescription description;
	description.SampleDesc.Count = context.MultisampleCount;
	description.PRootSignature = m_RootSignature;
	description.PVertexShader = vertexShader;
	description.PPixelShader = pixelShader;
	description.PVertexInputLayout = &m_MergedIL;
	description.RasterizerState.DisableCulling();
	description.DepthStencilState.DisableDepthWriting();
	description.BlendState.SetToNonpremultipliedAlphaBlending();

	m_PipelineStateObject = context.DX12M->PipelineStateObjectManager.GetGraphicsPipelineStateObject(context.Device, description);
}

void TextRenderer::CreateTextures(const UtilityInitializationContext& context,
	StreamedDescriptorHeap& CBV_SRV_UAV_DescriptorHeap)
{
	unsigned countFonts = static_cast<unsigned>(m_Fonts.size());
	for (unsigned i = 0; i < countFonts; i++)
	{
		auto& fontData = m_Fonts[i];
		auto imageFileName = context.PathHandler->GetPathFromResourcesDirectory(
			std::string("Fonts/SDFF/") + fontData.FileName + ".png");
		auto tResult = context.DX12M->ConstantTextureManager.GetTexture2DFromFile(context.Device, context.CommandList,
			&CBV_SRV_UAV_DescriptorHeap, imageFileName,
			DirectX12Render::TextureMipmapGenerationMode::Pow2Box);
		if (tResult.IsTextureCreated)
		{
			tResult.Texture->TransitionToPixelShaderResource(context.CommandList);
		}
		fontData.Texture = tResult.Texture;
		fontData.TextureGPUDescriptorHandle = tResult.Texture->GetShaderResourceView().GPUDescriptorHandle;
	}
}

void TextRenderer::CreateSymbolInstaceBuffer(const UtilityInitializationContext& context)
{
	auto& font = m_Fonts[0];
	auto& glyphs = font.Font.Glyphs;

	auto device = context.Device;
	unsigned frameCount = context.FrameCount;

	const VertexElement elements[] =
	{
		{ "PositionMultiplier", VertexElementType::Float, sizeof(float), 2 },
		{ "PositionOffset", VertexElementType::Float, sizeof(float), 2 },
		{ "PositionInTex", VertexElementType::Float, sizeof(float), 2 },
		{ "SizeInTex", VertexElementType::Float, sizeof(float), 2 }
	};
	EngineBuildingBlocks::Graphics::VertexInputLayout inputLayout;
	inputLayout.Elements.insert(inputLayout.Elements.end(), elements, elements + 4);

	for (unsigned i = 0; i < frameCount; i++)
	{
		auto& frameResource = m_FrameResources[i];
		auto& symbolBuffer = frameResource.SymbolInstanceBuffer;
		symbolBuffer.Initialize(context.Device, inputLayout, D3D12_HEAP_TYPE_UPLOAD, c_MaxCountSymbols, 1, false);
	}

	auto& vertexIL = m_Primitive.PVertexBuffer->GetInputLayout().InputLayout;
	const EngineBuildingBlocks::Graphics::VertexInputLayout* inputLayouts[]
		= { &vertexIL, &inputLayout };
	auto mergedInputLayout = EngineBuildingBlocks::Graphics::VertexInputLayout::Merge(inputLayouts, 2);
	Core::IndexVector inputSlots;
	Core::SimpleTypeVector<bool> isPerVertex;
	inputSlots.PushBack(0U, vertexIL.Elements.size());
	inputSlots.PushBack(1, inputLayout.Elements.size());
	isPerVertex.PushBack(true, vertexIL.Elements.size());
	isPerVertex.PushBack(false, inputLayout.Elements.size());
	m_MergedIL.Initialize(mergedInputLayout, inputSlots.GetArray(), isPerVertex.GetArray());
}

void TextRenderer::Render(const UtilityRenderContext& context)
{
	auto commandList = context.CommandList;

	// Using first font data.
	auto& fontData = m_Fonts[0];

	commandList->SetPipelineState(m_PipelineStateObject->GetPipelineStateObject());
	commandList->SetGraphicsRootSignature(m_RootSignature->GetRootSignature());
	
	commandList->SetGraphicsRootDescriptorTable((int)RootParameters::TextureSRVDescriptorTable, fontData.TextureGPUDescriptorHandle);

	auto& primitive = m_Primitive;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] =
	{
		primitive.PVertexBuffer->GetVertexBufferView(),
		m_FrameResources[context.FrameIndex].SymbolInstanceBuffer.GetVertexBufferView()
	};

	commandList->IASetPrimitiveTopology(primitive.PIndexBuffer->GetTopology());
	commandList->IASetVertexBuffers(0, 2, vertexBufferViews);
	commandList->IASetIndexBuffer(&primitive.PIndexBuffer->GetIndexBufferView());

	unsigned instanceIndex = 0;
	auto it = m_Tasks.GetBeginIterator();
	auto end = m_Tasks.GetEndIterator();
	for (; it != end; ++it)
	{
		auto countInstances = it->Symbols.GetSize();
		commandList->SetGraphicsRoot32BitConstants((int)RootParameters::DepthRC, 1, &it->Depth, 0);
		commandList->SetGraphicsRoot32BitConstants((int)RootParameters::ColorRC, 4, glm::value_ptr(it->Color), 0);
		commandList->DrawIndexedInstanced(primitive.CountIndices, countInstances, primitive.BaseIndex,
			primitive.BaseVertex, instanceIndex);
		instanceIndex += countInstances;
	}
}

void TextRenderer::Update(const UtilityUpdateContext& context)
{
	auto& frameResources = m_FrameResources[context.FrameIndex];
	auto& symbolData = frameResources.SymbolData;

	if(frameResources.IsDirty)
	{
		// Updating symbol data.
		unsigned totalSymbolCount = 0;
		auto it = m_Tasks.GetBeginIterator();
		auto end = m_Tasks.GetEndIterator();
		for (; it != end; ++it)
		{
			totalSymbolCount += it->Symbols.GetSize();
		}

		const auto symbolSize = sizeof(SymbolData);
		symbolData.Resize(totalSymbolCount * symbolSize);

		it = m_Tasks.GetBeginIterator();
		auto target = symbolData.GetArray();
		for (; it != end; ++it)
		{
			auto& symbols = it->Symbols;	
			unsigned currentSize = symbols.GetSize() * symbolSize;
			memcpy(target, symbols.GetArray(), currentSize);
			target += currentSize;
		}

		// Updating symbol instance buffer.
		// Do we need to synchronize the vertex buffer's content here?
		auto& symbolInstanceBuffer = frameResources.SymbolInstanceBuffer;
		symbolInstanceBuffer.SetData(symbolData.GetArray(), symbolData.GetSize());

		frameResources.IsDirty = false;
	}
}

void TextRenderer::SetDirty()
{
	for (size_t i = 0; i < m_FrameResources.size(); i++)
	{
		m_FrameResources[i].IsDirty = true;
	}
}

unsigned TextRenderer::AddRenderTask(const TextRenderTask& task)
{
	auto& font = m_Fonts[0];
	auto& glyphs = font.Font.Glyphs;
	unsigned textureWidth = font.Texture->GetDescription().Width;
	unsigned textureHeight = font.Texture->GetDescription().Height;

	auto& text = task.Text;

	// Scaling position and line height to [-1, 1].
	auto position = task.Position * 2.0f - 1.0f;
	float lineHeight = task.LineHeight * 2.0f;
	float depth = task.Depth;

	float sizeMultiplier = lineHeight / font.Font.MaximumHeight;

	float spacedLineHeight = lineHeight * 1.2f;
	float spaceWidth = lineHeight * 0.5f;
	float tabWidth = spaceWidth * 4;

	auto index = m_Tasks.Add(TextRenderTaskData());
	auto& data = m_Tasks[index];

	auto currentPosition = position;

	auto length = static_cast<unsigned>(text.length());
	for (unsigned i = 0; i < length; i++)
	{
		auto ch = static_cast<unsigned>(text[i]);
		if (IsPrintableCharacter(ch))
		{
			auto& glyph = glyphs[ch];

			auto size = glm::vec2(glyph.Width, glyph.Height) * sizeMultiplier;
			auto offsetToCurrentPos = glm::vec2(glyph.XOffset, glyph.YOffset - glyph.Height) * sizeMultiplier;
			float xTotalSize = size.x + offsetToCurrentPos.x;
			float xAdvance = size.x + offsetToCurrentPos.x;

			PreparePositionForCharacter(currentPosition, xTotalSize, spacedLineHeight, position.x);

			auto& symbolData = data.Symbols.PushBackPlaceHolder();

			// Symbol data.
			symbolData.PositionMultiplier = size;
			symbolData.PositionOffset = currentPosition + offsetToCurrentPos;

			auto x = glyph.X / (float)textureWidth;
			auto y = 1.0f - (glyph.Y + glyph.Height) / (float)textureHeight;

			auto w = glyph.Width / (float)textureWidth;
			auto h = glyph.Height / (float)textureHeight;

			// Glyph data.
			symbolData.PositionInTex = glm::vec2(x, y);
			symbolData.SizeInTex = glm::vec2(w, h);

			IncrementPosition(currentPosition, xAdvance);
		}
		else if (ch == ' ')
		{
			if (PreparePositionForCharacter(currentPosition, spaceWidth, spacedLineHeight, position.x))
			{
				IncrementPosition(currentPosition, spaceWidth);
			}
		}
		else if (ch == '\t')
		{
			PreparePositionForCharacter(currentPosition, tabWidth, spacedLineHeight, position.x);
			IncrementPosition(currentPosition, tabWidth);
		}
		else if (ch == '\n')
		{
			PreparePositionForCharacter(currentPosition, 2.0f, spacedLineHeight, position.x);
		}
		else
		{
			RaiseException("Unhandled character in font rendering.");
		}
	}

	if (data.Symbols.GetSize() == 0) // Only whitespaces.
	{
		m_Tasks.Remove(index);
		return Core::c_InvalidIndexU;
	}

	data.Depth = task.Depth;
	data.Color = task.Color;

	SetDirty();

	return index;
}

void TextRenderer::RemoveRenderTask(unsigned index)
{
	m_Tasks.Remove(index);
	SetDirty();
}

void TextRenderer::ClearRenderTasks()
{
	if (m_Tasks.GetSize() > 0)
	{
		m_Tasks.Clear();
		SetDirty();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

UpdatableTextRenderTask::UpdatableTextRenderTask()
	: m_Handle(Core::c_InvalidIndexU)
	, m_IsDirty(false)
{
	m_Task.Color = c_TextRenderTask_DefaultColor;
	m_Task.Depth = c_TextRenderTask_DefaultDepth;
	m_Task.LineHeight = c_TextRenderTask_DefaultLineHeight;
	m_Task.Position = c_TextRenderTask_DefaultPosition;
}

void UpdatableTextRenderTask::Update(TextRenderer& textRenderer)
{
	if (m_IsDirty)
	{
		if(m_Handle != Core::c_InvalidIndexU) textRenderer.RemoveRenderTask(m_Handle);
		m_Handle = textRenderer.AddRenderTask(m_Task);
		m_IsDirty = false;
	}
}

const char* UpdatableTextRenderTask::GetText() const
{
	return m_Task.Text.c_str();
}

void UpdatableTextRenderTask::SetText(const char* text)
{
	if (m_Task.Text != text){ m_Task.Text = text; m_IsDirty = true; }
}

const glm::vec2& UpdatableTextRenderTask::GetPosition() const
{
	return m_Task.Position;
}

void UpdatableTextRenderTask::SetPosition(const glm::vec2& position)
{
	if (m_Task.Position != position) { m_Task.Position = position; m_IsDirty = true; }
}

void UpdatableTextRenderTask::SetPosition(TextPositionConstant textPosition)
{
	switch (textPosition)
	{
	case TextPositionConstant::BottomLeft: SetPosition(glm::vec2(0.0f, 0.0f)); break;
	case TextPositionConstant::TopLeft: SetPosition(glm::vec2(0.0f, 1.0f - m_Task.LineHeight)); break;
	}
}

float UpdatableTextRenderTask::GetDepth() const
{
	return m_Task.Depth;
}

void UpdatableTextRenderTask::SetDepth(float depth)
{
	if (m_Task.Depth != depth) { m_Task.Depth = depth; m_IsDirty = true; }
}

float UpdatableTextRenderTask::GetLineHeight() const
{
	return m_Task.LineHeight;
}

void UpdatableTextRenderTask::SetLineHeight(float lineHeight)
{
	if (m_Task.LineHeight != lineHeight) { m_Task.LineHeight = lineHeight; m_IsDirty = true; }
}

const glm::vec4& UpdatableTextRenderTask::GetColor() const
{
	return m_Task.Color;
}

void UpdatableTextRenderTask::SetColor(const glm::vec4& color)
{
	if (m_Task.Color != color) { m_Task.Color = color; m_IsDirty = true; }
}