// DirectX12Render/Resources/Buffer.h

#ifndef _DIRECTX12RENDER_BUFFER_H_INCLUDED_
#define _DIRECTX12RENDER_BUFFER_H_INCLUDED_

#include <Core/DataStructures/SimpleTypeVector.hpp>
#include <DirectX12Render/Resources/View.h>

namespace DirectX12Render
{
	struct BufferDescription
	{
		unsigned Size;
		D3D12_HEAP_TYPE HeapType;
		D3D12_HEAP_FLAGS HeapFlags;
		D3D12_RESOURCE_FLAGS ResourceFlags;

		BufferDescription();
	};

	class Buffer
	{
		ComPtr<ID3D12Resource> m_Resource;

		BufferDescription m_Description;
		D3D12_RESOURCE_STATES m_State;

		Core::SimpleTypeVectorU<UnorderedAccessView> m_UnorderedAccessViews;

		void CreateResource(ID3D12Device* device, D3D12_RESOURCE_STATES startState);

	public:

		Buffer();
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = default;
		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) = default;

		const BufferDescription& GetDescription() const;
		unsigned GetSize() const;

		ID3D12Resource* GetResource();

		void Initialize(ID3D12Device* device,
			const BufferDescription& description,
			D3D12_RESOURCE_STATES startState = D3D12_RESOURCE_STATE_COMMON);

		D3D12_RESOURCE_STATES GetState();
		D3D12_RESOURCE_STATES Transition(ID3D12GraphicsCommandList* commandList,
			D3D12_RESOURCE_STATES targetState);
		D3D12_RESOURCE_STATES TransitionToUAV(ID3D12GraphicsCommandList* commandList);

		// This function should be called if the transition is handled
		// by the user code.
		void IndicateTransitioned(D3D12_RESOURCE_STATES targetState);

		// Mapping a buffer for both reading and writing is never allowed.

		void* MapForReading(unsigned start = 0, unsigned end = 0);
		void* MapForWriting();
		
		void UnmapAfterReading();
		void UnmapAfterWriting(unsigned start = 0, unsigned end = 0);
	
	public: // Unordered access view.

		unsigned CreateRawUnorderedAccessView(ID3D12Device* device,
			IDescriptorHeap* uavDescHeap);

		const Core::SimpleTypeVectorU<UnorderedAccessView>& GetUnorderedAccessViews() const;
	};
}

#endif