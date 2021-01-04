// DirectX12Render/GraphicsDevice.h

#ifndef _DIRECTX12RENDER_GRAPHICSDEVICE_H_INCLUDED_
#define _DIRECTX12RENDER_GRAPHICSDEVICE_H_INCLUDED_

#include <WindowsApplication/DirectX/GraphicsDevice.h>
#include <DirectX12Render/DirectX12Includes.h>

namespace DirectX12Render
{
	// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
	// If no such adapter can be found, *ppAdapter will be set to nullptr.
	inline void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL featureLevel)
	{
		ComPtr<IDXGIAdapter1> adapter;
		*ppAdapter = nullptr;

		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}

			// Check to see if the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}

		*ppAdapter = adapter.Detach();
	}

	inline void EnableDebugLayer()
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}

	inline void CreateDXGIFactory(ComPtr<IDXGIFactory4>& factory, HWND windowHandle = nullptr)
	{
		auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
		if (FAILED(hr))
		{
			EngineBuildingBlocks::RaiseException("Failed to create a DXGI factory 1.");
		}

		// This code does not support fullscreen transitions.
		if(windowHandle != nullptr) ThrowIfFailed(factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER));
	}

	inline void CreateGraphicsDevice(DirectXRender::GraphicsDeviceType deviceType, D3D_FEATURE_LEVEL featureLevel,
		ComPtr<IDXGIFactory4>& factory, ComPtr<ID3D12Device>& device)
	{
		HRESULT hr;
		switch (deviceType)
		{
		case DirectXRender::GraphicsDeviceType::WARP:
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
			if (FAILED(hr))
			{
				EngineBuildingBlocks::RaiseException("Failed to enumerate the WARP graphics adapter.");
			}

			hr = D3D12CreateDevice(
				warpAdapter.Get(),
				featureLevel,
				IID_PPV_ARGS(&device));
			if (FAILED(hr))
			{
				EngineBuildingBlocks::RaiseException("Failed to create a WARP graphics device.");
			}
		}
		case DirectXRender::GraphicsDeviceType::Hardware:
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter(factory.Get(), &hardwareAdapter, featureLevel);

			hr = D3D12CreateDevice(
				hardwareAdapter.Get(),
				featureLevel,
				IID_PPV_ARGS(&device));
			if (FAILED(hr))
			{
				EngineBuildingBlocks::RaiseException("Failed to create a hardware graphics device.");
			}
		}
		}
	}

	inline void CreateCommandQueue(ComPtr<ID3D12Device>& device, ComPtr<ID3D12CommandQueue>& commandQueue)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
	}

	inline void CreateSwapChain(ComPtr<IDXGIFactory4>& factory, ComPtr<ID3D12Device>& device,
		ComPtr<ID3D12CommandQueue>& commandQueue, ComPtr<IDXGISwapChain3>& swapChain3,
		HWND windowHandle, UINT width, UINT height, bool isWindowed = true, UINT backBufferFrameCount = 2,
		UINT maximumFrameLatency = 2, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = backBufferFrameCount;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = format;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = windowHandle;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = isWindowed;
		swapChainDesc.Flags = (isWindowed
			? DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
			: 0);
		ComPtr<IDXGISwapChain> swapChain;
		ThrowIfFailed(factory->CreateSwapChain(
			commandQueue.Get(),		// Swap chain needs the command queue so that it can force a flush on it.
			&swapChainDesc,
			&swapChain
		));
		ThrowIfFailed(swapChain.As(&swapChain3));

		if (isWindowed) swapChain3->SetMaximumFrameLatency(maximumFrameLatency);
	}
}

#endif