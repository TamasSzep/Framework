// OpenGLRender/Utilities/BackgroundCMRenderer.h

#include <OpenGLRender/Utilities/BackgroundCMRenderer.h>

using namespace EngineBuildingBlocks::Graphics;
using namespace OpenGLRender;

void BackgroundCMRenderer::Initialize(const UtilityInitializationContext& context)
{
	// Creating quad primitive.
	m_Primitive = context.OGLM->PrimitiveManager.GetIndexedPrimitive(QuadDescription());

	// Creating shader program.
	ShaderProgramDescription spd;
	spd.Shaders = {
		ShaderDescription::FromFile(*context.PathHandler, "GLSL/BackgroundCubemap_vs.glsl", ShaderType::Vertex),
		ShaderDescription::FromFile(*context.PathHandler, "GLSL/BackgroundCubemap_ps.glsl", ShaderType::Fragment)
	};
	m_Program = context.OGLM->ShaderManager.GetShaderProgram(spd);
}

unsigned BackgroundCMRenderer::AddTexture(const UtilityInitializationContext& context, const char* name)
{
	TextureSamplingDescription samplingDescription;
	samplingDescription.WrapMode = TextureWrapMode::ClampToEdge;
	return m_BackgroundCubemaps.Add(
		context.OGLM->ConstantTextureManager.GetTexture2DFromFile(name, 
			TextureMipmapGenerationMode::Default, &samplingDescription).Texture);
}

void BackgroundCMRenderer::RemoveTexture(unsigned index)
{
	m_BackgroundCubemaps.Remove(index);
}

void BackgroundCMRenderer::ClearTextures()
{
	m_BackgroundCubemaps.Clear();
}

void BackgroundCMRenderer::Render(const UtilityRenderContext& context, UniformBuffer* renderPassCB,
	unsigned textureIndex)
{
	auto cullingEnabled = glIsEnabled(GL_CULL_FACE);
	GLint depthMask;
	glGetIntegerv(GL_DEPTH_WRITEMASK, &depthMask);

	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);

	m_Program->Bind();

	m_Primitive.PVertexArrayObject->Bind();
	m_Primitive.PVertexBuffer->Bind();
	m_Program->TrySetInputLayoutElements(*m_Primitive.PInputLayout);

	m_BackgroundCubemaps[textureIndex]->Bind(0);
	m_Program->SetUniformValue("BackgroundTexture", 0);

	renderPassCB->Bind(0);

	DrawPrimitive(m_Primitive);

	if(cullingEnabled) glEnable(GL_CULL_FACE);
	if (depthMask) glDepthMask(GL_TRUE);
}