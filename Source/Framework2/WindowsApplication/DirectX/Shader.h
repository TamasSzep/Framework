// WindowsApplication/DirectX/Shader.h

#ifndef _WINDOWSAPPLICATION_SHADER_H_INCLUDED_
#define _WINDOWSAPPLICATION_SHADER_H_INCLUDED_

#include <Core/Enum.h>

#include <string>
#include <sstream>
#include <vector>

namespace DirectXRender
{
	enum class ShaderType : unsigned char
	{
		Vertex, Hull, Domain, Geometry, Pixel, Compute
	};

	enum class ShaderFlagType : unsigned char
	{
		None = 0x0,
		Vertex = 0x01,
		Hull = 0x02,
		Domain = 0x04,
		Geometry = 0x08,
		Pixel = 0x10,
		Compute = 0x20,
		All = Vertex | Hull | Domain | Geometry | Pixel | Compute
	};

	UseEnumAsFlagSet(ShaderFlagType);

	ShaderFlagType AsFlag(ShaderType type);

	struct ShaderDefine
	{
		std::string Name;
		std::string Definition;

		ShaderDefine(const char* name, const char* definition) : Name(name), Definition(definition) {}

		template <typename T>
		ShaderDefine(const char* name, T&& definition)
			: Name(name)
		{
			std::stringstream ss;
			ss << std::forward<T>(definition);
			Definition = ss.str();
		}

		bool operator==(const ShaderDefine& other) const;
		bool operator!=(const ShaderDefine& other) const;
		bool operator<(const ShaderDefine& other) const;
	};

	struct ShaderDescription
	{
		std::string FileName;
		std::string EntryPointName;
		ShaderType Type;
		std::string Version;
		unsigned CompileFlags;
		std::vector<ShaderDefine> Defines;

		std::string GetTargetName() const;

		bool operator==(const ShaderDescription& other) const;
		bool operator!=(const ShaderDescription& other) const;
		bool operator<(const ShaderDescription& other) const;
	};

	struct ShaderSimpleDescription
	{
		std::string FileName;
		std::string EntryPointName;
		ShaderType Type;
		std::vector<ShaderDefine> Defines;
	};
}

#endif