// OpenGLRender/Utilities/ScreenshotHandler.cpp

#include <OpenGLRender/Utilities/ScreenshotHandler.h>

using namespace EngineBuildingBlocks::Graphics;
using namespace OpenGLRender;

ScreenshotHandler::ScreenshotHandler()
	: m_ThreadPool(1)
{
}

void ScreenshotHandler::Initialize(const UtilityInitializationContext& context)
{
	m_PathHandler = context.PathHandler;
}

void ScreenshotHandler::SaveCapturedImage(unsigned width, unsigned height, unsigned countChannels)
{
	SaveImageToFile(m_PrintScreenBuffer.GetArray(),
		Image2DDescription::ColorImage(width, height, countChannels),
		m_PathHandler->GetPathFromExecutableDirectory("screenshot.png"),
		ImageSaveFlags::None);
}

void ScreenshotHandler::MakeScreenshot(const Texture2D* texture, unsigned width, unsigned height,
	PixelDataFormat pixelDataFormat)
{
	unsigned countChannels = 4;
	auto pixelDataType = PixelDataType::Uint8;
	unsigned colorBufferSize = width * height * countChannels * sizeof(uint8_t);

	// Copying data to CPU memory.
	{
		m_ThreadPool.Join();

		// Allocating CPU buffer.
		m_PrintScreenBuffer.Resize(colorBufferSize);
	
		texture->GetData(m_PrintScreenBuffer.GetArray(), pixelDataFormat, pixelDataType);
	}

	// Saving image to the hard drive.
	m_ThreadPool.GetThread(0).Execute(&ScreenshotHandler::SaveCapturedImage, this, width, height, countChannels);
}

ScreenshotHandler::ResourceData ScreenshotHandler::GetCopyResource(const Texture2DDescription& texDesc)
{
	auto tIt = m_FBOMap.find(texDesc);
	if (tIt == m_FBOMap.end())
	{
		auto texture = m_Textures.Request();
		auto fbo = m_FBOs.Request();

		texture->Initialize(texDesc);
		fbo->Initialize();
		fbo->Bind();
		fbo->Attach(*texture, FrameBufferAttachment::Color);

		tIt = m_FBOMap.insert({ texDesc, { texture, fbo } }).first;
	}
	return tIt->second;
}

inline void GetFBOState(GLint* pReadFBo, GLint* pDrawFBO, GLint* pReadBuffer, GLint* pDrawBuffer)
{
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, pReadFBo);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, pDrawFBO);
	glGetIntegerv(GL_READ_BUFFER, pReadBuffer);
	glGetIntegerv(GL_DRAW_BUFFER, pDrawBuffer);
}

inline void SetFBOState(GLint readFBo, GLint drawFBO, GLint readBuffer, GLint drawBuffer)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
	glReadBuffer(readBuffer);
	glDrawBuffer(drawBuffer);
}

void ScreenshotHandler::MakeScreenshotFromFBO(unsigned fbo, unsigned readBuffer, unsigned width, unsigned height,
	PixelDataFormat pixelDataFormat)
{
	auto textureFormat = TextureFormat::RGBA8;

	GLint prevReadFBO, prevDrawFBO, prevReadBuffer, prevDrawBuffer;
	GetFBOState(&prevReadFBO, &prevDrawFBO, &prevReadBuffer, &prevDrawBuffer);

	auto copyResource = GetCopyResource({ width, height, textureFormat });

	SetFBOState(fbo, copyResource.FBO->GetHandle(), readBuffer, GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	SetFBOState(prevReadFBO, prevDrawFBO, prevReadBuffer, prevDrawBuffer);

	MakeScreenshot(copyResource.Texture, width, height, pixelDataFormat);
}