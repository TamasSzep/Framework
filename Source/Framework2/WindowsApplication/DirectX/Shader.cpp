// WindowsApplication/DirectX/Shader.cpp

#include <WindowsApplication/DirectX/Shader.h>

#include <Core/Comparison.h>

using namespace DirectXRender;

namespace DirectXRender
{
	ShaderFlagType AsFlag(ShaderType type)
	{
		return (ShaderFlagType)(1 << (int)type);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderDefine::operator==(const ShaderDefine& other) const
{
	StringEqualCompareBlock(Name);
	StringEqualCompareBlock(Definition);
	return true;
}

bool ShaderDefine::operator!=(const ShaderDefine& other) const
{
	return !(*this == other);
}

bool ShaderDefine::operator<(const ShaderDefine& other) const
{
	StringLessCompareBlock(Name);
	StringLessCompareBlock(Definition);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderDescription::GetTargetName() const
{
	char buffer[16];
	switch (Type)
	{
	case ShaderType::Vertex: sprintf_s(buffer, "vs_%s", Version.c_str()); break;
	case ShaderType::Hull: sprintf_s(buffer, "hs_%s", Version.c_str()); break;
	case ShaderType::Domain: sprintf_s(buffer, "ds_%s", Version.c_str()); break;
	case ShaderType::Geometry: sprintf_s(buffer, "gs_%s", Version.c_str()); break;
	case ShaderType::Pixel: sprintf_s(buffer, "ps_%s", Version.c_str()); break;
	case ShaderType::Compute: sprintf_s(buffer, "cs_%s", Version.c_str()); break;
	}
	return buffer;
}

bool ShaderDescription::operator==(const ShaderDescription& other) const
{
	NumericalEqualCompareBlock(Type);
	StringEqualCompareBlock(FileName);
	StructureEqualCompareBlock(Defines); // It's more likely to be different than the following members.
	StringEqualCompareBlock(EntryPointName);
	StringEqualCompareBlock(Version);
	NumericalEqualCompareBlock(CompileFlags);
	return true;
}

bool ShaderDescription::operator!=(const ShaderDescription& other) const
{
	return !(*this == other);
}

bool ShaderDescription::operator<(const ShaderDescription& other) const
{
	NumericalLessCompareBlock(Type);
	StringLessCompareBlock(FileName);
	StructureLessCompareBlock(Defines); // It's more likely to be different than the following members.
	StringLessCompareBlock(EntryPointName);
	StringLessCompareBlock(Version);
	NumericalLessCompareBlock(CompileFlags);
	return false;
}