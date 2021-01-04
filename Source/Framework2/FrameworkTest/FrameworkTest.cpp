// FrameworkTest.cpp : Defines the entry point for the console application.
//

#include <FrameworkTest/SimpleOpenGLTest.h>
#include <FrameworkTest/SimpleDirectX11Test.h>
#include <FrameworkTest/SimpleDirectX12Test.h>

enum class GraphicsAPI
{
	OpenGL, DirectX11, DirectX12
};

const GraphicsAPI c_UsedAPI = GraphicsAPI::DirectX11;

int main(int argc, char *argv[])
{
	try
	{
		switch (c_UsedAPI)
		{
		case GraphicsAPI::OpenGL:
		{
			FrameworkTest::SimpleOpenGLTest test(argc, argv);
			return test.Run();
		}
		case GraphicsAPI::DirectX11:
		{
			FrameworkTest::SimpleDirectX11Test test(argc, argv);
			return test.Run();
		}
		case GraphicsAPI::DirectX12:
		{
			FrameworkTest::SimpleDirectX12Test test(argc, argv);
			return test.Run();
		}
		}
	}
	catch (const std::exception& ex)
	{
		EngineBuildingBlocks::PrintException(ex);
	}
	return 2;
}