// DirectX12Render/Resources/RootSignature.cpp

#include <DirectX12Render/Resources/RootSignature.h>

#include <Core/Comparison.h>

#include <sstream>

using namespace DirectX12Render;

RootSignatureDescription::RootSignatureDescription()
	: Flags(D3D12_ROOT_SIGNATURE_FLAG_NONE)
{
}

void RootSignatureDescription::Initialize(const D3D12_ROOT_SIGNATURE_DESC& description)
{
	Initialize(description.NumParameters, description.pParameters, description.NumStaticSamplers,
		description.pStaticSamplers, description.Flags);
}

void RootSignatureDescription::Initialize(unsigned countRootParameters, const D3D12_ROOT_PARAMETER* rootParameters,
	unsigned countStaticSamplers, const D3D12_STATIC_SAMPLER_DESC* staticSamplers, D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	for (unsigned i = 0; i < countRootParameters; i++)
	{
		auto& parameter = rootParameters[i];
		RootParameters.PushBack(parameter);
		if (parameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			unsigned countDescriptorRanges = parameter.DescriptorTable.NumDescriptorRanges;
			auto& ranges = parameter.DescriptorTable.pDescriptorRanges;
			for (size_t j = 0; j < countDescriptorRanges; j++)
			{
				DescriptorRanges.PushBack(ranges[j]);
			}
		}
	}
	auto rangeArray = DescriptorRanges.GetArray();
	for (unsigned i = 0; i < countRootParameters; i++)
	{
		auto& parameter = rootParameters[i];
		if (parameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			RootParameters[i].DescriptorTable.pDescriptorRanges = rangeArray;
			rangeArray += parameter.DescriptorTable.NumDescriptorRanges;
		}
	}
	for (unsigned i = 0; i < countStaticSamplers; i++)
	{
		StaticSamplers.PushBack(staticSamplers[i]);
	}
	Flags = flags;
}

D3D12_ROOT_SIGNATURE_DESC RootSignatureDescription::ToD3DDescription() const
{
	D3D12_ROOT_SIGNATURE_DESC description;
	description.NumParameters = RootParameters.GetSize();
	description.pParameters = RootParameters.GetArray();
	description.NumStaticSamplers = StaticSamplers.GetSize();
	description.pStaticSamplers = StaticSamplers.GetArray();
	description.Flags = Flags;
	return description;
}

bool RootSignatureDescription::operator==(const RootSignatureDescription& other) const
{
	NumericalEqualCompareBlock(RootParameters.GetSize());
	for (unsigned i = 0; i < RootParameters.GetSize(); i++)
	{
		NumericalEqualCompareBlock(RootParameters[i].ParameterType);
		NumericalEqualCompareBlock(RootParameters[i].ShaderVisibility);
		switch (RootParameters[i].ParameterType)
		{
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
		{
			NumericalEqualCompareBlock(RootParameters[i].DescriptorTable.NumDescriptorRanges);
			// The descriptor ranges themselves are compared separately.
		}
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
		{
			MemoryEqualCompareBlock(RootParameters[i].Constants);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_CBV:
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
		case D3D12_ROOT_PARAMETER_TYPE_UAV:
		{
			MemoryEqualCompareBlock(RootParameters[i].Descriptor);
			break;
		}
		}
	}
	StructureEqualCompareBlock(DescriptorRanges);
	StructureEqualCompareBlock(StaticSamplers);
	NumericalEqualCompareBlock(Flags);
	return true;
}

bool RootSignatureDescription::operator!=(const RootSignatureDescription& other) const
{
	return !(*this == other);
}

bool RootSignatureDescription::operator<(const RootSignatureDescription& other) const
{
	NumericalLessCompareBlock(RootParameters.GetSize());
	for (unsigned i = 0; i < RootParameters.GetSize(); i++)
	{
		NumericalLessCompareBlock(RootParameters[i].ParameterType);
		NumericalLessCompareBlock(RootParameters[i].ShaderVisibility);
		switch (RootParameters[i].ParameterType)
		{
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
		{
			NumericalLessCompareBlock(RootParameters[i].DescriptorTable.NumDescriptorRanges);
			// The descriptor ranges themselves are compared separately.
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
		{
			MemoryLessCompareBlock(RootParameters[i].Constants);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_CBV:
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
		case D3D12_ROOT_PARAMETER_TYPE_UAV:
		{
			MemoryLessCompareBlock(RootParameters[i].Descriptor);
			break;
		}
		}
	}
	StructureLessCompareBlock(DescriptorRanges);
	StructureLessCompareBlock(StaticSamplers);
	NumericalLessCompareBlock(Flags);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ID3D12RootSignature* RootSignature::GetRootSignature()
{
	return m_Resource.Get();
}

void RootSignature::Initialize(ID3D12Device* device, const RootSignatureDescription& description)
{
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	auto hr = D3D12SerializeRootSignature(&description.ToD3DDescription(), D3D_ROOT_SIGNATURE_VERSION_1,
		&signature, &error);
	if (FAILED(hr))
	{
		if (error)
		{
			std::stringstream ss;
			ss << "A root signature serialization error as occured." << std::endl;
			ss << "Error message:" << std::endl << std::endl;
			ss << reinterpret_cast<const char*>(error->GetBufferPointer());
			EngineBuildingBlocks::RaiseException(ss);
		}
	}
	
	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
		IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		EngineBuildingBlocks::RaiseException("Failed to create a root signature.");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

RootSignature* RootSignatureManager::GetRootSignature(ID3D12Device* device,
	const RootSignatureDescription& description)
{
	auto it = m_RootSignatureMap.find(description);
	if (it == m_RootSignatureMap.end())
	{
		auto pRootSignature = m_RootSignatures.Request();
		pRootSignature->Initialize(device, description);
		m_RootSignatureMap[description] = pRootSignature;
		return pRootSignature;
	}
	return it->second;
}

RootSignature* RootSignatureManager::GetRootSignature(ID3D12Device* device,
	const D3D12_ROOT_SIGNATURE_DESC& description)
{
	RootSignatureDescription rDescription;
	rDescription.Initialize(description);
	return GetRootSignature(device, rDescription);
}