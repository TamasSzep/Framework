// OpenGLRender/Resources/Texture2D.cpp

#include <OpenGLRender/Resources/Texture2D.h>

#include <Core/Comparison.h>
#include <EngineBuildingBlocks/Graphics/Resources/ResourceUtility.h>

#include <cassert>

using namespace OpenGLRender;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool IsSampleCountAndTargetConsistent(unsigned sampleCount, TextureTarget target)
{
	if (sampleCount == 0) return false;
	return 
		((sampleCount == 1)
		== 
		(target != TextureTarget::Texture2DMS && target != TextureTarget::Texture2DMSArray));
}

Texture2DDescription::Texture2DDescription()
	: Width(0)
	, Height(0)
	, ArraySize(1)
	, SampleCount(1)
	, Target(TextureTarget::Texture2D)
	, Format(TextureFormat::Unknown)
	, HasMipmaps(false)
{
	assert(IsSampleCountAndTargetConsistent(SampleCount, Target));
}

Texture2DDescription::Texture2DDescription(unsigned width, unsigned height, TextureFormat format,
	unsigned arraySize, unsigned sampleCount, TextureTarget target, bool hasMipmaps)
	: Width(width)
	, Height(height)
	, ArraySize(arraySize)
	, SampleCount(sampleCount)
	, Target(target)
	, Format(format)
	, HasMipmaps(hasMipmaps)
{
	assert(IsSampleCountAndTargetConsistent(SampleCount, Target));
}

bool Texture2DDescription::IsCubemap() const
{
	return (Target == TextureTarget::TextureCubemap || Target == TextureTarget::TextureCubemapArray);
}

bool Texture2DDescription::IsArray() const
{
	return (Target == TextureTarget::Texture2DArray || Target == TextureTarget::Texture2DMSArray
		|| Target == TextureTarget::TextureCubemapArray);
}

bool Texture2DDescription::IsMultisampled() const
{
	return (Target == TextureTarget::Texture2DMS || Target == TextureTarget::Texture2DMSArray);
}

unsigned Texture2DDescription::GetCountMipmapLevels() const
{
	return (HasMipmaps ? EngineBuildingBlocks::Graphics::GetMipmapLevelCount(Width, Height) : 1);
}

unsigned Texture2DDescription::GetPixelFormatSizeInBytes() const
{
	return GetTextureFormatSizeInBytes(Format);
}

unsigned Texture2DDescription::GetCountChannels() const
{
	return OpenGLRender::GetCountChannels(Format);
}

unsigned Texture2DDescription::GetCompactRowPitch() const
{
	return Width * SampleCount * GetPixelFormatSizeInBytes();
}

unsigned Texture2DDescription::GetCompactSlicePitch() const
{
	return Width * Height * SampleCount * GetPixelFormatSizeInBytes();
}

PixelType Texture2DDescription::GetPixelType() const
{
	return OpenGLRender::GetPixelType(Format);
}

void Texture2DDescription::SetToCubemap()
{
	Target = TextureTarget::TextureCubemap;
}

bool Texture2DDescription::operator==(const Texture2DDescription& other) const
{
	NumericalEqualCompareBlock(Width);
	NumericalEqualCompareBlock(Height);
	NumericalEqualCompareBlock(ArraySize);
	NumericalEqualCompareBlock(SampleCount);
	NumericalEqualCompareBlock(Target);
	NumericalEqualCompareBlock(Format);
	NumericalEqualCompareBlock(HasMipmaps);
	return true;
}

bool Texture2DDescription::operator!=(const Texture2DDescription& other) const
{
	return !(*this == other);
}

bool Texture2DDescription::operator<(const Texture2DDescription& other) const
{
	NumericalLessCompareBlock(Width);
	NumericalLessCompareBlock(Height);
	NumericalLessCompareBlock(ArraySize);
	NumericalLessCompareBlock(SampleCount);
	NumericalLessCompareBlock(Target);
	NumericalLessCompareBlock(Format);
	NumericalLessCompareBlock(HasMipmaps);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2D::Texture2D()
{
}

Texture2D::Texture2D(const Texture2DDescription& textureDesc, const TextureSamplingDescription& samplingDesc)
{
	Initialize(textureDesc, samplingDesc);
}

void Texture2D::Initialize(const Texture2DDescription& textureDesc, const TextureSamplingDescription& samplingDesc)
{
	assert(textureDesc.SampleCount == 1 || !textureDesc.HasMipmaps);

	m_TextureDescription = textureDesc;
	m_Texture.Initialize();

	Bind();

	if (textureDesc.SampleCount == 1)
	{
		// Note that multisampled textures are not sampled at all, and an error is generated if one tries
		// to set their sampler state.
		SetTextureSampler(textureDesc.Target, samplingDesc);
	}

	auto target = (GLenum)textureDesc.Target;
	auto format = (GLint)textureDesc.Format;
	auto width = textureDesc.Width;
	auto height = textureDesc.Height;
	auto arraySize = textureDesc.ArraySize;
	auto countLevels = textureDesc.GetCountMipmapLevels();
	auto countSamples = textureDesc.SampleCount;
	switch (textureDesc.Target)
	{
	case TextureTarget::Texture2D:
	case TextureTarget::TextureCubemap: glTexStorage2D(target, countLevels, format, width, height); break;
	case TextureTarget::Texture2DArray: glTexStorage3D(target, countLevels, format, width, height, arraySize); break;
	case TextureTarget::TextureCubemapArray: glTexStorage3D(target, countLevels, format, width, height, arraySize * 6); break;
	case TextureTarget::Texture2DMS: glTexStorage2DMultisample(target, countSamples, format, width, height, GL_TRUE); break;
	case TextureTarget::Texture2DMSArray: glTexStorage3DMultisample(target, countSamples, format, width, height, arraySize, GL_TRUE); break;
	default: break;
	}

	Unbind();
}

void Texture2D::Delete()
{
	m_Texture.Delete();
}

GLuint Texture2D::GetHandle() const
{
	return m_Texture.GetHandle();
}

const Texture2DDescription& Texture2D::GetTextureDescription() const
{
	return m_TextureDescription;
}

void Texture2D::Bind()
{
	glBindTexture((GLenum)m_TextureDescription.Target, m_Texture.GetHandle());
}

void Texture2D::Unbind()
{
	glBindTexture((GLenum)m_TextureDescription.Target, 0);
}

void Texture2D::Bind(unsigned unitIndex)
{
	glActiveTexture(GL_TEXTURE0 + unitIndex);
	Bind();
}

void Texture2D::Unbind(unsigned unitIndex)
{
	glActiveTexture(GL_TEXTURE0 + unitIndex);
	Unbind();
}

void Texture2D::GetData(void* pData, PixelDataFormat pixelDataFormat, PixelDataType pixelDataType) const
{
	unsigned start[] = { 0, 0, 0, 0 };
	unsigned end[] = { m_TextureDescription.Width, m_TextureDescription.Height,
		(m_TextureDescription.Target == TextureTarget::TextureCubemap ? 6 : m_TextureDescription.ArraySize), 6 };
	GetData(pData, pixelDataFormat, pixelDataType, start, end, 0);
}

void Texture2D::GetData(void* pData, PixelDataFormat pixelDataFormat, PixelDataType pixelDataType,
	const unsigned* pOffset, const unsigned* pSize, unsigned mipmapLevel) const
{
	unsigned bufferSize = std::numeric_limits<GLsizei>::max(); // Not checking the buffer size.
	unsigned zOffset = 0;
	unsigned zSize = 1;
	switch (m_TextureDescription.Target)
	{
	case TextureTarget::Texture2DArray:
	case TextureTarget::TextureCubemap:
		zOffset = pOffset[2];
		zSize = pSize[2];
	case TextureTarget::TextureCubemapArray:
		assert(pSize[3] == 6);
		zOffset = pOffset[2] * 6 + pOffset[3];
		zSize = pSize[2] * 6;
	}
	glGetTextureSubImage(GetHandle(), mipmapLevel, pOffset[0], pOffset[1], zOffset, pSize[0], pSize[1], zSize,
		(GLenum)pixelDataFormat, (GLenum)pixelDataType, bufferSize, pData);
}

void Texture2D::SetData(const void* pData, PixelDataFormat pixelDataFormat, PixelDataType pixelDataType)
{
	unsigned start[] = { 0, 0, 0, 0 };
	unsigned end[] = { m_TextureDescription.Width, m_TextureDescription.Height,
		(m_TextureDescription.Target == TextureTarget::TextureCubemap ? 6 : m_TextureDescription.ArraySize), 6 };
	SetData(pData, pixelDataFormat, pixelDataType, start, end, 0);
}

void Texture2D::SetData(const void* pData, PixelDataFormat pixelDataFormat, PixelDataType pixelDataType,
	const unsigned* pOffset, const unsigned* pSize, unsigned mipmapLevel)
{
	assert(pData != nullptr);
	assert(m_TextureDescription.Target != TextureTarget::Texture2DMS
		&& m_TextureDescription.Target != TextureTarget::Texture2DMSArray);
	auto dataFormat = (GLenum)pixelDataFormat;
	auto dataType = (GLenum)pixelDataType;
	switch (m_TextureDescription.Target)
	{
	case TextureTarget::Texture2D:
		glTexSubImage2D(GL_TEXTURE_2D, mipmapLevel, pOffset[0], pOffset[1], pSize[0], pSize[1], dataFormat, dataType, pData); break;
	case TextureTarget::Texture2DArray:
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipmapLevel, pOffset[0], pOffset[1], pOffset[2], pSize[0], pSize[1], pSize[2],
			dataFormat, dataType, pData); break;
	case TextureTarget::TextureCubemap:
	{
		unsigned writeSize = pSize[0] * pSize[1] * GetPixelDataSizeInBytes(pixelDataFormat, pixelDataType);
		auto ptr = reinterpret_cast<const unsigned char*>(pData);
		auto faceIndexLimit = pOffset[2] + pSize[2];
		for (unsigned faceIndex = pOffset[2]; faceIndex < faceIndexLimit; ++faceIndex, ptr += writeSize)
		{
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, mipmapLevel, pOffset[0], pOffset[1],
				pSize[0], pSize[1], dataFormat, dataType, ptr);
		}
		break;
	}
	case TextureTarget::TextureCubemapArray:
	{
		unsigned writeSize = pSize[0] * pSize[1] * GetPixelDataSizeInBytes(pixelDataFormat, pixelDataType);
		auto ptr = reinterpret_cast<const unsigned char*>(pData);
		auto faceIndexLimit = pOffset[3] + pSize[3];
		for (unsigned faceIndex = pOffset[3]; faceIndex < faceIndexLimit; ++faceIndex, ptr += writeSize)
		{
			glTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, mipmapLevel, pOffset[0], pOffset[1],
				pOffset[2], pSize[0], pSize[1], pSize[2], dataFormat, dataType, ptr);
		}
		break;
	}
	default: break;
	}
}

void Texture2D::GenerateMipmaps()
{
	assert(m_TextureDescription.HasMipmaps == true);
	glGenerateMipmap((GLenum)m_TextureDescription.Target);
}