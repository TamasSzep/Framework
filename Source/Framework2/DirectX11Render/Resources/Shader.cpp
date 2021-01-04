// DirectX11Render/Resources/Shader.cpp

#include <DirectX11Render/Resources/Shader.h>

#include <Core/String.hpp>
#include <EngineBuildingBlocks/PathHandler.h>

using namespace EngineBuildingBlocks;
using namespace DirectXRender;
using namespace DirectX11Render;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Shader::Shader()
	: m_ShaderPtr(nullptr)
{
}

Shader::Shader(const ShaderDescription& description)
	: m_Description(description)
	, m_ShaderPtr(nullptr)
{
}

Shader::~Shader()
{
	SafeRelease(reinterpret_cast<ID3D11DeviceChild*&>(m_ShaderPtr));
}

void Shader::Initialize(ID3D11Device* device, const EngineBuildingBlocks::PathHandler& pathHandler)
{
	auto path = pathHandler.GetPathFromResourcesDirectory("Shaders/DX11/" + m_Description.FileName);

	// Creating defines array.
	std::vector<D3D_SHADER_MACRO> defines;
	auto& sourceDefines = m_Description.Defines;
	for (size_t i = 0; i < sourceDefines.size(); i++)
	{
		auto& cDefine = sourceDefines[i];
		defines.push_back({ cDefine.Name.c_str(), cDefine.Definition.c_str() });
	}
	defines.push_back({ nullptr, nullptr }); // Terminating the define array.

	ID3DBlob* pErrorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(Core::ToWString(path).c_str(),
		defines.data(),
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // The default includer uses paths relative to the shader.
		m_Description.EntryPointName.c_str(),
		m_Description.GetTargetName().c_str(),
		m_Description.CompileFlags, 0, &m_Blob, &pErrorBlob);
	if (FAILED(hr))
	{
		std::stringstream ss;
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			ss << "DirectX 11 shader file not found: " << m_Description.FileName;
		}
		else if (pErrorBlob != nullptr)
		{
			ss << "A shader compilation error as occured." << std::endl;
			ss << "Shader name: " << m_Description.FileName << std::endl;
			ss << "Error message:" << std::endl << std::endl;
			ss << (const char*)pErrorBlob->GetBufferPointer();	
		}
		else
		{
			ss << "An unknown error was occured while compiling from DirectX 11 shader file: "
				<< m_Description.FileName;
		}
		EngineBuildingBlocks::RaiseException(ss);
	}
	SafeRelease(pErrorBlob);

	switch (m_Description.Type)
	{
	case ShaderType::Vertex:
		hr = device->CreateVertexShader(m_Blob->GetBufferPointer(),
			m_Blob->GetBufferSize(), nullptr, (ID3D11VertexShader**)&m_ShaderPtr);
		break;
	case ShaderType::Hull:
		hr = device->CreateHullShader(m_Blob->GetBufferPointer(),
			m_Blob->GetBufferSize(), nullptr, (ID3D11HullShader**)&m_ShaderPtr);
		break;
	case ShaderType::Domain:
		hr = device->CreateDomainShader(m_Blob->GetBufferPointer(),
			m_Blob->GetBufferSize(), nullptr, (ID3D11DomainShader**)&m_ShaderPtr);
		break;
	case ShaderType::Geometry:
		hr = device->CreateGeometryShader(m_Blob->GetBufferPointer(),
			m_Blob->GetBufferSize(), nullptr, (ID3D11GeometryShader**)&m_ShaderPtr);
		break;
	case ShaderType::Pixel:
		hr = device->CreatePixelShader(m_Blob->GetBufferPointer(),
			m_Blob->GetBufferSize(), nullptr, (ID3D11PixelShader**)&m_ShaderPtr);
		break;
	case ShaderType::Compute:
		hr = device->CreateComputeShader(m_Blob->GetBufferPointer(),
			m_Blob->GetBufferSize(), nullptr, (ID3D11ComputeShader**)&m_ShaderPtr);
		break;
	}
	if (FAILED(hr))
	{
		RaiseException("Failed to create shader.");
	}
}

const DirectXRender::ShaderDescription& Shader::GetDescription() const
{
	return m_Description;
}

ID3DBlob* Shader::GetBlob() const
{
	return m_Blob.Get();
}

const void* Shader::GetShaderPtr() const
{
	return m_ShaderPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderManager::ShaderManager(EngineBuildingBlocks::PathHandler* pathHandler)
	: m_PathHandler(pathHandler)
{
}

const Shader* ShaderManager::GetShaderSimple(ID3D11Device* device, const ShaderSimpleDescription& description, bool forceRecompilation)
{
	ShaderDescription desc;
	desc.FileName = description.FileName;
	desc.EntryPointName = description.EntryPointName;
	desc.Type = description.Type;
	desc.Version = "5_0";

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	desc.CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	desc.CompileFlags = 0;
#endif

	desc.Defines = description.Defines;

	return GetShader(device, desc, forceRecompilation);
}

const Shader* ShaderManager::GetShader(ID3D11Device* device, const ShaderDescription& description, bool forceRecompilation)
{
	auto it = m_ShaderMap.find(description);

	Shader* shader;
	if (forceRecompilation || it == m_ShaderMap.end())
	{
		shader = m_Shaders.Request(description);
		shader->Initialize(device, *m_PathHandler);
	
		// This overwrites an existing mapping if the same shader description was encountered before and we use force recompilation now.
		m_ShaderMap[description] = shader; 
	}
	else
	{
		shader = it->second;
	}

	return shader;
}