// DirectX11Render/Resources/VertexBuffer.h

#ifndef _DIRECTX11RENDER_VERTEXBUFFER_H_INCLUDED_
#define _DIRECTX11RENDER_VERTEXBUFFER_H_INCLUDED_

#include <Core/DataStructures/Pool.hpp>
#include <EngineBuildingBlocks/Graphics/Primitives/Primitive.h>

#include <DirectX11Render/Resources/Shader.h>

#include <map>

namespace DirectX11Render
{
	struct VertexInputLayout
	{
		EngineBuildingBlocks::Graphics::VertexInputLayout InputLayout;
		Core::SimpleTypeVectorU<D3D11_INPUT_ELEMENT_DESC> Elements;

		static VertexInputLayout Create(
			const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
			const unsigned inputSlot = 0, const bool isPerVertexArray = true);

		static VertexInputLayout Create(
			const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
			const unsigned* inputSlots, const bool* isPerVertexArray);

		bool operator==(const VertexInputLayout& other) const;
		bool operator!=(const VertexInputLayout& other) const;
		bool operator<(const VertexInputLayout& other) const;
	};

	class VertexBuffer
	{
		ComPtr<ID3D11Buffer> m_Buffer;

		unsigned m_CountVertices;
		EngineBuildingBlocks::Graphics::VertexInputLayout m_InputLayout;

		void InitializeResource(ID3D11Device* device, D3D11_USAGE usage, const void* pData);

	public:

		VertexBuffer();

		unsigned GetSizeInBytes() const;
		unsigned GetCountVertices() const;
		unsigned GetVertexStride() const;

		const EngineBuildingBlocks::Graphics::VertexInputLayout& GetInputLayout() const;

		ID3D11Buffer* GetBuffer();

		void Initialize(ID3D11Device* device,
			const EngineBuildingBlocks::Graphics::Vertex_AOS_Data& aosData);
		void Initialize(ID3D11Device* device,
			const EngineBuildingBlocks::Graphics::Vertex_SOA_Data& soaData);
		void Initialize(ID3D11Device* device, D3D11_USAGE usage,
			const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout, unsigned countVertices);
	
		void SetData(ID3D11DeviceContext* context, const void* pData, size_t size);
	};
}

#endif