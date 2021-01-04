// DirectX11Render/Utilities/TextRenderer.cpp

#include <DirectX11Render/Utilities/TextRenderer.h>

#include <Core/String.hpp>
#include <Core/Parse.hpp>
#include <Core/System/SimpleIO.h>
#include <Core/Constants.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <DirectX11Render/Resources/Shader.h>
#include <DirectX11Render/Resources/ConstantTextureManager.h>
#include <DirectX11Render/Resources/Texture2D.h>
#include <DirectX11Render/Resources/IndexBuffer.h>

#include <filesystem>
#include <algorithm>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace DirectXRender;
using namespace DirectX11Render;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void LoadFont(const std::string& fileName, DirectX11Render::Font& font)
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

struct DX11TextRenderCBType
{
	glm::vec4 Color;
	float Depth;
	glm::vec3 _Padding0;
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

TextRenderer::TextRenderer()
	: m_IsDirty(true)
{
}

void TextRenderer::Initialize(const UtilityInitializationContext& context)
{
	LoadFonts(context);

	CreatePrimitive(context);
	CreateSymbolInstaceBuffer(context);
	CreatePipelineState(context);
	CreateTextures(context);
	CreateConstantBuffer(context);
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
	m_Primitive = context.DX11M->PrimitiveManager.GetIndexedPrimitive(
			QuadDescription(PrimitiveRange::_0_To_Plus1), context.Device);
}

void TextRenderer::CreatePipelineState(const UtilityInitializationContext& context)
{
	auto vs = context.DX11M->ShaderManager.GetShaderSimple(context.Device,
	{ "TextRender.hlsl", "SDF_VS_Main", ShaderType::Vertex });
	auto ps = context.DX11M->ShaderManager.GetShaderSimple(context.Device,
	{ "TextRender.hlsl", "SDF_PS_Main", ShaderType::Pixel });

	SamplerStateDescription ssd;
	ssd.SetToTrilinear();

	PipelineStateDescription description;
	description.Shaders = { vs, ps };
	description.InputLayout = m_MergedIL;
	description.RasterizerState.DisableCulling();
	description.DepthStencilState.DisableDepthWriting();
	description.BlendState.SetToNonpremultipliedAlphaBlending();
	description.SamplerStates = { { ssd, ShaderFlagType::Pixel } };

	m_PipelineStateIndex = context.DX11M->PipelineStateManager.GetPipelineStateIndex(context.Device, description);
}

void TextRenderer::CreateTextures(const UtilityInitializationContext& context)
{
	unsigned countFonts = static_cast<unsigned>(m_Fonts.size());
	for (unsigned i = 0; i < countFonts; i++)
	{
		auto& fontData = m_Fonts[i];
		auto imageFileName = context.PathHandler->GetPathFromResourcesDirectory(
			std::string("Fonts/SDFF/") + fontData.FileName + ".png");
		fontData.Texture = context.DX11M->ConstantTextureManager.GetTexture2DFromFile(context.Device,
			imageFileName, TextureMipmapGenerationMode::Pow2Box).Texture;
	}
}

void TextRenderer::CreateSymbolInstaceBuffer(const UtilityInitializationContext& context)
{
	auto& font = m_Fonts[0];
	auto& glyphs = font.Font.Glyphs;

	auto device = context.Device;

	const VertexElement elements[] =
	{
		{ "PositionMultiplier", VertexElementType::Float, sizeof(float), 2 },
		{ "PositionOffset", VertexElementType::Float, sizeof(float), 2 },
		{ "PositionInTex", VertexElementType::Float, sizeof(float), 2 },
		{ "SizeInTex", VertexElementType::Float, sizeof(float), 2 }
	};
	EngineBuildingBlocks::Graphics::VertexInputLayout inputLayout;
	inputLayout.Elements.insert(inputLayout.Elements.end(), elements, elements + 4);

	m_SymbolInstanceBuffer.Initialize(context.Device, D3D11_USAGE_DYNAMIC, inputLayout, c_MaxCountSymbols);

	auto& vertexIL = m_Primitive.PVertexBuffer->GetInputLayout();
	const EngineBuildingBlocks::Graphics::VertexInputLayout* inputLayouts[]
		= { &vertexIL, &inputLayout };
	auto mergedInputLayout = EngineBuildingBlocks::Graphics::VertexInputLayout::Merge(inputLayouts, 2);
	Core::IndexVector inputSlots;
	Core::SimpleTypeVector<bool> isPerVertex;
	inputSlots.PushBack(0U, vertexIL.Elements.size());
	inputSlots.PushBack(1, inputLayout.Elements.size());
	isPerVertex.PushBack(true, vertexIL.Elements.size());
	isPerVertex.PushBack(false, inputLayout.Elements.size());
	m_MergedIL = DirectX11Render::VertexInputLayout::Create(mergedInputLayout,
		inputSlots.GetArray(), isPerVertex.GetArray());
}

void TextRenderer::CreateConstantBuffer(const UtilityInitializationContext& context)
{
	m_ConstantBuffer.Initialize(context.Device, sizeof(DX11TextRenderCBType), c_MaxCountTasks);
}

void TextRenderer::Render(const UtilityRenderContext& context)
{
	auto d3dContext = context.Context;

	context.DX11M->PipelineStateManager.GetPipelineState(m_PipelineStateIndex).SetForContext(d3dContext);

	// Using first font data.
	auto& fontData = m_Fonts[0];
	auto srv = fontData.Texture->GetSRV();
	d3dContext->PSSetShaderResources(0, 1, &srv);

	ID3D11Buffer* vbs[] ={ m_Primitive.PVertexBuffer->GetBuffer(), m_SymbolInstanceBuffer.GetBuffer() };
	unsigned vbStrides[] = { m_Primitive.PVertexBuffer->GetVertexStride(), m_SymbolInstanceBuffer.GetVertexStride() };
	unsigned vbOffsets[] = { 0U, 0U };
	d3dContext->IASetVertexBuffers(0, 2, vbs, vbStrides, vbOffsets);
	d3dContext->IASetIndexBuffer(m_Primitive.PIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
	d3dContext->IASetPrimitiveTopology(c_PrimitiveTopologyMap[(int)m_Primitive.PIndexBuffer->GetTopology()]);

	unsigned instanceIndex = 0;
	auto it = m_Tasks.GetBeginIterator();
	auto end = m_Tasks.GetEndIterator();
	for (; it != end; ++it)
	{
		auto countInstances = it->Symbols.GetSize();
		
		m_ConstantBuffer.Update(d3dContext, m_Tasks.ToIndex(it));
		auto cb = m_ConstantBuffer.GetBuffer();
		d3dContext->VSSetConstantBuffers(0, 1, &cb);
		d3dContext->PSSetConstantBuffers(0, 1, &cb);

		d3dContext->DrawIndexedInstanced(m_Primitive.CountIndices, countInstances, m_Primitive.BaseIndex,
			m_Primitive.BaseVertex, instanceIndex);
		instanceIndex += countInstances;
	}
}

void TextRenderer::Update(const UtilityUpdateContext& context)
{
	if(m_IsDirty)
	{
		// Updating task and symbol data.
		unsigned totalSymbolCount = 0;
		auto it = m_Tasks.GetBeginIterator();
		auto end = m_Tasks.GetEndIterator();
		for (; it != end; ++it)
		{
			totalSymbolCount += it->Symbols.GetSize();

			auto& cbData = m_ConstantBuffer.Access<DX11TextRenderCBType>(m_Tasks.ToIndex(it));
			cbData.Color = it->Color;
			cbData.Depth = it->Depth;
		}

		const auto symbolSize = sizeof(SymbolData);
		m_SymbolData.Resize(totalSymbolCount * symbolSize);

		it = m_Tasks.GetBeginIterator();
		auto target = m_SymbolData.GetArray();
		for (; it != end; ++it)
		{
			auto& symbols = it->Symbols;	
			unsigned currentSize = symbols.GetSize() * symbolSize;
			memcpy(target, symbols.GetArray(), currentSize);
			target += currentSize;
		}

		// Updating symbol instance buffer.
		m_SymbolInstanceBuffer.SetData(context.Context, m_SymbolData.GetArray(), m_SymbolData.GetSize());

		m_IsDirty = false;
	}
}

void TextRenderer::SetDirty()
{
	m_IsDirty = true;
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