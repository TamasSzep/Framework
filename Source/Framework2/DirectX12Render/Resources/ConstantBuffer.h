// DirectX12Render/Resources/ConstantBuffer.h

#ifndef _DIRECTX12RENDER_CONSTANTBUFFER_H_INCLUDED_
#define _DIRECTX12RENDER_CONSTANTBUFFER_H_INCLUDED_

#include <DirectX12Render/DirectX12Includes.h>

#include <Core/DataStructures/SimpleTypeVector.hpp>

#include <limits>

namespace DirectX12Render
{
	class IDescriptorHeap;

	enum class ConstantBufferMappingType
	{
		AlwaysMapped, OnWriteMapped
	};

	class ConstantBuffer
	{
		ComPtr<ID3D12Resource> m_Resource;
		unsigned char* m_DirectDataPointer;

		ConstantBufferMappingType m_MappingType;

		unsigned m_DataSize;
		unsigned m_AlignedDataSize;
		unsigned m_MaximumViews;
		unsigned m_CountViews;

		D3D12_RANGE m_WrittenRange;

		unsigned m_DescriptorSize;
		Core::SimpleTypeVectorU<D3D12_GPU_DESCRIPTOR_HANDLE> m_GPUDescriptorHandles;

		static const size_t c_InvalidRangeOffset = std::numeric_limits<size_t>::max();

		void Map();
		void Unmap();

		void CreateConstantBufferView(ID3D12Device* device,
			IDescriptorHeap* pDescriptorHeap);

	public:

		ConstantBuffer(
			ConstantBufferMappingType mappingType,
			unsigned dataSize, unsigned maximumViews);
		~ConstantBuffer();

		unsigned GetSize() const;

		void Initialize(ID3D12Device* device);

		void StartWrite();
		void EndWrite();

		void IndicateWrittenRangeWithViewIndex(unsigned startIndex, unsigned endIndex);

		template <typename DataType>
		DataType& Access(unsigned index)
		{
			return *reinterpret_cast<DataType*>(m_DirectDataPointer
				+ static_cast<size_t>(index * m_AlignedDataSize));
		}

		// Adds a constant buffer and returns its index.
		unsigned AddConstantBuffer();

		// Adds a constant buffer and returns its index. It also allocates a constant buffer view
		// on a descriptor managed by the descriptor heap handler.
		unsigned AddConstantBuffer(ID3D12Device* device,
			IDescriptorHeap* pDescriptorHeap);

		const D3D12_GPU_DESCRIPTOR_HANDLE& GetConstantBufferViewDescriptorHandle(unsigned index) const;
		D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGPUVirtualAddress(unsigned index);
	};
}

#endif