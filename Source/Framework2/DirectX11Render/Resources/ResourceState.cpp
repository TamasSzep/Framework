// DirectX11Render/Resources/ResourceState.cpp

#include <DirectX11Render/Resources/ResourceState.h>

using namespace DirectXRender;
using namespace DirectX11Render;

ResourceState::ResourceState()
	: Shaders(ShaderFlagType::None)
	, CountCBs(0)
	, CountSRVs(0)
{
	memset(PointerStartIndex, 0, sizeof(PointerStartIndex));
}

ResourceState::ResourceState(ShaderFlagType shaders, unsigned char countCBs, unsigned char countSRVs)
	: Shaders(shaders)
	, CountCBs(countCBs)
	, CountSRVs(countSRVs)
{
	unsigned stageSize = countCBs + countSRVs;

	unsigned countFlags = 0;
	for (unsigned i = 0; i < 6; i++)
	{
		PointerStartIndex[i] = countFlags * stageSize;
		if (HasFlag(shaders, AsFlag((ShaderType)i)))
		{
			countFlags++;
		}
	}

	Pointers.Resize(stageSize * countFlags);
	Pointers.SetByte(0);
}

void ResourceState::AddConstantBuffer(ID3D11Buffer* buffer, unsigned char index, ShaderFlagType shaders)
{
	assert(IsSubset(Shaders, shaders));
	assert(index < CountCBs);

	for (unsigned i = 0; i < 6; i++)
	{
		if (HasFlag(shaders, AsFlag((ShaderType)i))) Pointers[PointerStartIndex[i] + index] = buffer;
	}
}

void ResourceState::AddShaderResourceView(ID3D11ShaderResourceView* srv, unsigned char index, ShaderFlagType shaders)
{
	assert(IsSubset(Shaders, shaders));
	assert(index < CountSRVs);

	for (unsigned i = 0; i < 6; i++)
	{
		if (HasFlag(shaders, AsFlag((ShaderType)i))) Pointers[PointerStartIndex[i] + CountCBs + index] = srv;
	}
}

template <typename D3DCBSettingFunctionType, typename D3DSRVSettingFunctionType>
void SetShaderResourcesForContext(ID3D11DeviceContext* context, ResourceState& resourceState, unsigned char index,
	D3DCBSettingFunctionType&& cbSettingFunction, D3DSRVSettingFunctionType&& srvSettingFunction)
{
	if (HasFlag(resourceState.Shaders, AsFlag((ShaderType)index)))
	{
		unsigned countCBs = resourceState.CountCBs;
		unsigned countSRVs = resourceState.CountSRVs;
		if (countCBs > 0) (context->*cbSettingFunction)(0, countCBs,
			reinterpret_cast<ID3D11Buffer* const*>(&resourceState.Pointers[resourceState.PointerStartIndex[index]]));
		if (countSRVs > 0) (context->*srvSettingFunction)(0, countSRVs,
			reinterpret_cast<ID3D11ShaderResourceView* const*>(&resourceState.Pointers[resourceState.PointerStartIndex[index] + resourceState.CountCBs]));
	}
}

void ResourceState::SetForContext(ID3D11DeviceContext* context)
{
	using dct = ID3D11DeviceContext;
	SetShaderResourcesForContext(context, *this, 0, &dct::VSSetConstantBuffers, &dct::VSSetShaderResources);
	SetShaderResourcesForContext(context, *this, 1, &dct::HSSetConstantBuffers, &dct::HSSetShaderResources);
	SetShaderResourcesForContext(context, *this, 2, &dct::DSSetConstantBuffers, &dct::DSSetShaderResources);
	SetShaderResourcesForContext(context, *this, 3, &dct::GSSetConstantBuffers, &dct::GSSetShaderResources);
	SetShaderResourcesForContext(context, *this, 4, &dct::PSSetConstantBuffers, &dct::PSSetShaderResources);
	SetShaderResourcesForContext(context, *this, 5, &dct::CSSetConstantBuffers, &dct::CSSetShaderResources);
}

bool ResourceState::operator==(const ResourceState& other) const
{
	NumericalEqualCompareBlock(Shaders);
	NumericalEqualCompareBlock(CountCBs);
	NumericalEqualCompareBlock(CountSRVs);
	StructureEqualCompareBlock(Pointers);
	return true;
}

bool ResourceState::operator!=(const ResourceState& other) const
{
	return !(*this == other);
}

bool ResourceState::operator<(const ResourceState& other) const
{
	NumericalLessCompareBlock(Shaders);
	NumericalLessCompareBlock(CountCBs);
	NumericalLessCompareBlock(CountSRVs);
	StructureLessCompareBlock(Pointers);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceState& ResourceStateManager::GetResourceState(unsigned index)
{
	return m_ResourceStates[index];
}

unsigned ResourceStateManager::AddResourceState(const ResourceState& resourceState)
{
	auto it = m_ResourceStateIndices.find(resourceState);
	if (it == m_ResourceStateIndices.end())
	{
		it = m_ResourceStateIndices.insert({ resourceState, m_ResourceStates.Add(resourceState) }).first;
	}
	return it->second;
}