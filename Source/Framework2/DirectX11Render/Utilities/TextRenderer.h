// DirectX11Render/Utilities/TextRenderer.h

#ifndef _DIRECTX11RENDER_TEXTRENDERER_H_INCLUDED_
#define _DIRECTX11RENDER_TEXTRENDERER_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <Core/DataStructures/ResourceUnorderedVector.hpp>
#include <EngineBuildingBlocks/Math/GLM.h>
#include <DirectX11Render/Primitive.h>
#include <DirectX11Render/Resources/ConstantBuffer.h>
#include <DirectX11Render/Resources/VertexBuffer.h>
#include <DirectX11Render/Utilities/Utility.h>

#include <vector>

namespace DirectX11Render
{
	struct Glyph
	{
		int ID;
		int Width, Height;
		int X, Y;
		float XOffset, YOffset;
		float XAdvance;
	};

	struct Font
	{
		std::string Name;
		Core::SimpleTypeVectorU<Glyph> Glyphs;
		int MaximumHeight;
	};

	struct FontData
	{
		Font Font;
		std::string FileName;
		Texture2D* Texture;
	};

	struct TextRenderTask
	{
		std::string Text;
		glm::vec2 Position;
		float LineHeight;
		float Depth;
		glm::vec4 Color;
	};

	struct SymbolData
	{
		// Symbol data.
		glm::vec2 PositionMultiplier;
		glm::vec2 PositionOffset;

		// Glyph data.
		glm::vec2 PositionInTex;
		glm::vec2 SizeInTex;
	};

	const unsigned c_MaxCountSymbols = 64 * 1024;
	const unsigned c_MaxCountTasks = 1024;

	struct TextRenderTaskData
	{
		Core::SimpleTypeVectorU<SymbolData> Symbols;
		float Depth;
		glm::vec4 Color;
	};

	class TextRenderer
	{
		IndexedPrimitive m_Primitive;
		DirectX11Render::VertexInputLayout m_MergedIL;
		unsigned m_PipelineStateIndex;
		ConstantBuffer m_ConstantBuffer;

		std::vector<FontData> m_Fonts;
		
		VertexBuffer m_SymbolInstanceBuffer;
		Core::ByteVectorU m_SymbolData;
		bool m_IsDirty;

	private: // Task management.

		Core::ResourceUnorderedVectorU<TextRenderTaskData> m_Tasks;

		void SetDirty();

	public:

		unsigned AddRenderTask(const TextRenderTask& task);
		void RemoveRenderTask(unsigned index);
		void ClearRenderTasks();

	private: // Initialization.

		void LoadFonts(const UtilityInitializationContext& context);

		void CreatePrimitive(const UtilityInitializationContext& context);
		void CreatePipelineState(const UtilityInitializationContext& context);
		void CreateTextures(const UtilityInitializationContext& context);
		void CreateSymbolInstaceBuffer(const UtilityInitializationContext& context);
		void CreateConstantBuffer(const UtilityInitializationContext& context);

	public:

		TextRenderer();

		void Initialize(const UtilityInitializationContext& context);
		void Render(const UtilityRenderContext& context);
		void Update(const UtilityUpdateContext& context);
	};

	const glm::vec4 c_TextRenderTask_DefaultColor = glm::vec4(1.0f);
	const float c_TextRenderTask_DefaultDepth = 0.000001f;
	const float c_TextRenderTask_DefaultLineHeight = 0.05f;
	const glm::vec2 c_TextRenderTask_DefaultPosition = glm::vec2(0.0f);

	enum class TextPositionConstant
	{
		BottomLeft, TopLeft
	};

	class UpdatableTextRenderTask
	{
		unsigned m_Handle;
		TextRenderTask m_Task;
		bool m_IsDirty;

	public:

		UpdatableTextRenderTask();

		void Update(TextRenderer& textRenderer);

		const char* GetText() const;
		void SetText(const char* text);

		const glm::vec2& GetPosition() const;
		void SetPosition(const glm::vec2& position);
		void SetPosition(TextPositionConstant textPosition);

		float GetDepth() const;
		void SetDepth(float depth);

		float GetLineHeight() const;
		void SetLineHeight(float lineHeight);

		const glm::vec4& GetColor() const;
		void SetColor(const glm::vec4& color);
	};
}

#endif