// DirectX12Render/DirectX12Includes.h

#ifndef _DIRECTX12RENDER_DIRECTX12INCLUDES_H_INCLUDED_
#define _DIRECTX12RENDER_DIRECTX12INCLUDES_H_INCLUDED_

#include <EngineBuildingBlocks/ErrorHandling.h>
#include <EngineBuildingBlocks/Graphics/Graphics.h>
#include <WindowsApplication/WindowsIncludes.h>
#include <WindowsApplication/DirectX/DirectX.h>

#include <d3d12.h>

// This header is not part of the Windows SDK. It came with the samples.
#include <DirectX12\include\d3dx12.h>

// Assign a name to the object to aid with debugging.
#if defined(_DEBUG)
inline void D3D12_Debug_SetName(ID3D12Object* pObject, LPCWSTR name)
{
	pObject->SetName(name);
}
#else
inline void D3D12_Debug_SetName(ID3D12Object*, LPCWSTR)
{
}
#endif

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
#define D3D12_DEBUG_SETNAME(x) D3D12_Debug_SetName(x.Get(), L#x)

// Note that currently all error messages and warnings are enabled in the debug layer.
// This might triggers some unwanted error messages about invalid vertex and index buffer description,
// due to a bug in Windows Update.

inline void DispatchByThreadCount(ID3D12GraphicsCommandList* commandList,
	unsigned globalSizeX, unsigned globalSizeY, unsigned globalSizeZ,
	unsigned localSizeX, unsigned localSizeY, unsigned localSizeZ)
{
	commandList->Dispatch(
		EngineBuildingBlocks::Graphics::GetThreadGroupCount(globalSizeX, localSizeX),
		EngineBuildingBlocks::Graphics::GetThreadGroupCount(globalSizeY, localSizeY),
		EngineBuildingBlocks::Graphics::GetThreadGroupCount(globalSizeZ, localSizeZ));
}

#endif