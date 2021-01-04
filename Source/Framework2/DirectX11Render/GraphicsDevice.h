// DirectX11Render/GraphicsDevice.h

#ifndef _DIRECTX11RENDER_GRAPHICSDEVICE_H_INCLUDED_
#define _DIRECTX11RENDER_GRAPHICSDEVICE_H_INCLUDED_

#include <WindowsApplication/DirectX/GraphicsDevice.h>
#include <DirectX11Render/DirectX11Includes.h>

namespace DirectX11Render
{
	const DXGI_FORMAT c_DefaultColorBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

	// Helper function for acquiring the first available hardware adapter that supports the given feature level.
	// If no such adapter can be found, *ppAdapter will be set to nullptr.
	inline void GetHardwareAdapter(IDXGIFactory2* pFactory, ComPtr<IDXGIAdapter>& adapter, D3D_FEATURE_LEVEL featureLevel)
	{
		ComPtr<IDXGIAdapter1> currentAdapter;

		uint64_t videoMemoryAmount = 0;
		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &currentAdapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			currentAdapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}

			auto currentVideoMemory = static_cast<uint64_t>(desc.DedicatedVideoMemory);
			if (currentVideoMemory > videoMemoryAmount)
			{
				adapter = currentAdapter;
				videoMemoryAmount = currentVideoMemory;
			}
		}
	}

	inline void CreateDXGIFactory(ComPtr<IDXGIFactory4>& factory, HWND windowHandle = nullptr)
	{
		HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
		if (FAILED(hr))
		{
			EngineBuildingBlocks::RaiseException("Failed to create DXGI factory 4.");
		}

		// This code does not support fullscreen transitions.
		if(windowHandle != nullptr) ThrowIfFailed(factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER));
	}

	inline void CreateGraphicsDevice(DirectXRender::GraphicsDeviceType deviceType, D3D_FEATURE_LEVEL featureLevel,
		ComPtr<IDXGIFactory4>& factory, ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& immediateContext, bool isCreatingDeviceDebug)
	{
		unsigned creationFlags = 0;
		if(isCreatingDeviceDebug) creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

		ComPtr<IDXGIAdapter> adapter;

		auto hr = (HRESULT)-1;
		switch (deviceType)
		{
			case DirectXRender::GraphicsDeviceType::WARP:
			{
				hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
				if (FAILED(hr))
				{
					EngineBuildingBlocks::RaiseException("Failed to enumerate the WARP graphics adapter.");
				}
				break;
			}
			case DirectXRender::GraphicsDeviceType::Hardware:
			{
				GetHardwareAdapter(factory.Get(), adapter, featureLevel);
				break;
			}
		}

		D3D_FEATURE_LEVEL actualFeatureLevel;
		hr = D3D11CreateDevice(
			adapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			creationFlags,
			&featureLevel, 1,
			D3D11_SDK_VERSION,
			&device,
			&actualFeatureLevel,
			&immediateContext);

		if (FAILED(hr))
		{
			EngineBuildingBlocks::RaiseException("Failed to create graphics device.");
		}
	}

	inline void CreateSwapChain(ComPtr<IDXGIFactory4>& factory, ComPtr<ID3D11Device>& device, ComPtr<IDXGISwapChain1>& swapChain,
		ComPtr<ID3D11Texture2D>& colorBuffer, ComPtr<ID3D11RenderTargetView>& colorBufferRTV,
		HWND windowHandle, UINT width, UINT height, bool isWindowed = true, UINT backBufferFrameCount = 1,
		DXGI_FORMAT format = c_DefaultColorBackBufferFormat, bool isGDICompatible = false)
	{
		// Creating the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDescription;
		swapChainDescription.Width = width;
		swapChainDescription.Height = height;
		swapChainDescription.Format = format;
		swapChainDescription.Stereo = false;
		swapChainDescription.SampleDesc.Count = 1;
		swapChainDescription.SampleDesc.Quality = 0;	// The backbuffer is not multisampled.
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.BufferCount = backBufferFrameCount;
		swapChainDescription.Scaling = DXGI_SCALING_STRETCH;
		swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDescription.Flags = (isGDICompatible ? DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE : 0);
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDescription;
		fullScreenDescription.RefreshRate.Numerator = 60;
		fullScreenDescription.RefreshRate.Denominator = 1; // 60 Hz refresh rate.
		fullScreenDescription.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fullScreenDescription.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fullScreenDescription.Windowed = isWindowed;
		auto hr = factory->CreateSwapChainForHwnd(
			device.Get(),
			windowHandle,
			&swapChainDescription,
			&fullScreenDescription,
			nullptr,
			&swapChain);
		if (FAILED(hr))
		{
			EngineBuildingBlocks::RaiseException("Failed to create swap chain.");
		}

		// Getting the back buffer's color buffer.
		hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&colorBuffer));
		if (FAILED(hr))
		{
			EngineBuildingBlocks::RaiseException("Failed to get the backbuffer's color buffer.");
		}

		// Creating render target view for the the back buffer's color buffer.
		hr = device->CreateRenderTargetView(colorBuffer.Get(), nullptr, &colorBufferRTV);
		if (FAILED(hr))
		{
			EngineBuildingBlocks::RaiseException("Failed to create render target view for the backbuffer's color buffer.");
		}
	}
}

#endif