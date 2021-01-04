// DirectX11Render/Utilities/ScreenshotHandler.cpp

#include <DirectX11Render/Utilities/ScreenshotHandler.h>

using namespace EngineBuildingBlocks::Graphics;
using namespace DirectX11Render;

ScreenshotHandler::ScreenshotHandler()
	: m_ThreadPool(1)
{
}

void ScreenshotHandler::Initialize(const UtilityInitializationContext& context)
{
	m_PathHandler = context.PathHandler;
}

Texture2D& ScreenshotHandler::GetTexture(ID3D11Device* device,
	const DirectX11Render::Texture2DDescription& description)
{
	auto it = m_Textures.find(description);
	if (it == m_Textures.end())
	{
		Texture2D texture;
		texture.Initialize(device, description);
		it = m_Textures.insert({ description, std::move(texture) }).first;
	}
	return it->second;
}

void ScreenshotHandler::SaveCapturedImage(unsigned width, unsigned height, unsigned countChannels)
{
	SaveImageToFile(m_PrintScreenBuffer.GetArray(),
		Image2DDescription::ColorImage(width, height, countChannels),
		m_PathHandler->GetPathFromExecutableDirectory("screenshot.png"),
		ImageSaveFlags::IsFlippingY);
}

void ScreenshotHandler::MakeScreenshot(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture,
	unsigned width, unsigned height, DXGI_FORMAT format)
{
	unsigned countChannels = 4;
	unsigned colorBufferSize = width * height * countChannels * sizeof(uint8_t);

	// Copying data to CPU memory.
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		// Allocating CPU buffer.
		m_PrintScreenBuffer.Resize(colorBufferSize);

		// Getting copy (staging) texture.
		Texture2DDescription copyTextureDesc(width, height, format, 1, 1, D3D11_USAGE_STAGING);
		auto& copyTexture = GetTexture(device, copyTextureDesc);

		// Copying from the texture to copy texture.
		context->CopyResource(copyTexture.GetResource(), texture);

		// Copying back from the copy texture to the CPU buffer.
		copyTexture.GetData(context, m_PrintScreenBuffer.GetArray());
	}

	// Saving image to the hard drive.
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		m_ThreadPool.GetThread(0).Execute(&ScreenshotHandler::SaveCapturedImage, this, width, height, countChannels);
	}
}