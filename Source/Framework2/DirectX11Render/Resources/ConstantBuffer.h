// DirectX11Render/Resources/ConstantBuffer.h

#ifndef _DIRECTX11RENDER_CONSTANTBUFFER_H_INCLUDED_
#define _DIRECTX11RENDER_CONSTANTBUFFER_H_INCLUDED_

#include <Core/AlignedAllocator.hpp>
#include <DirectX11Render/DirectX11Includes.h>

namespace DirectX11Render
{
	class ConstantBuffer
	{
		ComPtr<ID3D11Buffer> m_Buffer;
		std::vector<unsigned char, Core::AlignedAllocator<unsigned char,
			Core::Alignment::_16Byte>> m_Data;

		unsigned m_ElementSize;

		void InitializeResource(ID3D11Device* device);

	public:

		ConstantBuffer();

		unsigned GetCPUSizeInBytes() const;
		unsigned GetGPUSizeInBytes() const;

		unsigned GetCountElements() const;
		unsigned GetElementSize() const;

		ID3D11Buffer* GetBuffer();

		void Initialize(ID3D11Device* device, unsigned elementSize,
			unsigned maxCountElements);

		void Update(ID3D11DeviceContext* context, unsigned index = 0);

		template <typename DataType>
		DataType& Access(unsigned index = 0)
		{
			return *reinterpret_cast<DataType*>(m_Data.data()
				+ static_cast<size_t>(index * m_ElementSize));
		}
	};
}

#endif