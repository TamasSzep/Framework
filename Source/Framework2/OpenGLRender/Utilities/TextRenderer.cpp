// OpenGLRender/Utilities/TextRenderer.cpp

#include <OpenGLRender/Utilities/TextRenderer.h>

#include <Core/String.hpp>
#include <Core/Parse.hpp>
#include <Core/System/SimpleIO.h>
#include <Core/Constants.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <OpenGLRender/Resources/Shader.h>
#include <OpenGLRender/Resources/ConstantTextureManager.h>
#include <OpenGLRender/Resources/Texture2D.h>
#include <OpenGLRender/Resources/IndexBuffer.h>

#include <filesystem>
#include <algorithm>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace OpenGLRender;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void LoadFont(const std::string& fileName, OpenGLRender::Font& font)
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

	CreateShader(context);
	CreatePrimitive(context);
	CreateSymbolInstaceBuffer(context);
	SetInputLayout();
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

void TextRenderer::CreateShader(const UtilityInitializationContext& context)
{
	ShaderProgramDescription spd;
	spd.Shaders = {
		ShaderDescription::FromFile(*context.PathHandler, "GLSL/TextRender_vs.glsl", ShaderType::Vertex),
		ShaderDescription::FromFile(*context.PathHandler, "GLSL/TextRender_ps.glsl", ShaderType::Fragment)
	};
	m_Program = context.OGLM->ShaderManager.GetShaderProgram(spd);
}

void TextRenderer::CreatePrimitive(const UtilityInitializationContext& context)
{
	m_Primitive = context.OGLM->PrimitiveManager.GetIndexedPrimitive(QuadDescription(PrimitiveRange::_0_To_Plus1));
}

void TextRenderer::CreateTextures(const UtilityInitializationContext& context)
{
	TextureSamplingDescription textureSampling;
	textureSampling.MaximumAnisotropy = 1.0f;

	unsigned countFonts = static_cast<unsigned>(m_Fonts.size());
	for (unsigned i = 0; i < countFonts; i++)
	{
		auto& fontData = m_Fonts[i];
		auto imageFileName = context.PathHandler->GetPathFromResourcesDirectory(
			std::string("Fonts/SDFF/") + fontData.FileName + ".png");
		fontData.Texture = context.OGLM->ConstantTextureManager.GetTexture2DFromFile(
			imageFileName, TextureMipmapGenerationMode::Pow2Box, &textureSampling).Texture;
	}
}

void TextRenderer::CreateSymbolInstaceBuffer(const UtilityInitializationContext& context)
{
	auto& font = m_Fonts[0];
	auto& glyphs = font.Font.Glyphs;

	const VertexElement elements[] =
	{
		{ "PositionMultiplier", VertexElementType::Float, sizeof(float), 2 },
		{ "PositionOffset", VertexElementType::Float, sizeof(float), 2 },
		{ "PositionInTex", VertexElementType::Float, sizeof(float), 2 },
		{ "SizeInTex", VertexElementType::Float, sizeof(float), 2 }
	};
	m_InstanceIL.Elements.insert(m_InstanceIL.Elements.end(), elements, elements + 4);

	m_SymbolInstanceBuffer.Initialize(BufferUsage::DynamicDraw, c_MaxCountSymbols, m_InstanceIL);
}

void TextRenderer::SetInputLayout()
{
	m_Program->Bind();

	m_Primitive.PVertexArrayObject->Bind();

	// Setting per vertex attributes.
	m_Primitive.PVertexBuffer->Bind();
	m_Program->TrySetInputLayoutElements(*m_Primitive.PInputLayout);

	// Setting per instance attributes.
	m_SymbolInstanceBuffer.Bind();
	m_Program->SetInputLayout(m_InstanceIL, 1);
}

void TextRenderer::CreateConstantBuffer(const UtilityInitializationContext& context)
{
	m_ConstantBuffer.Initialize(BufferUsage::StaticDraw, sizeof(DX11TextRenderCBType));
}

inline void OGL_TR_UpdateConstantBuffer(UniformBuffer* cb, const TextRenderTaskData& rtData)
{
	DX11TextRenderCBType data;
	data.Color = rtData.Color;
	data.Depth = rtData.Depth;
	cb->Write(&data, sizeof(DX11TextRenderCBType));
}

inline void OGL_TR_Draw(const IndexedPrimitive& primitive, unsigned instanceIndex, unsigned countInstances)
{
	glDrawElementsInstancedBaseVertexBaseInstance(c_PrimitiveTopologyMap[(int)primitive.Topology],
		primitive.CountIndices, GL_UNSIGNED_INT, (const void*)(size_t)(primitive.BaseIndex << 2),
		countInstances, primitive.BaseVertex, instanceIndex);
}

void TextRenderer::Render(const UtilityRenderContext& context)
{
	// Getting previous pipeline state.
	GLint cullingEnabled, depthMask, blendingEnabled;
	glGetIntegerv(GL_CULL_FACE, &cullingEnabled);
	glGetIntegerv(GL_DEPTH_WRITEMASK, &depthMask);
	glGetIntegerv(GL_BLEND, &blendingEnabled);

	// Setting pipeline state.
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	m_Program->Bind();

	// Using first font data.
	auto& fontData = m_Fonts[0];
	fontData.Texture->Bind(0);
	m_Program->SetUniformValue("DistanceTexture", 0);

	m_Primitive.PVertexArrayObject->Bind();

	m_ConstantBuffer.Bind(0);

	unsigned instanceIndex = 0;
	auto it = m_Tasks.GetBeginIterator();
	auto end = m_Tasks.GetEndIterator();
	for (; it != end; ++it)
	{
		auto countInstances = it->Symbols.GetSize();

		OGL_TR_UpdateConstantBuffer(&m_ConstantBuffer, *it);
		OGL_TR_Draw(m_Primitive, instanceIndex, countInstances);

		instanceIndex += countInstances;
	}

	// Resetting pipeline state.
	if (cullingEnabled) glEnable(GL_CULL_FACE);
	if (depthMask) glDepthMask(GL_TRUE);
	if (!blendingEnabled) glDisable(GL_BLEND);
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
		if (m_SymbolData.GetSize() > 0)
		{
			m_SymbolInstanceBuffer.Write(m_SymbolData.GetArray(), m_SymbolData.GetSize());
		}

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
	unsigned textureWidth = font.Texture->GetTextureDescription().Width;
	unsigned textureHeight = font.Texture->GetTextureDescription().Height;

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