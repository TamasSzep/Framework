// DirectX11Render/Resources/IndexBuffer.h

#ifndef _DIRECTX11RENDER_INDEXBUFFER_H_INCLUDED_
#define _DIRECTX11RENDER_INDEXBUFFER_H_INCLUDED_

#include <EngineBuildingBlocks/Graphics/Primitives/Primitive.h>
#include <DirectX11Render/DirectX11Includes.h>

namespace DirectX11Render
{
	class IndexBuffer
	{
		ComPtr<ID3D11Buffer> m_Buffer;

		unsigned m_CountIndices;
		EngineBuildingBlocks::Graphics::PrimitiveTopology m_Topology;

		void InitializeResource(ID3D11Device* device, D3D11_USAGE usage, const unsigned* pIndices);

	public:

		IndexBuffer();

		unsigned GetSizeInBytes() const;
		unsigned GetCountIndices() const;

		EngineBuildingBlocks::Graphics::PrimitiveTopology GetTopology() const;

		ID3D11Buffer* GetBuffer();

		void Initialize(ID3D11Device* device,
			const EngineBuildingBlocks::Graphics::IndexData& indexData);
		void Initialize(ID3D11Device* device, D3D11_USAGE usage, 
			EngineBuildingBlocks::Graphics::PrimitiveTopology topology,
			unsigned countIndices, const unsigned* pIndices = nullptr);
	
		void SetData(ID3D11DeviceContext* context, const unsigned* pIndices);
		void SetData(ID3D11DeviceContext* context, const unsigned* pIndices,
			unsigned startIndex, unsigned countIndices);
	};
}

#endif