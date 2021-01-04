// DirectX12Render/Utilities/TextRenderer.h

#ifndef _DIRECTX12RENDER_TEXTRENDERER_H_INCLUDED_
#define _DIRECTX12RENDER_TEXTRENDERER_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <Core/DataStructures/ResourceUnorderedVector.hpp>
#include <EngineBuildingBlocks/Math/GLM.h>
#include <DirectX12Render/Primitive.h>
#include <DirectX12Render/Resources/ConstantBuffer.h>
#include <DirectX12Render/Resources/VertexBuffer.h>
#include <DirectX12Render/Utilities/Utility.h>

#include <vector>

namespace EngineBuildingBlocks
{
	class PathHandler;
	namespace Graphics
	{
		class VertexInputLayoutManager;
	}
}

namespace DirectX12Render
{
	class PrimitiveManager;
	class TransferBufferManager;
	class RootSignature;
	class RootSignatureManager;
	class GraphicsPipelineStateObject;
	class PipelineStateObjectManager;
	class ShaderManager;
	class ConstantTextureManager;
	class StreamedDescriptorHeap;
	class Texture2D;

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
		D3D12_GPU_DESCRIPTOR_HANDLE TextureGPUDescriptorHandle;
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

	struct TextRenderFrameResources
	{
		VertexBuffer SymbolInstanceBuffer;
		Core::ByteVectorU SymbolData;
		bool IsDirty;

		TextRenderFrameResources();
	};

	class TextRenderer
	{
		IndexedPrimitive m_Primitive;
		DirectX12Render::VertexInputLayout m_MergedIL;
		RootSignature* m_RootSignature;
		GraphicsPipelineStateObject* m_PipelineStateObject;

		std::vector<FontData> m_Fonts;
		std::vector<TextRenderFrameResources> m_FrameResources;

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
		void CreateRootSignature(const UtilityInitializationContext& context);
		void CreatePipelineStateObject(const UtilityInitializationContext& context);
		void CreateTextures(const UtilityInitializationContext& context,
			StreamedDescriptorHeap& CBV_SRV_UAV_DescriptorHeap);
		void CreateSymbolInstaceBuffer(const UtilityInitializationContext& context);

	public:

		TextRenderer();

		void Initialize(const UtilityInitializationContext& context,
			StreamedDescriptorHeap& CBV_SRV_UAV_DescriptorHeap);
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