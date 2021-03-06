// OpenGLRender/Resources/Shader.h

#ifndef OPENGLRENDER_SHADER_H_INCLUDED_
#define OPENGLRENDER_SHADER_H_INCLUDED_

#include <OpenGLRender/Resources/Resource.h>

#include <Core/Constants.h>
#include <EngineBuildingBlocks/Math/GLM.h>
#include <EngineBuildingBlocks/PathHandler.h>
#include <EngineBuildingBlocks/Graphics/Primitives/Primitive.h>

#include <string>
#include <vector>
#include <sstream>

namespace OpenGLRender
{
	enum class ShaderType : unsigned
	{
		Vertex = GL_VERTEX_SHADER,
		TessellationControl = GL_TESS_CONTROL_SHADER,
		TessellationEvaluation = GL_TESS_EVALUATION_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Compute = GL_COMPUTE_SHADER,

		COUNT = 6
	};

	enum class ShaderSourceType : unsigned char
	{
		Text, File
	};

	struct ShaderDefine
	{
		std::string Name;
		std::string Definition;

		ShaderDefine(const std::string& name, const std::string& definition)
			: Name(name), Definition(definition) {}

		template <typename T>
		ShaderDefine(const std::string& name, T&& value)
			: Name(name)
		{
			std::stringstream ss;
			ss << value;
			Definition = ss.str();
		}

		bool operator==(const ShaderDefine& other) const;
		bool operator!=(const ShaderDefine& other) const;
		bool operator<(const ShaderDefine& other) const;
	};

	// Describes shader source. For SourceType == Text SourceString contains the
	// shader string, for SourceType == File, it contains the path to the file.
	struct ShaderDescription
	{
		std::string SourceString;
		ShaderSourceType SourceType;
		ShaderType Type;
		std::string Version;				// Optional.
		std::vector<ShaderDefine> Defines;	// Optional.

		static ShaderDescription FromText(const std::string& shaderText,
			ShaderType type, const std::string& version = std::string(),
			const std::vector<ShaderDefine>& defines = std::vector<ShaderDefine>());
		static ShaderDescription FromFile(const std::string& filePath,
			ShaderType type, const std::string& version = std::string(),
			const std::vector<ShaderDefine>& defines = std::vector<ShaderDefine>());
		static ShaderDescription FromFile(
			const EngineBuildingBlocks::PathHandler& pathHandler,
			const std::string& fileName,
			ShaderType type, const std::string& version = std::string(),
			const std::vector<ShaderDefine>& defines = std::vector<ShaderDefine>());

		const std::string& GetFilePath() const;
		std::string GetText() const;

		bool operator==(const ShaderDescription& other) const;
		bool operator!=(const ShaderDescription& other) const;
		bool operator<(const ShaderDescription& other) const;
	};

	struct ShaderProgramDescription
	{
		std::vector<ShaderDescription> Shaders;

		bool operator==(const ShaderProgramDescription& other) const;
		bool operator!=(const ShaderProgramDescription& other) const;
		bool operator<(const ShaderProgramDescription& other) const;
	};

	class Shader
	{
	public:

		static void DeleteResource(GLuint handle);

	private:

		ResourceHandle<Shader> m_Handle;

	public:

		void Initialize(const ShaderDescription& description);

		GLuint GetHandle() const;
	};

	class ShaderProgram
	{
	public:

		static void DeleteResource(GLuint handle);

	private:

		ResourceHandle<ShaderProgram> m_Handle;

	public:

		GLuint GetHandle() const;
		GLint GetUniformLocation(const char* name) const;
		GLint GetAttributeLocation(const char* name) const;

		void Initialize(const ShaderProgramDescription& description,
			Shader** pShaders);

		void Bind();
		void Unbind();

		void SetAttributeBuffer(GLint location, int tupleSize, GLenum type,
			int stride, int offset, GLboolean normalized = GL_FALSE);
		void SetAttributeBuffer(const char* name, int tupleSize, GLenum type,
			int stride, int offset, GLboolean normalized = GL_FALSE);

		void EnableAttributeArray(GLint location);
		void EnableAttributeArray(const char* name);

		void SetAttributeDivisor(GLint location, GLuint divisor);
		void SetAttributeDivisor(const char* name, GLuint divisor);

		void SetInputLayout(const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout);
		void SetInputLayout(const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
			unsigned vertexAttributeDivisor);

		void TrySetInputLayoutElements(const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout);
		void TrySetInputLayoutElements(const EngineBuildingBlocks::Graphics::VertexInputLayout& inputLayout,
			unsigned vertexAttributeDivisor);

		void SetUniformValue(GLint location, int value);
		void SetUniformValue(GLint location, unsigned value);
		void SetUniformValue(GLint location, float value);
		void SetUniformValue(GLint location, double value);
		void SetUniformValue(GLint location, const glm::ivec2& value);
		void SetUniformValue(GLint location, const glm::uvec2& value);
		void SetUniformValue(GLint location, const glm::vec2& value);
		void SetUniformValue(GLint location, const glm::dvec2& value);
		void SetUniformValue(GLint location, const glm::ivec3& value);
		void SetUniformValue(GLint location, const glm::uvec3& value);
		void SetUniformValue(GLint location, const glm::vec3& value);
		void SetUniformValue(GLint location, const glm::dvec3& value);
		void SetUniformValue(GLint location, const glm::ivec4& value);
		void SetUniformValue(GLint location, const glm::uvec4& value);
		void SetUniformValue(GLint location, const glm::vec4& value);
		void SetUniformValue(GLint location, const glm::dvec4& value);
		void SetUniformValue(GLint location, const glm::mat3& value);
		void SetUniformValue(GLint location, const glm::mat4& value);

		void SetUniformValueArray(GLint location, const int* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const unsigned* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const float* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const double* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::ivec2* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::uvec2* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::vec2* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::dvec2* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::ivec3* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::uvec3* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::vec3* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::dvec3* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::ivec4* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::uvec4* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::vec4* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::dvec4* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::mat3* pValue, GLsizei count);
		void SetUniformValueArray(GLint location, const glm::mat4* pValue, GLsizei count);

		template <typename T>
		void SetUniformValue(const char* name, const T& value)
		{
			SetUniformValue(GetUniformLocation(name), value);
		}

		template <typename T>
		void SetUniformValueArray(const char* name, const T* pValue, GLsizei count)
		{
			SetUniformValueArray(GetUniformLocation(name), pValue, count);
		}
	};

	class ShaderManager
	{
		EngineBuildingBlocks::PathHandler* m_PathHandler;

		Core::ResourcePoolU<Shader> m_Shaders;
		Core::ResourcePoolU<ShaderProgram> m_ShaderPrograms;
		std::map<ShaderDescription, Shader*> m_ShaderMap;
		std::map<ShaderProgramDescription, ShaderProgram*> m_ShaderProgramMap;

	public:

		ShaderManager(EngineBuildingBlocks::PathHandler* pathHandler);

		Shader* GetShader(const ShaderDescription& description,
			bool forceRecompilation = false);
		ShaderProgram* GetShaderProgram(const ShaderProgramDescription& description,
			bool forceRecompilation = false);
	};
}

#endif